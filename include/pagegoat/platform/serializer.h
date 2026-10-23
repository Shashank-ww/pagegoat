#ifndef PAGEGOAT_SERIALIZER_H
#define PAGEGOAT_SERIALIZER_H

#include <stddef.h>

void pg_google_search_url(
    const char *query,
    char *url,
    size_t size);

void pg_bing_search_url(
    const char *query,
    char *url,
    size_t size);

void pg_duckduckgo_search_url(
    const char *query,
    char *url,
    size_t size);

void pg_yahoo_search_url(
    const char *query,
    char *url,
    size_t size);

#endif