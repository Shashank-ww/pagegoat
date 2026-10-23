#ifndef PAGEGOAT_CONTEXT_H
#define PAGEGOAT_CONTEXT_H

#include "pagegoat/platform/http.h"
#include "pagegoat/coregoat/company.h"
#include "pagegoat/coregoat/status.h"

typedef struct
{
    char url[512];

    PGHttpResponse response;

    PGCompany company;

} PGContext;

PGStatus
pg_context_init(
    PGContext *ctx);

PGStatus
pg_context_reset(
    PGContext *ctx);

PGStatus
pg_context_destroy(
    PGContext *ctx);

#endif