#include <stdio.h>

#include "pagegoat/coregoat/context.h"

#include "pagegoat/services/extractor/extractor.h"

#include "pagegoat/services/extractor/modules/metadata.h"
#include "pagegoat/services/extractor/modules/contact.h"
#include "pagegoat/services/extractor/modules/social.h"
#include "pagegoat/services/extractor/modules/branding.h"
#include "pagegoat/services/extractor/modules/technology.h"
#include "pagegoat/services/extractor/modules/seo.h"

PGStatus pg_extractor_run(
    PGContext *ctx)
{
    if(ctx==NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    printf("\n========================================\n");
    printf("Extracting Company Intelligence\n");
    printf("========================================\n\n");

    pg_extract_metadata(ctx);
    pg_context_emit_event(ctx, "metadata_extracted",
                          "Extracted page metadata.", ctx->url);

    pg_extract_contact(ctx);
    pg_context_emit_event(ctx, "contact_extracted",
                          "Finalized public contact details.", ctx->url);

    pg_extract_social(ctx);
    pg_context_emit_event(ctx, "social_extracted",
                          "Extracted social profile links.", ctx->url);

    pg_extract_branding(ctx);
    {
        char message[160];
        snprintf(message, sizeof(message),
                 "Detected branding assets and %d font families.", ctx->company.font_count);
        pg_context_emit_event(ctx, "branding_extracted", message, ctx->url);
    }

    pg_extract_technology(ctx);
    {
        char message[160];
        snprintf(message, sizeof(message), "Detected %d technologies.",
                 ctx->company.technology_count);
        pg_context_emit_event(ctx, "technologies_detected", message, ctx->url);
    }

    pg_extract_seo(ctx);
    pg_context_emit_event(ctx, "seo_extracted",
                          "Extracted SEO metadata.", ctx->url);

    printf("Title        : %s\n",ctx->company.title);
    printf("Description  : %s\n",ctx->company.description);
    printf("Keywords     : %s\n",ctx->company.keywords);
    printf("Language     : %s\n",ctx->company.language);
    printf("Canonical    : %s\n",ctx->company.canonical);
    printf("Generator    : %s\n",ctx->company.generator);

    printf("\n");

    printf("Emails Found : %d\n",ctx->company.email_count);

    for(int i=0;i<ctx->company.email_count;i++)
        printf("  %s\n",ctx->company.emails[i]);

    printf("\n");

    printf("Phones Found : %d\n",ctx->company.phone_count);

    for(int i=0;i<ctx->company.phone_count;i++)
        printf("  %s\n",ctx->company.phones[i]);

    printf("\n");

    printf("LinkedIn     : %s\n",ctx->company.linkedin);
    printf("Facebook     : %s\n",ctx->company.facebook);
    printf("Instagram    : %s\n",ctx->company.instagram);
    printf("Twitter/X    : %s\n",ctx->company.twitter);
    printf("YouTube      : %s\n",ctx->company.youtube);
    printf("GitHub       : %s\n",ctx->company.github);

    printf("\n");

    printf("Logo         : %s\n",ctx->company.logo);
    printf("Favicon      : %s\n",ctx->company.favicon);
    printf("Apple Icon   : %s\n",ctx->company.apple_touch_icon);
    printf("Theme Color  : %s\n",ctx->company.theme_color);
    printf("Fonts Found  : %d\n", ctx->company.font_count);
    for (int i = 0; i < ctx->company.font_count; i++)
        printf("  - %s\n", ctx->company.fonts[i]);

    printf("\n");

    printf("Technologies Found : %d\n",
           ctx->company.technology_count);

    if (ctx->company.technology_count == 0)
    {
        printf("  None detected\n");
    }
    else
    {
        for (int i = 0; i < ctx->company.technology_count; i++)
        {
            printf("  - %s\n",
                   ctx->company.technologies[i]);
        }
    }

    printf("\n");

    printf("Robots       : %s\n",ctx->company.robots);
    printf("Sitemap      : %s\n",ctx->company.sitemap);

    printf("\n");

    return PG_STATUS_OK;
}