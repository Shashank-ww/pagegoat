
#include <stdio.h>
#include <string.h>

#include "pagegoat/services/export.h"

static void write_json_string(FILE *file, const char *value)
{
    if (value == NULL || value[0] == '\0')
    {
        fputs("null", file);
        return;
    }

    fputc('"', file);

    for (const unsigned char *p = (const unsigned char *)value;
         *p != '\0';
         p++)
    {
        switch (*p)
        {
            case '"':
                fputs("\\\"", file);
                break;

            case '\\':
                fputs("\\\\", file);
                break;

            case '\b':
                fputs("\\b", file);
                break;

            case '\f':
                fputs("\\f", file);
                break;

            case '\n':
                fputs("\\n", file);
                break;

            case '\r':
                fputs("\\r", file);
                break;

            case '\t':
                fputs("\\t", file);
                break;

            default:
                if (*p < 0x20)
                {
                    fprintf(file, "\\u%04x", (unsigned int)*p);
                }
                else
                {
                    fputc(*p, file);
                }
                break;
        }
    }

    fputc('"', file);
}

static void write_json_array(
    FILE *file,
    char values[][256],
    int count)
{
    fputc('[', file);

    for (int i = 0; i < count; i++)
    {
        if (i > 0)
            fputc(',', file);

        write_json_string(file, values[i]);
    }

    fputc(']', file);
}

static void write_phone_array(
    FILE *file,
    char values[][64],
    int count)
{
    fputc('[', file);

    for (int i = 0; i < count; i++)
    {
        if (i > 0)
            fputc(',', file);

        write_json_string(file, values[i]);
    }

    fputc(']', file);
}

PGStatus pg_export_company_json(
    const PGContext *ctx,
    const char *path)
{
    FILE *file;
    const PGCompany *company;
    const char *name;

    if (ctx == NULL || path == NULL || path[0] == '\0')
        return PG_STATUS_INVALID_ARGUMENT;

    company = &ctx->company;

    if (company->email_count < 0 ||
        company->email_count > PG_MAX_EMAILS ||
        company->phone_count < 0 ||
        company->phone_count > PG_MAX_PHONES)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    file = fopen(path, "wb");

    if (file == NULL)
        return PG_STATUS_EXPORT_ERROR;

    name = company->company_name[0] != '\0'
        ? company->company_name
        : company->title;

    fputs("{\n  \"schema_version\": \"1.0\",\n  \"website\": ", file);
    write_json_string(file, ctx->url);

    fputs(",\n  \"company\": {\n    \"name\": ", file);
    write_json_string(file, name);

    fputs(",\n    \"title\": ", file);
    write_json_string(file, company->title);

    fputs(",\n    \"description\": ", file);
    write_json_string(file, company->description);

    fputs(",\n    \"keywords\": ", file);
    write_json_string(file, company->keywords);

    fputs(",\n    \"language\": ", file);
    write_json_string(file, company->language);

    fputs(",\n    \"canonical_url\": ", file);
    write_json_string(file, company->canonical);

    fputs(",\n    \"contact\": {\n      \"emails\": ", file);
    write_json_array(file, (char (*)[256])company->emails,
                     company->email_count);

    fputs(",\n      \"phones\": ", file);
    write_phone_array(file, (char (*)[64])company->phones,
                      company->phone_count);
    fputs("\n    },\n    \"social\": {\n      \"linkedin\": ", file);
    write_json_string(file, company->linkedin);

    fputs(",\n      \"facebook\": ", file);
    write_json_string(file, company->facebook);

    fputs(",\n      \"instagram\": ", file);
    write_json_string(file, company->instagram);

    fputs(",\n      \"twitter\": ", file);
    write_json_string(file, company->twitter);

    fputs(",\n      \"youtube\": ", file);
    write_json_string(file, company->youtube);

    fputs(",\n      \"github\": ", file);
    write_json_string(file, company->github);

    fputs(",\n      \"tiktok\": ", file);
    write_json_string(file, company->tiktok);

    fputs(",\n      \"pinterest\": ", file);
    write_json_string(file, company->pinterest);

    fputs("\n    },\n    \"branding\": {\n      \"logo_url\": ", file);
    write_json_string(file, company->logo);

    fputs(",\n      \"favicon_url\": ", file);
    write_json_string(file, company->favicon);

    fputs(",\n      \"apple_icon_url\": ", file);
    write_json_string(file, company->apple_icon);

    fputs(",\n      \"apple_touch_icon_url\": ", file);
    write_json_string(file, company->apple_touch_icon);

    fputs(",\n      \"theme_color\": ", file);
    write_json_string(file, company->theme_color);

    fputs("\n    },\n    \"seo\": {\n      \"generator\": ", file);
    write_json_string(file, company->generator);

    fputs(",\n      \"robots\": ", file);
    write_json_string(file, company->robots);

    fputs(",\n      \"sitemap\": ", file);
    write_json_string(file, company->sitemap);

    fputs("\n    },\n    \"organization\": {\n      \"legal_name\": ", file);
    write_json_string(file, company->legal_name);

    fputs(",\n      \"founded\": ", file);
    write_json_string(file, company->founded);

    fputs(",\n      \"copyright\": ", file);
    write_json_string(file, company->copyright);

    fputs("\n    }\n  }\n}\n", file);

    if (ferror(file))
    {
        fclose(file);
        return PG_STATUS_EXPORT_ERROR;
    }

    if (fclose(file) != 0)
        return PG_STATUS_EXPORT_ERROR;

    return PG_STATUS_OK;
}
