
#ifndef PAGEGOAT_COMPANY_H
#define PAGEGOAT_COMPANY_H

#include "pagegoat/coregoat/status.h"

#define PG_MAX_EMAILS  32
#define PG_MAX_PHONES  32
#define PG_MAX_SOCIALS 32

#define PG_MAX_PEOPLE 32
#define PG_PERSON_NAME_SIZE 256
#define PG_PERSON_ROLE_SIZE 128
#define PG_PERSON_URL_SIZE 512
#define PG_PERSON_EVIDENCE_SIZE 512

#define PG_MAX_TECHNOLOGIES 64
#define PG_TECHNOLOGY_SIZE 128
#define PG_MAX_FONTS 24
#define PG_FONT_SIZE 128

typedef struct
{
    char name[PG_PERSON_NAME_SIZE];
    char role[PG_PERSON_ROLE_SIZE];
    char email[256];
    char source_url[PG_PERSON_URL_SIZE];
    char evidence[PG_PERSON_EVIDENCE_SIZE];
} PGPerson;

typedef struct
{
    char title[256];
    char description[1024];
    char keywords[1024];
    char language[32];
    char generator[256];

    char canonical[512];

    /* Open Graph metadata */
    char og_title[256];
    char og_description[1024];
    char og_image[512];
    char og_url[512];

    /* Twitter/X card metadata */
    char twitter_card[128];
    char twitter_title[256];
    char twitter_description[1024];
    char twitter_image[512];

    char favicon[512];
    char apple_icon[512];
    char theme_color[64];
    char apple_touch_icon[512];
    char robots[512];
    char sitemap[512];

    char logo[512];
    char fonts[PG_MAX_FONTS][PG_FONT_SIZE];
    int font_count;

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

    char technologies[PG_MAX_TECHNOLOGIES][PG_TECHNOLOGY_SIZE];
    int technology_count;

PGPerson people[PG_MAX_PEOPLE];
    int people_count;

} PGCompany;

PGStatus pg_company_validate(const PGCompany *company);

#endif
