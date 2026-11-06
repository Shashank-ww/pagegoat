
#include <stdio.h>
#include <string.h>

#include "pagegoat/coregoat/context.h"
#include "pagegoat/coregoat/status.h"

#include "pagegoat/services/extractor/extractor.h"

#include "pagegoat/services/extractor/modules/metadata.h"
#include "pagegoat/services/extractor/modules/contact.h"
#include "pagegoat/services/extractor/modules/social.h"
#include "pagegoat/services/extractor/modules/branding.h"
#include "pagegoat/services/extractor/modules/technology.h"
#include "pagegoat/services/extractor/modules/seo.h"

static void report_module_status(
    PGContext *ctx,
    const char *module_name,
    const char *success_event,
    const char *warning_event,
    const char *success_message,
    PGStatus status)
{
    char message[256];

    if (status == PG_STATUS_OK)
    {
        pg_context_emit_event(
            ctx,
            success_event,
            success_message,
            ctx->url);

        return;
    }

    snprintf(
        message,
        sizeof(message),
        "%s extraction returned: %s. Available results have been retained.",
        module_name,
        pg_status_string(status));

    pg_context_emit_event(
        ctx,
        warning_event,
        message,
        ctx->url);

    fprintf(
        stderr,
        "[WARNING] %s extraction: %s\n",
        module_name,
        pg_status_string(status));
}

PGStatus pg_extractor_run(PGContext *ctx)
{
    PGStatus status;

    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    printf("\n========================================\n");
    printf("Extracting Company Intelligence\n");
    printf("========================================\n\n");

    status = pg_extract_metadata(ctx);
    report_module_status(
        ctx,
        "Metadata",
        "metadata_extracted",
        "metadata_extraction_warning",
        "Extracted page metadata.",
        status);

    status = pg_extract_contact(ctx);
    report_module_status(
        ctx,
        "Contact",
        "contact_extracted",
        "contact_extraction_warning",
        "Finalized public contact details.",
        status);

    status = pg_extract_social(ctx);
    report_module_status(
        ctx,
        "Social",
        "social_extracted",
        "social_extraction_warning",
        "Extracted social profile links.",
        status);

    status = pg_extract_branding(ctx);

    {
        char message[160];

        snprintf(
            message,
            sizeof(message),
            "Detected branding assets and %d font families.",
            ctx->company.font_count);

        report_module_status(
            ctx,
            "Branding",
            "branding_extracted",
            "branding_extraction_warning",
            message,
            status);
    }

    status = pg_extract_technology(ctx);

    {
        char message[160];

        snprintf(
            message,
            sizeof(message),
            "Detected %d technologies.",
            ctx->company.technology_count);

        report_module_status(
            ctx,
            "Technology",
            "technologies_detected",
            "technology_extraction_warning",
            message,
            status);
    }

    status = pg_extract_seo(ctx);
    report_module_status(
        ctx,
        "SEO",
        "seo_extracted",
        "seo_extraction_warning",
        "Extracted SEO metadata.",
        status);

    printf("Title        : %s\n", ctx->company.title);
    printf("Description  : %s\n", ctx->company.description);
    printf("Keywords     : %s\n", ctx->company.keywords);
    printf("Language     : %s\n", ctx->company.language);
    printf("Canonical    : %s\n", ctx->company.canonical);
    printf("Generator    : %s\n", ctx->company.generator);

    printf("\nEmails Found : %d\n", ctx->company.email_count);

    for (int i = 0; i < ctx->company.email_count; i++)
        printf("  %s\n", ctx->company.emails[i]);

    printf("\nPhones Found : %d\n", ctx->company.phone_count);

    for (int i = 0; i < ctx->company.phone_count; i++)
        printf("  %s\n", ctx->company.phones[i]);

    printf("\nLinkedIn     : %s\n", ctx->company.linkedin);
    printf("Facebook     : %s\n", ctx->company.facebook);
    printf("Instagram    : %s\n", ctx->company.instagram);
    printf("Twitter/X    : %s\n", ctx->company.twitter);
    printf("YouTube      : %s\n", ctx->company.youtube);
    printf("GitHub       : %s\n", ctx->company.github);

    printf("\nLogo         : %s\n", ctx->company.logo);
    printf("Favicon      : %s\n", ctx->company.favicon);
    printf("Apple Icon   : %s\n", ctx->company.apple_touch_icon);
    printf("Theme Color  : %s\n", ctx->company.theme_color);

    printf("Fonts Found  : %d\n", ctx->company.font_count);

    for (int i = 0; i < ctx->company.font_count; i++)
        printf("  - %s\n", ctx->company.fonts[i]);

    printf("\nTechnologies Found : %d\n",
           ctx->company.technology_count);

    if (ctx->company.technology_count == 0)
    {
        printf("  None detected\n");
    }
    else
    {
        for (int i = 0; i < ctx->company.technology_count; i++)
            printf("  - %s\n", ctx->company.technologies[i]);
    }

    printf("\nRobots       : %s\n", ctx->company.robots);
    printf("Sitemap      : %s\n", ctx->company.sitemap);

    printf("\n");

    return PG_STATUS_OK;
}
