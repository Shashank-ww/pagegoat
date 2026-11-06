
#include <stdlib.h>
#include <string.h>

#include "pagegoat/services/extractor/modules/metadata.h"
#include "pagegoat/services/normalize/normalize.h"

static void copy_between(
    const char *start,
    const char *end,
    char *output,
    size_t size)
{
    size_t len;

    if (start == NULL ||
        end == NULL ||
        output == NULL ||
        size == 0 ||
        end < start)
    {
        return;
    }

    len = (size_t)(end - start);

    if (len >= size)
        len = size - 1;

    memcpy(output, start, len);
    output[len] = '\0';
}

/*
 * Copy an HTML value and decode its entities.
 *
 * A temporary buffer ensures the decoder receives
 * separate input and output buffers.
 */
static void copy_decoded_between(
    const char *start,
    const char *end,
    char *output,
    size_t size)
{
    size_t len;
    char *raw;

    if (start == NULL ||
        end == NULL ||
        output == NULL ||
        size == 0 ||
        end < start)
    {
        return;
    }

    output[0] = '\0';

    len = (size_t)(end - start);

    if (len >= size)
        len = size - 1;

    raw = (char *)malloc(len + 1);

    if (raw == NULL)
    {
        copy_between(start, end, output, size);
        return;
    }

    memcpy(raw, start, len);
    raw[len] = '\0';

    /*
     * If decoding encounters an output-size warning,
     * the decoder leaves a null-terminated partial result.
     */
    pg_decode_html_entities(raw, output, size);

    free(raw);
}

static void extract_title(PGContext *ctx)
{
    const char *html = ctx->response.body;

    const char *a = strstr(html, "<title>");

    if (a == NULL)
        return;

    a += 7;

    const char *b = strstr(a, "</title>");

    if (b == NULL)
        return;

    copy_decoded_between(
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
    const char *m = strstr(html, needle);

    if (m == NULL)
        return;

    const char *c = strstr(m, "content=\"");

    if (c == NULL)
        return;

    c += 9;

    const char *e = strchr(c, '"');

    if (e == NULL)
        return;

    copy_decoded_between(
        c,
        e,
        output,
        size);
}

static void extract_language(PGContext *ctx)
{
    const char *html = ctx->response.body;

    const char *h = strstr(html, "<html");

    if (h == NULL)
        return;

    const char *l = strstr(h, "lang=\"");

    if (l == NULL)
        return;

    l += 6;

    const char *e = strchr(l, '"');

    if (e == NULL)
        return;

    copy_decoded_between(
        l,
        e,
        ctx->company.language,
        sizeof(ctx->company.language));
}

static void extract_canonical(PGContext *ctx)
{
    const char *html = ctx->response.body;

    const char *l = strstr(html, "rel=\"canonical\"");

    if (l == NULL)
        return;

    const char *h = strstr(l, "href=\"");

    if (h == NULL)
        return;

    h += 6;

    const char *e = strchr(h, '"');

    if (e == NULL)
        return;

    copy_decoded_between(
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
