
#include <stdio.h>
#include <string.h>

#include "pagegoat/coregoat/context.h"
#include "pagegoat/coregoat/status.h"

#include "pagegoat/platform/http.h"
#include "pagegoat/platform/logger.h"

#include "pagegoat/services/normalize/normalize.h"
#include "pagegoat/coregoat/pipeline.h"

#include "pagegoat/services/export.h"

int main(void)
{
    PGContext ctx;
    PGStatus status;
    char input[512];

    if (pg_context_init(&ctx) != PG_STATUS_OK)
    {
        pg_log_error("Unable to initialize context.");
        return 1;
    }

    pg_log_info("PageGoat CLI");

    printf("\nWebsite: ");
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL)
    {
        pg_log_warning("Unable to read website input.");
        pg_context_destroy(&ctx);
        return 1;
    }

    /* Reject input that exceeded the input buffer. */
    if (strchr(input, '\n') == NULL && !feof(stdin))
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* Discard the remainder of the input line. */
        }

        pg_log_warning("Input is too long.");
        pg_context_destroy(&ctx);
        return 1;
    }

    input[strcspn(input, "\r\n")] = '\0';

    status = pg_normalize_input(
        input,
        ctx.url,
        sizeof(ctx.url));

    if (status != PG_STATUS_OK)
    {
        pg_log_warning(
            "Invalid website input. Enter a domain or website URL.");

        pg_context_destroy(&ctx);
        return 1;
    }

    printf("\nTarget : %s\n\n", ctx.url);

    if (pg_http_init() != PG_STATUS_OK)
    {
        pg_log_error("Unable to initialize HTTP.");
        pg_context_destroy(&ctx);
        return 1;
    }

    
    status = pg_pipeline_run(&ctx);

    if (status != PG_STATUS_OK)
    {
        if (status == PG_STATUS_NETWORK_ERROR)
        {
            if (ctx.response.status < 200 ||
                ctx.response.status >= 400)
            {
                fprintf(
                    stderr,
                    "[FAIL] Website returned HTTP status %ld.\n",
                    ctx.response.status);
            }
            else
            {
                pg_log_error(
                    "Website download failed. Check the URL, network, or TLS certificate.");
            }
        }
        else if (status == PG_STATUS_PARSE_ERROR)
        {
            pg_log_error("Parser failed.");
        }
        else
        {
            pg_log_error("Extraction failed.");
        }

        pg_http_free(&ctx.response);
        pg_http_shutdown();
        pg_context_destroy(&ctx);
        return 1;
    }

    status = pg_export_company_json(&ctx, "company.json");

    if (status != PG_STATUS_OK)
    {
        pg_log_error("Unable to export company.json.");

        pg_http_free(&ctx.response);
        pg_http_shutdown();
        pg_context_destroy(&ctx);
        return 1;
    }

    printf("\n[INFO] JSON exported to company.json\n");

    pg_http_free(&ctx.response);
    pg_http_shutdown();
    pg_context_destroy(&ctx);

    return 0;
}
