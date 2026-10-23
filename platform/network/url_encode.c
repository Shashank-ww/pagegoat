#include "pagegoat/platform/url_encode.h"

#include <curl/curl.h>
#include <string.h>

void pg_url_encode(
    const char *input,
    char *output,
    size_t output_size)
{
    CURL *curl = curl_easy_init();

    if (!curl)
    {
        output[0] = '\0';
        return;
    }

    char *encoded =
        curl_easy_escape(
            curl,
            input,
            0);

    if (!encoded)
    {
        curl_easy_cleanup(curl);
        output[0] = '\0';
        return;
    }

    strncpy(
        output,
        encoded,
        output_size - 1);

    output[output_size - 1] = '\0';

    curl_free(encoded);

    curl_easy_cleanup(curl);
}