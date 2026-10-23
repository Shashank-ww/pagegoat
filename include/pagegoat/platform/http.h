#ifndef PAGEGOAT_HTTP_H
#define PAGEGOAT_HTTP_H

#include <stddef.h>

typedef struct
{
    long status;

    char *body;

    size_t length;

    char final_url[1024];

} PGHttpResponse;

/* Initialize libcurl */
int pg_http_init(void);

/* Cleanup libcurl */
void pg_http_shutdown(void);

/* HTTP GET */
int pg_http_get(
    const char *url,
    PGHttpResponse *response);

/* Release downloaded response */
void pg_http_free(
    PGHttpResponse *response);

#endif