
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

#endif
