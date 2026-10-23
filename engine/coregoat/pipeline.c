
#include "pagegoat/coregoat/pipeline.h"

#include "pagegoat/platform/http.h"
#include "pagegoat/services/extractor/parser.h"
#include "pagegoat/services/extractor/extractor.h"

PGStatus pg_pipeline_run(PGContext *ctx)
{
    PGStatus status;

    if (ctx == NULL || ctx->url[0] == '\0')
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    if (pg_http_get(ctx->url, &ctx->response) != 0)
    {
        return PG_STATUS_NETWORK_ERROR;
    }

    if (ctx->response.status < 200 || ctx->response.status >= 400)
    {
        return PG_STATUS_NETWORK_ERROR;
    }

    status = pg_parser_parse(ctx);

    if (status != PG_STATUS_OK)
    {
        return status;
    }

    status = pg_extractor_run(ctx);

    if (status != PG_STATUS_OK)
    {
        return status;
    }

    return PG_STATUS_OK;
}
