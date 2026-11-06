#ifndef PAGEGOAT_NORMALIZE_H
#define PAGEGOAT_NORMALIZE_H

#include <stddef.h>

#include "pagegoat/coregoat/status.h"

PGStatus pg_normalize_input(
    const char *input,
    char *output,
    size_t output_size);

PGStatus pg_validate_url(
    const char *url);

/*
 * Decode HTML entities into UTF-8.
 *
 * Unknown or malformed entities are preserved.
 * Input and output must use separate buffers.
 */
PGStatus pg_decode_html_entities(
    const char *input,
    char *output,
    size_t output_size);

#endif