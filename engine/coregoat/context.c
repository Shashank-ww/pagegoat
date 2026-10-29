#include <stdio.h>
#include <string.h>
#include "pagegoat/platform/logger.h"

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
void pg_context_emit_event(
    PGContext *ctx,
    const char *type,
    const char *message,
    const char *url)
{
    if (ctx == NULL || type == NULL || message == NULL)
        return;

    if (ctx->event_callback != NULL)
    {
        ctx->event_callback(ctx->event_user_data, type, message,
                            url != NULL ? url : "");
    }
    else
    {
        if (url != NULL && url[0] != '\0')
        {
            char line[1400];
            snprintf(line, sizeof(line), "%s [%s]", message, url);
            pg_log_info(line);
        }
        else
        {
            pg_log_info(message);
        }
    }
}
