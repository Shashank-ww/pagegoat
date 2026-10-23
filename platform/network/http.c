
#include "pagegoat/platform/http.h"
#include "pagegoat/platform/http_headers.h"
#include "pagegoat/platform/logger.h"

#include <curl/curl.h>

#define PG_HTTP_MAX_RESPONSE_BYTES (10U * 1024U * 1024U)

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

typedef struct
{
    char *data;
    size_t size;
} Buffer;


/*----------------------------------------------------------*/
/* SSRF protection: reject non-public destination addresses  */
/*----------------------------------------------------------*/

static int is_forbidden_ipv4(const unsigned char *ip)
{
    unsigned int a = ip[0];
    unsigned int b = ip[1];

    if (a == 0 || a == 10 || a == 127 || a >= 224)
        return 1;

    if (a == 100 && b >= 64 && b <= 127)
        return 1;

    if (a == 169 && b == 254)
        return 1;

    if (a == 172 && b >= 16 && b <= 31)
        return 1;

    if (a == 192 && b == 168)
        return 1;

    if (a == 192 && b == 0 && ip[2] == 0)
        return 1;

    if (a == 192 && b == 0 && ip[2] == 2)
        return 1;

    if (a == 192 && b == 88 && ip[2] == 99)
        return 1;

    if (a == 198 && (b == 18 || b == 19))
        return 1;

    if (a == 198 && b == 51 && ip[2] == 100)
        return 1;

    if (a == 203 && b == 0 && ip[2] == 113)
        return 1;

    return 0;
}

static int is_forbidden_ip(const char *address)
{
    if (address == NULL || address[0] == '\0')
        return 1;

    struct in_addr ipv4;

#ifdef _WIN32
    if (InetPtonA(AF_INET, address, &ipv4) == 1)
#else
    if (inet_pton(AF_INET, address, &ipv4) == 1)
#endif
    {
        const unsigned char *ip =
            (const unsigned char *)&ipv4;

        return is_forbidden_ipv4(ip);
    }

    struct in6_addr ipv6;

#ifdef _WIN32
    if (InetPtonA(AF_INET6, address, &ipv6) != 1)
#else
    if (inet_pton(AF_INET6, address, &ipv6) != 1)
#endif
    {
        /* Fail closed if the IP cannot be parsed. */
        return 1;
    }

    const unsigned char *ip = ipv6.s6_addr;

    /* IPv6 unspecified and loopback. */
    static const unsigned char zero[16] = {0};

    if (memcmp(ip, zero, 16) == 0)
        return 1;

    if (memcmp(ip, zero, 15) == 0 && ip[15] == 1)
        return 1;

    /* IPv6 unique-local, link-local and multicast. */
    if ((ip[0] & 0xFE) == 0xFC)
        return 1;

    if (ip[0] == 0xFE && (ip[1] & 0xC0) == 0x80)
        return 1;

    if (ip[0] == 0xFF)
        return 1;

    /* IPv4-mapped IPv6: ::ffff:a.b.c.d */
    if (memcmp(ip, zero, 10) == 0 &&
        ip[10] == 0xFF && ip[11] == 0xFF)
    {
        return is_forbidden_ipv4(ip + 12);
    }

    return 0;
}

static int prereq_callback(
    void *clientp,
    char *conn_primary_ip,
    char *conn_local_ip,
    int conn_primary_port,
    int conn_local_port)
{
    (void)clientp;
    (void)conn_local_ip;
    (void)conn_local_port;

    if (is_forbidden_ip(conn_primary_ip))
    {
        pg_log_error(
            "Blocked HTTP connection to a non-public IP address.");
        return CURL_PREREQFUNC_ABORT;
    }

    return CURL_PREREQFUNC_OK;
}


/*----------------------------------------------------------*/
/* Write callback                                            */
/*----------------------------------------------------------*/

static size_t write_callback(
    void *contents,
    size_t size,
    size_t nmemb,
    void *userp)
{
    if (contents == NULL || userp == NULL)
        return 0;

    if (size != 0 && nmemb > SIZE_MAX / size)
        return 0;

    size_t total = size * nmemb;
    Buffer *buffer = (Buffer *)userp;

    /* Reject responses larger than 10 MiB. */
    if (buffer->size > PG_HTTP_MAX_RESPONSE_BYTES ||
        total > PG_HTTP_MAX_RESPONSE_BYTES - buffer->size)
        return 0;

    if (total > SIZE_MAX - buffer->size - 1)
        return 0;

    char *ptr = realloc(
        buffer->data,
        buffer->size + total + 1);

    if (ptr == NULL)
        return 0;

    buffer->data = ptr;

    memcpy(
        buffer->data + buffer->size,
        contents,
        total);

    buffer->size += total;
    buffer->data[buffer->size] = '\0';

    return total;
}



/*----------------------------------------------------------*/
/* Init                                                      */
/*----------------------------------------------------------*/

int pg_http_init(void)
{
    CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);

    if (code != CURLE_OK)
    {
        pg_log_error("Unable to initialize libcurl.");
        return 1;
    }

    pg_log_info("HTTP initialized.");

    return 0;
}


