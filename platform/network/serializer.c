#include "pagegoat/platform/serializer.h"
#include "pagegoat/platform/url_encode.h"

#include <stdio.h>

void pg_google_search_url(
    const char *query,
    char *url,
    size_t size)
{
    char encoded[1024];

    pg_url_encode(
        query,
        encoded,
        sizeof(encoded));

    snprintf(
        url,
        size,
        "https://www.google.com/search?q=%s",
        encoded);
}

void pg_bing_search_url(
    const char *query,
    char *url,
    size_t size)
{
    char encoded[1024];

    pg_url_encode(query, encoded, sizeof(encoded));

    snprintf(
        url,
        size,
        "https://www.bing.com/search?q=%s",
        encoded);
}

void pg_duckduckgo_search_url(
    const char *query,
    char *url,
    size_t size)
{
    char encoded[1024];

    pg_url_encode(query, encoded, sizeof(encoded));

    snprintf(
        url,
        size,
        "https://duckduckgo.com/html/?q=%s",
        encoded);
}

void pg_yahoo_search_url(
    const char *query,
    char *url,
    size_t size)
{
    char encoded[1024];

    pg_url_encode(query, encoded, sizeof(encoded));

    snprintf(
        url,
        size,
        "https://search.yahoo.com/search?p=%s",
        encoded);
}