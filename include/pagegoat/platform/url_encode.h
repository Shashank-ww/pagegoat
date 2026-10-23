#ifndef PAGEGOAT_URL_ENCODE_H
#define PAGEGOAT_URL_ENCODE_H

#include <stddef.h>

void pg_url_encode(
    const char *input,
    char *output,
    size_t output_size);

#endif