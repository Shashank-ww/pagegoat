#ifndef PAGEGOAT_HTTP_HEADERS_H
#define PAGEGOAT_HTTP_HEADERS_H

#include <curl/curl.h>

struct curl_slist *pg_http_headers(void);

#endif