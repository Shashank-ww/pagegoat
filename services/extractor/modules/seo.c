#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "pagegoat/services/extractor/modules/seo.h"

static int equals_ignore_case(
    const char *a,
    const char *b)
{
    while (*a != '\0' && *b != '\0')
    {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b))
        {
            return 0;
        }

        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}


static int equals_ignore_case_n(
    const char *a,
    const char *b,
    size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        if (tolower((unsigned char)a[i]) !=
            tolower((unsigned char)b[i]))
        {
            return 0;
        }
    }

    return 1;
}


static void copy_value(
    const char *start,
    size_t length,
    char *output,
    size_t output_size)
{
    if (output == NULL || output_size == 0)
        return;

    if (length >= output_size)
        length = output_size - 1;

    memcpy(output, start, length);
    output[length] = '\0';
}

static int is_space_char(char c)
{
    return isspace((unsigned char)c);
}

/*
 * Reads one attribute from a meta tag.
 * Supports single quotes, double quotes and unquoted values.
 */
static int get_attribute(
    const char *tag,
    const char *tag_end,
    const char *wanted_name,
    char *output,
    size_t output_size)
{
    const char *p = tag;

    while (p < tag_end && !is_space_char(*p))
        p++;

    while (p < tag_end && *p != '>')
    {
        while (p < tag_end &&
               (is_space_char(*p) || *p == '/'))
        {
            p++;
        }

        if (p >= tag_end || *p == '>')
            break;

        const char *name_start = p;

        while (p < tag_end &&
               !is_space_char(*p) &&
               *p != '=' &&
               *p != '>' &&
               *p != '/')
        {
            p++;
        }

        if (p == name_start)
        {
            p++;
            continue;
        }

        size_t name_length = (size_t)(p - name_start);

        while (p < tag_end && is_space_char(*p))
            p++;

        const char *value_start = p;
        const char *value_end = p;

        if (p < tag_end && *p == '=')
        {
            p++;

            while (p < tag_end && is_space_char(*p))
                p++;

            if (p < tag_end && (*p == '"' || *p == '\''))
            {
                char quote = *p;
                p++;

                value_start = p;

                while (p < tag_end && *p != quote)
                    p++;

                value_end = p;

                if (p < tag_end)
                    p++;
            }
            else
            {
                value_start = p;

                while (p < tag_end &&
                       !is_space_char(*p) &&
                       *p != '>')
                {
                    p++;
                }

                value_end = p;
            }
        }

        if (name_length == strlen(wanted_name) &&
            equals_ignore_case_n(
                name_start,
                wanted_name,
                name_length))

        {
            copy_value(
                value_start,
                (size_t)(value_end - value_start),
                output,
                output_size);

            return 1;
        }
    }

    return 0;
}

static void assign_if_empty(
    char *destination,
    size_t destination_size,
    const char *value)
{
    if (destination[0] == '\0' && value[0] != '\0')
    {
        copy_value(
            value,
            strlen(value),
            destination,
            destination_size);
    }
}

static void extract_meta_tag(
    const char *tag,
    const char *tag_end,
    PGCompany *company)
{
    char property[128] = {0};
    char name[128] = {0};
    char content[2048] = {0};

    get_attribute(
        tag, tag_end, "property",
        property, sizeof(property));

    get_attribute(
        tag, tag_end, "name",
        name, sizeof(name));

    if (!get_attribute(
            tag, tag_end, "content",
            content, sizeof(content)))
    {
        return;
    }

    const char *key = property[0] != '\0'
        ? property
        : name;

    if (equals_ignore_case(key, "og:title"))
    {
        assign_if_empty(
            company->og_title,
            sizeof(company->og_title),
            content);
    }
    else if (equals_ignore_case(key, "og:description"))
    {
        assign_if_empty(
            company->og_description,
            sizeof(company->og_description),
            content);
    }
    else if (equals_ignore_case(key, "og:image"))
    {
        assign_if_empty(
            company->og_image,
            sizeof(company->og_image),
            content);
    }
    else if (equals_ignore_case(key, "og:url"))
    {
        assign_if_empty(
            company->og_url,
            sizeof(company->og_url),
            content);
    }
    else if (equals_ignore_case(key, "twitter:card"))
    {
        assign_if_empty(
            company->twitter_card,
            sizeof(company->twitter_card),
            content);
    }
    else if (equals_ignore_case(key, "twitter:title"))
    {
        assign_if_empty(
            company->twitter_title,
            sizeof(company->twitter_title),
            content);
    }
    else if (equals_ignore_case(key, "twitter:description"))
    {
        assign_if_empty(
            company->twitter_description,
            sizeof(company->twitter_description),
            content);
    }
    else if (equals_ignore_case(key, "twitter:image"))
    {
        assign_if_empty(
            company->twitter_image,
            sizeof(company->twitter_image),
            content);
    }
}

PGStatus pg_extract_seo(PGContext *ctx)
{
    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    if (ctx->response.body == NULL)
        return PG_STATUS_PARSE_ERROR;

    const char *html = ctx->response.body;
    const char *p = html;

    while (*p != '\0')
    {
        if (*p != '<')
        {
            p++;
            continue;
        }

        const char *tag = p;
        p++;

        if (tolower((unsigned char)*p) != 'm')
        {
            continue;
        }

        if (tolower((unsigned char)p[1]) != 'e' ||
            tolower((unsigned char)p[2]) != 't' ||
            tolower((unsigned char)p[3]) != 'a')
        {
            continue;
        }

        char next = p[4];

        if (next != '>' && next != '/' &&
            !is_space_char(next))
        {
            continue;
        }

        const char *tag_end = p + 4;
        char quote = '\0';

        while (*tag_end != '\0')
        {
            if (quote != '\0')
            {
                if (*tag_end == quote)
                    quote = '\0';
            }
            else if (*tag_end == '"' || *tag_end == '\'')
            {
                quote = *tag_end;
            }
            else if (*tag_end == '>')
            {
                break;
            }

            tag_end++;
        }

        if (*tag_end == '\0')
            break;

        extract_meta_tag(tag, tag_end, &ctx->company);

        p = tag_end + 1;
    }

    return PG_STATUS_OK;
}