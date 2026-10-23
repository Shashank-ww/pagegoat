
#include "pagegoat/platform/http.h"
#include "pagegoat/platform/http_headers.h"
#include "pagegoat/platform/logger.h"

#include <curl/curl.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *data;
    size_t size;
} Buffer;


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
    PG_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, 1L);
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
