#include <string.h>

#include "pagegoat/services/extractor/modules/metadata.h"

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

static void extract_title(PGContext *ctx)
{
    const char *html = ctx->response.body;

    char *a = strstr((char *)html, "<title>");

    if (a == NULL)
        return;

    a += 7;

    char *b = strstr(a, "</title>");

    if (b == NULL)
        return;

    copy_between(
        a,
        b,
        ctx->company.title,
        sizeof(ctx->company.title));
}

static void extract_meta(
    const char *html,
    const char *needle,
    char *output,
    size_t size)
{
    char *m = strstr((char *)html, needle);

    if (m == NULL)
        return;

    char *c = strstr(m, "content=\"");

    if (c == NULL)
        return;

    c += 9;

    char *e = strchr(c, '"');

    if (e == NULL)
        return;

    copy_between(
        c,
        e,
        output,
        size);
}

static void extract_language(PGContext *ctx)
{
    const char *html = ctx->response.body;

    char *h = strstr((char *)html, "<html");

    if (h == NULL)
        return;

    char *l = strstr(h, "lang=\"");

    if (l == NULL)
        return;

    l += 6;

    char *e = strchr(l, '"');

    if (e == NULL)
        return;

    copy_between(
        l,
        e,
        ctx->company.language,
        sizeof(ctx->company.language));
}

static void extract_canonical(PGContext *ctx)
{
    const char *html = ctx->response.body;

    char *l = strstr((char *)html, "rel=\"canonical\"");

    if (l == NULL)
        return;

    char *h = strstr(l, "href=\"");

    if (h == NULL)
        return;

    h += 6;

    char *e = strchr(h, '"');

    if (e == NULL)
        return;

    copy_between(
        h,
        e,
        ctx->company.canonical,
        sizeof(ctx->company.canonical));
}

PGStatus
pg_extract_metadata(PGContext *ctx)
{
    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    if (ctx->response.body == NULL)
        return PG_STATUS_PARSE_ERROR;

    extract_title(ctx);

    extract_meta(
        ctx->response.body,
        "name=\"description\"",
        ctx->company.description,
        sizeof(ctx->company.description));

    extract_meta(
        ctx->response.body,
        "name=\"keywords\"",
        ctx->company.keywords,
        sizeof(ctx->company.keywords));

    extract_meta(
        ctx->response.body,
        "name=\"generator\"",
        ctx->company.generator,
        sizeof(ctx->company.generator));

    extract_language(ctx);

    extract_canonical(ctx);

    return PG_STATUS_OK;
}