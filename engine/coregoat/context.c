#include "pagegoat/coregoat/context.h"

#include <string.h>

PGStatus pg_context_init(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    memset(ctx, 0, sizeof(PGContext));

    return PG_STATUS_OK;
}

PGStatus pg_context_reset(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    memset(ctx, 0, sizeof(PGContext));

    return PG_STATUS_OK;
}

PGStatus pg_context_destroy(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    memset(ctx, 0, sizeof(PGContext));

    return PG_STATUS_OK;
}