
#ifndef PAGEGOAT_COMPANY_H
#define PAGEGOAT_COMPANY_H

#include "pagegoat/coregoat/status.h"

#define PG_MAX_EMAILS  32
#define PG_MAX_PHONES  32
#define PG_MAX_SOCIALS 32

typedef struct
{
    char title[256];
    char description[1024];
    char keywords[1024];
    char language[32];
    char generator[256];

    char canonical[512];

    char favicon[512];
    char apple_icon[512];
    char theme_color[64];
    char apple_touch_icon[512];
    char robots[512];
    char sitemap[512];

    char logo[512];

    char company_name[256];
    char legal_name[256];
    char founded[64];
    char copyright[256];

    char emails[PG_MAX_EMAILS][256];
    int email_count;

    char phones[PG_MAX_PHONES][64];
    int phone_count;

    char linkedin[512];
    char facebook[512];
    char instagram[512];
    char twitter[512];
    char youtube[512];
    char github[512];
    char tiktok[512];
    char pinterest[512];

} PGCompany;

PGStatus pg_company_validate(const PGCompany *company);

#endif
