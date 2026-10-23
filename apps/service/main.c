// This is a service/main.c not the app/main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pagegoat/coregoat/context.h"
#include "pagegoat/coregoat/status.h"
#include "pagegoat/coregoat/pipeline.h"
#include "pagegoat/platform/http.h"
#include "pagegoat/platform/logger.h"
#include "pagegoat/services/normalize/normalize.h"
#include "pagegoat/services/export.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET PGSocket;
    #define PG_CLOSE_SOCKET(s) closesocket(s)
    #define PG_SOCKET_ERROR SOCKET_ERROR
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int PGSocket;
    #define PG_CLOSE_SOCKET(s) close(s)
    #define PG_SOCKET_ERROR (-1)
#endif

#define PG_SERVICE_PORT 8787
#define PG_REQUEST_SIZE 8192
#define PG_RESPONSE_FILE "pagegoat_service_response.json"

static int send_all(PGSocket client, const char *data, size_t length)
{
    size_t sent = 0;

    while (sent < length)
    {
        int n = send(client, data + sent, (int)(length - sent), 0);

        if (n <= 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
}

static void send_json(
    PGSocket client,
    int status,
    const char *reason,
    const char *json)
{
    char header[512];
    size_t body_length = strlen(json);

    int header_length = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: http://localhost:3000\r\n"
        "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "\r\n",
        status,
        reason,
        body_length);

    if (header_length > 0 && (size_t)header_length < sizeof(header))
    {
        send_all(client, header, (size_t)header_length);
        send_all(client, json, body_length);
    }
}


static char *find_case_insensitive(
    const char *text,
    const char *target)
{
    if (text == NULL || target == NULL || target[0] == '\0')
        return NULL;

    for (const char *p = text; *p != '\0'; p++)
    {
        const char *a = p;
        const char *b = target;

        while (*a != '\0' && *b != '\0' &&
               tolower((unsigned char)*a) ==
               tolower((unsigned char)*b))
        {
            a++;
            b++;
        }

        if (*b == '\0')
            return (char *)p;
    }

    return NULL;
}


static int extract_url(const char *body, char *url, size_t capacity)
{
    const char *key = strstr(body, "\"url\"");
    const char *value;
    const char *end;
    size_t length;

    if (key == NULL)
        return -1;

    value = strchr(key, ':');

    if (value == NULL)
        return -1;

    value++;

    while (*value != '\0' && isspace((unsigned char)*value))
        value++;

    if (*value != '"')
        return -1;

    value++;
    end = value;

    while (*end != '\0' && *end != '"')
    {
        /* Reject escaped strings instead of misreading their contents. */
        if (*end == '\\')
            return -1;

        end++;
    }

    if (*end != '"')
        return -1;

    length = (size_t)(end - value);

    if (length == 0 || length >= capacity)
        return -1;

    memcpy(url, value, length);
    url[length] = '\0';

    return 0;
}

static void handle_request(PGSocket client)
{
    char request[PG_REQUEST_SIZE + 1] = {0};
    char url[512] = {0};
    char normalized_url[512] = {0};
    char *header_end;
    char *body;
    char *length_header;
    size_t received = 0;
    size_t header_length;
    size_t content_length = 0;
    PGContext ctx;
    PGStatus status;
    FILE *file;
    long file_size;
    char *json = NULL;

    /* Read until the HTTP headers arrive. */
    while (received < PG_REQUEST_SIZE)
    {
        int n = recv(
            client,
            request + received,
            (int)(PG_REQUEST_SIZE - received),
            0);

        if (n <= 0)
            return;

        received += (size_t)n;
        request[received] = '\0';

        header_end = strstr(request, "\r\n\r\n");

        if (header_end != NULL)
            break;
    }

    header_end = strstr(request, "\r\n\r\n");

    if (header_end == NULL)
    {
        send_json(client, 413, "Payload Too Large",
                  "{\"error\":\"Request is too large.\"}");
        return;
    }

    header_length = (size_t)(header_end + 4 - request);
    body = header_end + 4;

    if (strncmp(request, "OPTIONS ", 8) == 0)
    {
        send_json(client, 204, "No Content", "");
        return;
    }

    if (strncmp(request, "POST /extract ", 14) != 0)
    {
        send_json(client, 404, "Not Found",
                  "{\"error\":\"Use POST /extract.\"}");
        return;
    }

    length_header = find_case_insensitive(
    request,
    "Content-Length:");

    if (length_header != NULL && length_header < header_end)
    {
        length_header += strlen("Content-Length:");

        while (*length_header == ' ')
            length_header++;

        content_length = (size_t)strtoul(length_header, NULL, 10);
    }

    if (content_length == 0 || content_length > 2048 ||
        header_length + content_length > PG_REQUEST_SIZE)
    {
        send_json(client, 400, "Bad Request",
                  "{\"error\":\"Invalid request body size.\"}");
        return;
    }

    /* Read the rest of the request body if it arrived in multiple chunks. */
    while (received < header_length + content_length)
    {
        int n = recv(
            client,
            request + received,
            (int)(header_length + content_length - received),
            0);

        if (n <= 0)
            return;

        received += (size_t)n;
        request[received] = '\0';
    }

    body[content_length] = '\0';

    if (extract_url(body, url, sizeof(url)) != 0)
    {
        send_json(client, 400, "Bad Request",
                  "{\"error\":\"Provide a valid URL field.\"}");
        return;
    }

    if (pg_normalize_input(url, normalized_url,
                           sizeof(normalized_url)) != PG_STATUS_OK)
    {
        send_json(client, 400, "Bad Request",
                  "{\"error\":\"Please enter a valid website URL.\"}");
        return;
    }

    if (pg_context_init(&ctx) != PG_STATUS_OK)
    {
        send_json(client, 500, "Internal Server Error",
                  "{\"error\":\"Could not initialize extraction.\"}");
        return;
    }

    snprintf(ctx.url, sizeof(ctx.url), "%s", normalized_url);

    status = pg_pipeline_run(&ctx);

    if (status != PG_STATUS_OK)
    {
        pg_http_free(&ctx.response);
        pg_context_destroy(&ctx);

        send_json(client, 502, "Bad Gateway",
                  "{\"error\":\"Website extraction failed.\"}");
        return;
    }

    remove(PG_RESPONSE_FILE);

    status = pg_export_company_json(&ctx, PG_RESPONSE_FILE);

    pg_http_free(&ctx.response);
    pg_context_destroy(&ctx);

    if (status != PG_STATUS_OK)
    {
        remove(PG_RESPONSE_FILE);
        send_json(client, 500, "Internal Server Error",
                  "{\"error\":\"Could not export extraction results.\"}");
        return;
    }

    file = fopen(PG_RESPONSE_FILE, "rb");

    if (file == NULL)
    {
        remove(PG_RESPONSE_FILE);
        send_json(client, 500, "Internal Server Error",
                  "{\"error\":\"Could not read extraction results.\"}");
        return;
    }

    if (fseek(file, 0, SEEK_END) != 0 ||
        (file_size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        remove(PG_RESPONSE_FILE);
        send_json(client, 500, "Internal Server Error",
                  "{\"error\":\"Could not read extraction results.\"}");
        return;
    }

    json = malloc((size_t)file_size + 1);

    if (json == NULL ||
        fread(json, 1, (size_t)file_size, file) != (size_t)file_size)
    {
        free(json);
        fclose(file);
        remove(PG_RESPONSE_FILE);
        send_json(client, 500, "Internal Server Error",
                  "{\"error\":\"Could not load extraction results.\"}");
        return;
    }

    json[file_size] = '\0';
    fclose(file);
    remove(PG_RESPONSE_FILE);

    send_json(client, 200, "OK", json);
    free(json);
}

int main(void)
{
    PGSocket server;
    struct sockaddr_in address;
    int reuse = 1;

#ifdef _WIN32
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        fprintf(stderr, "Unable to initialize Windows sockets.\n");
        return 1;
    }
#endif

    if (pg_http_init() != PG_STATUS_OK)
    {
        fprintf(stderr, "Unable to initialize PageGoat HTTP.\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server == PG_SOCKET_ERROR)
    {
        fprintf(stderr, "Unable to create service socket.\n");
        pg_http_shutdown();
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&reuse, sizeof(reuse));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PG_SERVICE_PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) ==
            PG_SOCKET_ERROR ||
        listen(server, 8) == PG_SOCKET_ERROR)
    {
        fprintf(stderr, "Unable to bind to localhost:%d.\n",
                PG_SERVICE_PORT);
        PG_CLOSE_SOCKET(server);
        pg_http_shutdown();
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("PageGoat service listening at http://127.0.0.1:%d\n",
           PG_SERVICE_PORT);
    printf("Endpoint: POST http://127.0.0.1:%d/extract\n",
           PG_SERVICE_PORT);

    for (;;)
    {
        PGSocket client = accept(server, NULL, NULL);

        if (client == PG_SOCKET_ERROR)
            continue;

        handle_request(client);
        PG_CLOSE_SOCKET(client);
    }

    /* The service runs until stopped. */
    PG_CLOSE_SOCKET(server);
    pg_http_shutdown();

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}