#include <string.h>

#include "pagegoat/services/extractor/modules/branding.h"

static void copy_between(
    const char *start,
    const char *end,
    char *output,
    size_t size)
{
    size_t len = (size_t)(end - start);

    if (len >= size)
        len = size - 1;

    memcpy(output, start, len);
    output[len] = '\0';
}

static void extract_href(
    const char *html,
    const char *needle,
    char *output,
    size_t size)
{
    char *p = strstr((char *)html, needle);

    if (p == NULL)
        return;

    char *href = strstr(p, "href=\"");

    if (href == NULL)
        return;

    href += 6;

    char *end = strchr(href, '"');

    if (end == NULL)
        return;

    copy_between(
        href,
        end,
        output,
        size);
}

static void extract_content(
    const char *html,
    const char *needle,
    char *output,
    size_t size)
{
    char *p = strstr((char *)html, needle);

    if (p == NULL)
        return;

    char *content = strstr(p, "content=\"");

    if (content == NULL)
        return;

    content += 9;

    char *end = strchr(content, '"');

    if (end == NULL)
        return;

    copy_between(
        content,
        end,
        output,
        size);
}

PGStatus
pg_extract_branding(PGContext *ctx)
{
    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    const char *html = ctx->response.body;

    if (html == NULL)
        return PG_STATUS_PARSE_ERROR;

    /*
        favicon.ico
    */

    extract_href(
        html,
        "rel=\"icon\"",
        ctx->company.favicon,
        sizeof(ctx->company.favicon));

    /*
        shortcut icon
    */

    if (ctx->company.favicon[0] == '\0')
    {
        extract_href(
            html,
            "shortcut icon",
            ctx->company.favicon,
            sizeof(ctx->company.favicon));
    }

    /*
        Apple Touch Icon
    */

    extract_href(
        html,
        "apple-touch-icon",
        ctx->company.apple_touch_icon,
        sizeof(ctx->company.apple_touch_icon));

    /*
        Theme Color
    */

    extract_content(
        html,
        "theme-color",
        ctx->company.theme_color,
        sizeof(ctx->company.theme_color));

    /*
        OpenGraph Logo
    */

    extract_content(
        html,
        "property=\"og:image\"",
        ctx->company.logo,
        sizeof(ctx->company.logo));

    /*
        Twitter image fallback
    */

    if (ctx->company.logo[0] == '\0')
    {
        extract_content(
            html,
            "twitter:image",
            ctx->company.logo,
            sizeof(ctx->company.logo));
    }

    return PG_STATUS_OK;
}