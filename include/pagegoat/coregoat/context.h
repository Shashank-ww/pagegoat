#ifndef PAGEGOAT_CONTEXT_H
#define PAGEGOAT_CONTEXT_H

#include <stddef.h>

#include "pagegoat/coregoat/status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PG_MAX_QUERY_LENGTH 512
#define PG_MAX_URL_LENGTH   2048

typedef struct PGContext
{
    char search_query[PG_MAX_QUERY_LENGTH];

    char current_url[PG_MAX_URL_LENGTH];

    unsigned int companies_found;

    unsigned int companies_processed;

    unsigned int current_company;

} PGContext;

PGStatus pg_context_init(PGContext *ctx);

PGStatus pg_context_reset(PGContext *ctx);

PGStatus pg_context_destroy(PGContext *ctx);

#ifdef __cplusplus
}
#endif

#endif