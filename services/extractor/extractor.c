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
    pg_extract_contact(ctx);
    pg_extract_social(ctx);
    pg_extract_branding(ctx);
    pg_extract_technology(ctx);
    pg_extract_seo(ctx);

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

    printf("\n");

    printf("Robots       : %s\n",ctx->company.robots);
    printf("Sitemap      : %s\n",ctx->company.sitemap);

    printf("\n");

    return PG_STATUS_OK;
}