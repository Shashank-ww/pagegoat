#include "pagegoat/services/extractor/parser.h"

PGStatus
pg_parser_parse(PGContext *ctx)
{
    if (ctx == NULL)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    /*
        Ensure we downloaded a page.
    */

    if (ctx->response.body == NULL)
    {
        return PG_STATUS_PARSE_ERROR;
    }

    /*
        Nothing else to do here.

        The extractor modules will read directly from:

            ctx->response.body

        This keeps one source of truth and avoids
        storing duplicate HTML buffers.
    */

    return PG_STATUS_OK;
}