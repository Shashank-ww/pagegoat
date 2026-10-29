
#include "pagegoat/coregoat/pipeline.h"

#include "pagegoat/platform/http.h"
#include "pagegoat/services/extractor/parser.h"
#include "pagegoat/services/extractor/extractor.h"
#include "pagegoat/services/crawl.h"

PGStatus pg_pipeline_run(PGContext *ctx)
{
    PGStatus status;

    if (ctx == NULL || ctx->url[0] == '\0')
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    pg_context_emit_event(ctx, "homepage_fetch_started",
                          "Fetching the homepage.", ctx->url);

    if (pg_http_get(ctx->url, &ctx->response) != 0)
    {
        pg_context_emit_event(ctx, "homepage_fetch_failed",
                              "The homepage request failed.", ctx->url);
        return PG_STATUS_NETWORK_ERROR;
    }

    if (ctx->response.status < 200 || ctx->response.status >= 400)
    {
        pg_context_emit_event(ctx, "homepage_fetch_failed",
                              "The homepage returned an unsuccessful HTTP status.",
                              ctx->response.final_url[0] ? ctx->response.final_url : ctx->url);
        return PG_STATUS_NETWORK_ERROR;
    }

    pg_context_emit_event(ctx, "homepage_fetched",
                          "Homepage downloaded successfully.",
                          ctx->response.final_url[0] ? ctx->response.final_url : ctx->url);

    status = pg_parser_parse(ctx);

    if (status != PG_STATUS_OK)
    {
        pg_context_emit_event(ctx, "html_parse_failed",
                              "Could not parse the homepage HTML.", ctx->url);
        return status;
    }

    pg_context_emit_event(ctx, "html_parsed",
                          "Homepage HTML parsed.", ctx->url);

    /*
     * Scan the homepage and prioritized same-origin pages for
     * additional public contact details before final extraction output.
     */
    status = pg_crawl_contact_pages(ctx);

    if (status != PG_STATUS_OK)
    {
        return status;
    }

    status = pg_extractor_run(ctx);

    if (status != PG_STATUS_OK)
    {
        pg_context_emit_event(ctx, "extraction_failed",
                              "Company intelligence extraction failed.", ctx->url);
        return status;
    }

    pg_context_emit_event(ctx, "extraction_completed",
                          "Company intelligence extraction completed.", ctx->url);
    return PG_STATUS_OK;
}
