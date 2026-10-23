#include "pagegoat/platform/http_headers.h"

struct curl_slist *pg_http_headers(void)
{
    struct curl_slist *headers = NULL;

    headers = curl_slist_append(
        headers,
        "Accept: text/html");

    headers = curl_slist_append(
        headers,
        "Accept-Language: en-US,en;q=0.9");

    headers = curl_slist_append(
        headers,
        "Accept-Encoding: gzip, deflate");

    headers = curl_slist_append(
        headers,
        "Connection: keep-alive");

    headers = curl_slist_append(
        headers,
        "Cache-Control: no-cache");

    return headers;
}