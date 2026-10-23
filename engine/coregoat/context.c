#include <string.h>

#include "pagegoat/coregoat/context.h"

PGStatus
pg_context_init(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    memset(ctx, 0, sizeof(PGContext));

    return PG_STATUS_OK;
}

PGStatus
pg_context_reset(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    /*
        Keep the normalized URL.
    */

    char url[512];

    strcpy(url, ctx->url);

    memset(ctx, 0, sizeof(PGContext));

    strcpy(ctx->url, url);

    return PG_STATUS_OK;
}

PGStatus
pg_context_destroy(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    /*
        HTTP response is released by
        pg_http_free() inside main().
    */

    memset(ctx, 0, sizeof(PGContext));

    return PG_STATUS_OK;
}