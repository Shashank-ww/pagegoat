
#include <string.h>

#include "pagegoat/coregoat/company.h"

#define CHECK_FIELD(field) \
    do \
    { \
        if (memchr((field), '\0', sizeof(field)) == NULL) \
            return PG_STATUS_INVALID_ARGUMENT; \
    } while (0)

PGStatus pg_company_validate(const PGCompany *company)
{
    if (company == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    if (company->email_count < 0 ||
        company->email_count > PG_MAX_EMAILS)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    if (company->phone_count < 0 ||
        company->phone_count > PG_MAX_PHONES)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    if (company->technology_count < 0 ||
        company->technology_count > PG_MAX_TECHNOLOGIES ||
        company->font_count < 0 ||
        company->font_count > PG_MAX_FONTS)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    if (company->people_count < 0 ||
        company->people_count > PG_MAX_PEOPLE)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    CHECK_FIELD(company->title);
    CHECK_FIELD(company->description);
    CHECK_FIELD(company->keywords);
    CHECK_FIELD(company->language);
    CHECK_FIELD(company->generator);
    CHECK_FIELD(company->canonical);

    CHECK_FIELD(company->og_title);
    CHECK_FIELD(company->og_description);
    CHECK_FIELD(company->og_image);
    CHECK_FIELD(company->og_url);

    CHECK_FIELD(company->twitter_card);
    CHECK_FIELD(company->twitter_title);
    CHECK_FIELD(company->twitter_description);
    CHECK_FIELD(company->twitter_image);

    CHECK_FIELD(company->favicon);
    CHECK_FIELD(company->apple_icon);
    CHECK_FIELD(company->theme_color);
    CHECK_FIELD(company->apple_touch_icon);
    CHECK_FIELD(company->robots);
    CHECK_FIELD(company->sitemap);
    CHECK_FIELD(company->logo);

    CHECK_FIELD(company->company_name);
    CHECK_FIELD(company->legal_name);
    CHECK_FIELD(company->founded);
    CHECK_FIELD(company->copyright);

    CHECK_FIELD(company->linkedin);
    CHECK_FIELD(company->facebook);
    CHECK_FIELD(company->instagram);
    CHECK_FIELD(company->twitter);
    CHECK_FIELD(company->youtube);
    CHECK_FIELD(company->github);
    CHECK_FIELD(company->tiktok);
    CHECK_FIELD(company->pinterest);

    for (int i = 0; i < company->email_count; i++)
    {
        CHECK_FIELD(company->emails[i]);

        if (company->emails[i][0] == '\0')
            return PG_STATUS_INVALID_ARGUMENT;
    }

    for (int i = 0; i < company->phone_count; i++)
    {
        CHECK_FIELD(company->phones[i]);

        if (company->phones[i][0] == '\0')
            return PG_STATUS_INVALID_ARGUMENT;
    }

    for (int i = 0; i < company->people_count; i++)
    {
        CHECK_FIELD(company->people[i].name);
        CHECK_FIELD(company->people[i].role);
        CHECK_FIELD(company->people[i].email);
        CHECK_FIELD(company->people[i].source_url);
        CHECK_FIELD(company->people[i].evidence);

        if (company->people[i].name[0] == '\0')
            return PG_STATUS_INVALID_ARGUMENT;
    }

    return PG_STATUS_OK;
}

#undef CHECK_FIELD