/*----------------------------------------------------------*/
/* Shutdown                                                  */
/*----------------------------------------------------------*/

void pg_http_shutdown(void)
{
    curl_global_cleanup();

    pg_log_info("HTTP shutdown.");
}


/*----------------------------------------------------------*/
/* HTTP GET                                                  */
/*----------------------------------------------------------*/

int pg_http_get(
    const char *url,
    PGHttpResponse *response)
{
    if (url == NULL || response == NULL || url[0] == '\0')
    {
        pg_log_error("Invalid HTTP request arguments.");
        return 1;
    }

    memset(response, 0, sizeof(*response));

    CURL *curl = curl_easy_init();

    if (curl == NULL)
    {
        pg_log_error("Unable to create CURL handle.");
        return 1;
    }

    Buffer buffer = {0};
    struct curl_slist *headers = NULL;
    int result = 1;

    headers = pg_http_headers();

    /*
     * Configure the request completely before performing it.
     * CURLOPT_ACCEPT_ENCODING enables supported compression
     * and automatic response decompression.
     */
    CURLcode code;

#define PG_CURL_SETOPT(option, value)                 \
    do                                                \
    {                                                 \
        code = curl_easy_setopt(curl, option, value); \
        if (code != CURLE_OK)                         \
        {                                             \
            pg_log_error(curl_easy_strerror(code));   \
            goto cleanup;                             \
        }                                             \
    } while (0)

    PG_CURL_SETOPT(CURLOPT_URL, url);

    /* Do not allow environment-configured proxies to bypass
    * destination IP checks. */
    PG_CURL_SETOPT(CURLOPT_PROXY, "");

    /* Only permit HTTP and HTTPS requests and redirects. */
    PG_CURL_SETOPT(
        CURLOPT_PROTOCOLS,
        CURLPROTO_HTTP | CURLPROTO_HTTPS);

    PG_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, 1L);

    PG_CURL_SETOPT(
        CURLOPT_REDIR_PROTOCOLS,
        CURLPROTO_HTTP | CURLPROTO_HTTPS);

    PG_CURL_SETOPT(CURLOPT_MAXREDIRS, 10L);

    PG_CURL_SETOPT(CURLOPT_ACCEPT_ENCODING, "");
    PG_CURL_SETOPT(CURLOPT_TIMEOUT, 30L);
    PG_CURL_SETOPT(CURLOPT_CONNECTTIMEOUT, 10L);
    PG_CURL_SETOPT(
        CURLOPT_USERAGENT,
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/138.0 Safari/537.36");
    PG_CURL_SETOPT(CURLOPT_WRITEFUNCTION, write_callback);
    PG_CURL_SETOPT(CURLOPT_WRITEDATA, &buffer);
    PG_CURL_SETOPT(CURLOPT_NOSIGNAL, 1L);

    PG_CURL_SETOPT(
    CURLOPT_PREREQFUNCTION,
    prereq_callback);

    PG_CURL_SETOPT(
    CURLOPT_PREREQDATA,
    NULL);

    if (headers == NULL)
    {
        pg_log_error("Unable to allocate HTTP request headers.");
        goto cleanup;
    }

    PG_CURL_SETOPT(CURLOPT_HTTPHEADER, headers);

#undef PG_CURL_SETOPT

    code = curl_easy_perform(curl);

    if (code != CURLE_OK)
    {
        pg_log_error(curl_easy_strerror(code));
        goto cleanup;
    }

    code = curl_easy_getinfo(
        curl,
        CURLINFO_RESPONSE_CODE,
        &response->status);

    if (code != CURLE_OK)
    {
        pg_log_error("Unable to retrieve HTTP response status.");
        goto cleanup;
    }

    char *effective_url = NULL;

    code = curl_easy_getinfo(
        curl,
        CURLINFO_EFFECTIVE_URL,
        &effective_url);

    if (code != CURLE_OK)
    {
        pg_log_error("Unable to retrieve final URL.");
        goto cleanup;
    }

    if (effective_url != NULL)
    {
        strncpy(
            response->final_url,
            effective_url,
            sizeof(response->final_url) - 1);

        response->final_url[
            sizeof(response->final_url) - 1] = '\0';
    }

    /*
     * Ensure an empty HTTP response still has a valid,
     * null-terminated body string.
     */
    if (buffer.data == NULL)
    {
        buffer.data = calloc(1, 1);

        if (buffer.data == NULL)
        {
            pg_log_error("Unable to allocate response body.");
            goto cleanup;
        }
    }

    response->body = buffer.data;
    response->length = buffer.size;

    buffer.data = NULL;
    result = 0;

cleanup:
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(buffer.data);

    if (result != 0)
    {
        pg_http_free(response);
        memset(response, 0, sizeof(*response));
    }

    return result;
}


/*----------------------------------------------------------*/
/* Free                                                      */
/*----------------------------------------------------------*/

void pg_http_free(
    PGHttpResponse *response)
{
    if (response == NULL)
        return;

    free(response->body);

    response->body = NULL;
    response->length = 0;
    response->status = 0;
    response->final_url[0] = '\0';
}
