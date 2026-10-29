#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#include "pagegoat/coregoat/context.h"
#include "pagegoat/platform/http.h"
#include "pagegoat/services/extractor/modules/branding.h"

static int equal_ci_n(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] == '\0' || tolower((unsigned char)a[i]) != tolower((unsigned char)b[i]))
            return 0;
    }
    return 1;
}

static const char *find_ci(const char *text, const char *needle)
{
    size_t n;
    if (text == NULL || needle == NULL || needle[0] == '\0') return NULL;
    n = strlen(needle);
    for (const char *p = text; *p; p++)
        if (equal_ci_n(p, needle, n)) return p;
    return NULL;
}

static void copy_trimmed(const char *start, const char *end,
                         char *out, size_t out_size)
{
    size_t n;
    while (start < end && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)end[-1])) end--;
    n = (size_t)(end - start);
    if (n >= out_size) n = out_size - 1;
    memcpy(out, start, n);
    out[n] = '\0';
}

/* Reads quoted or unquoted HTML attributes without assuming attribute order. */
static int get_attr(const char *tag, const char *tag_end,
                    const char *wanted, char *out, size_t out_size)
{
    const char *p = tag;
    size_t wanted_len = strlen(wanted);
    out[0] = '\0';
    while (p < tag_end && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') p++;
    while (p < tag_end)
    {
        const char *name;
        const char *value_start;
        const char *value_end;
        char quote = '\0';
        while (p < tag_end && (isspace((unsigned char)*p) || *p == '/')) p++;
        if (p >= tag_end || *p == '>') break;
        name = p;
        while (p < tag_end && !isspace((unsigned char)*p) && *p != '=' && *p != '>' && *p != '/') p++;
        if (p == name) { p++; continue; }
        size_t name_len = (size_t)(p - name);
        while (p < tag_end && isspace((unsigned char)*p)) p++;
        if (p >= tag_end || *p != '=')
        {
            if (name_len == wanted_len && equal_ci_n(name, wanted, wanted_len)) return 0;
            continue;
        }
        p++;
        while (p < tag_end && isspace((unsigned char)*p)) p++;
        if (p < tag_end && (*p == '\'' || *p == '"')) quote = *p++;
        value_start = p;
        if (quote)
        {
            while (p < tag_end && *p != quote) p++;
            value_end = p;
            if (p < tag_end) p++;
        }
        else
        {
            while (p < tag_end && !isspace((unsigned char)*p) && *p != '>') p++;
            value_end = p;
        }
        if (name_len == wanted_len && equal_ci_n(name, wanted, wanted_len))
        {
            copy_trimmed(value_start, value_end, out, out_size);
            return out[0] != '\0';
        }
    }
    return 0;
}

static void resolve_url(const char *base_url, const char *value,
                        char *out, size_t out_size)
{
    CURLU *url;
    char *full = NULL;
    out[0] = '\0';
    if (value == NULL || value[0] == '\0') return;
    url = curl_url();
    if (url == NULL) return;
    if (curl_url_set(url, CURLUPART_URL, base_url, 0) == CURLUE_OK &&
        curl_url_set(url, CURLUPART_URL, value, 0) == CURLUE_OK &&
        curl_url_get(url, CURLUPART_URL, &full, 0) == CURLUE_OK &&
        full != NULL && strlen(full) < out_size)
        snprintf(out, out_size, "%s", full);
    curl_free(full);
    curl_url_cleanup(url);
}

static void extract_meta(const char *html, const char *key_name,
                         const char *key_value, char *out, size_t out_size)
{
    const char *p = html;
    while ((p = find_ci(p, "<meta")) != NULL)
    {
        const char *end = strchr(p, '>');
        char name[128], property[128], content[2048];
        if (end == NULL) return;
        name[0] = property[0] = content[0] = '\0';
        (void)get_attr(p, end, "name", name, sizeof(name));
        (void)get_attr(p, end, "property", property, sizeof(property));
        if (get_attr(p, end, "content", content, sizeof(content)))
        {
            const char *actual = key_name[0] == 'n' ? name : property;
            int matches = strlen(actual) == strlen(key_value) &&
                          equal_ci_n(actual, key_value, strlen(key_value));
            if (!matches)
                matches = strlen(name) == strlen(key_value) &&
                          equal_ci_n(name, key_value, strlen(key_value));
            if (!matches)
                matches = strlen(property) == strlen(key_value) &&
                          equal_ci_n(property, key_value, strlen(key_value));
            if (matches)
            {
                snprintf(out, out_size, "%s", content);
                return;
            }
        }
        p = end + 1;
    }
}

static void extract_link(const char *html, const char *rel_token,
                         const char *base_url, char *out, size_t out_size)
{
    const char *p = html;
    while ((p = find_ci(p, "<link")) != NULL)
    {
        const char *end = strchr(p, '>');
        char rel[256], href[1024];
        if (end == NULL) return;
        rel[0] = href[0] = '\0';
        (void)get_attr(p, end, "rel", rel, sizeof(rel));
        if (find_ci(rel, rel_token) != NULL && get_attr(p, end, "href", href, sizeof(href)))
        {
            resolve_url(base_url, href, out, out_size);
            if (out[0] != '\0') return;
        }
        p = end + 1;
    }
}

static int font_exists(const PGCompany *company, const char *font)
{
    for (int i = 0; i < company->font_count; i++)
        if (strlen(company->fonts[i]) == strlen(font) &&
            equal_ci_n(company->fonts[i], font, strlen(font))) return 1;
    return 0;
}

static int is_generic_or_secondary_font(const char *font)
{
    static const char *const ignored[] = {
        "serif", "sans-serif", "monospace", "cursive", "fantasy",
        "system-ui", "ui-serif", "ui-sans-serif", "ui-monospace",
        "ui-rounded", "emoji", "math", "fangsong", "inherit",
        "initial", "unset", "revert", "default"
    };
    size_t length = strlen(font);

    if (length == 0) return 1;

    /* CSS-escaped names are often locale-specific variants; omit them. */
    if (strchr(font, '\\') != NULL) return 1;

    /* Icon fonts and localized variants are not the site's main text fonts. */
    if (find_ci(font, "icon") != NULL ||
        find_ci(font, "SF Pro AR") != NULL ||
        find_ci(font, "SF Pro JP") != NULL ||
        find_ci(font, "SF Pro KR") != NULL ||
        find_ci(font, "SF Pro TH") != NULL ||
        find_ci(font, "SF Pro SC") != NULL ||
        find_ci(font, "SF Pro Gulf") != NULL ||
        find_ci(font, "Hiragino") != NULL ||
        find_ci(font, "Meiryo") != NULL ||
        find_ci(font, "PingFang") != NULL ||
        find_ci(font, "Malgun") != NULL ||
        find_ci(font, "Gulim") != NULL ||
        find_ci(font, "Dotum") != NULL ||
        find_ci(font, "Apple Gothic") != NULL)
        return 1;

    for (size_t i = 0; i < sizeof(ignored) / sizeof(ignored[0]); i++)
        if (length == strlen(ignored[i]) && equal_ci_n(font, ignored[i], length))
            return 1;

    return 0;
}

static void add_font(PGCompany *company, const char *start, const char *end)
{
    char font[PG_FONT_SIZE];
    copy_trimmed(start, end, font, sizeof(font));
    size_t font_len = strlen(font);
    if (font_len >= 2 && ((font[0] == '\'' && font[font_len - 1] == '\'') ||
                          (font[0] == '"' && font[font_len - 1] == '"')))
    {
        memmove(font, font + 1, font_len - 2);
        font[font_len - 2] = '\0';
    }
    if (is_generic_or_secondary_font(font) || font_exists(company, font) ||
        company->font_count >= 4 || company->font_count >= PG_MAX_FONTS) return;
    snprintf(company->fonts[company->font_count], PG_FONT_SIZE, "%s", font);
    company->font_count++;
}

/* Keep only the first (primary) family in each CSS font-family declaration. */
static void scan_font_families(PGCompany *company, const char *css)
{
    const char *p = css;
    while (company->font_count < 4 && (p = find_ci(p, "font-family")) != NULL)
    {
        const char *colon = p + strlen("font-family");
        const char *end;
        const char *first_end;
        char quote = '\0';

        while (*colon && isspace((unsigned char)*colon)) colon++;
        if (*colon != ':') { p += strlen("font-family"); continue; }

        const char *value = ++colon;
        end = value;
        while (*end)
        {
            if (quote)
            {
                if (*end == quote) quote = '\0';
            }
            else if (*end == '\'' || *end == '"') quote = *end;
            else if (*end == ';' || *end == '}' || *end == '<' || *end == '>') break;
            end++;
        }

        first_end = value;
        quote = '\0';
        while (first_end < end)
        {
            if (quote)
            {
                if (*first_end == quote) quote = '\0';
            }
            else if (*first_end == '\'' || *first_end == '"') quote = *first_end;
            else if (*first_end == ',') break;
            first_end++;
        }
        add_font(company, value, first_end);
        p = end > p ? end : p + strlen("font-family");
    }
}

static void extract_favicon(const char *html, const char *base_url,
                            char *out, size_t out_size)
{
    const char *p = html;
    while ((p = find_ci(p, "<link")) != NULL)
    {
        const char *end = strchr(p, '>');
        char rel[256], href[1024];
        if (end == NULL) return;
        rel[0] = href[0] = '\0';
        (void)get_attr(p, end, "rel", rel, sizeof(rel));
        if (find_ci(rel, "icon") != NULL &&
            find_ci(rel, "apple-touch-icon") == NULL &&
            get_attr(p, end, "href", href, sizeof(href)))
        {
            resolve_url(base_url, href, out, out_size);
            if (out[0] != '\0') return;
        }
        p = end + 1;
    }
}

static void extract_explicit_logo(const char *html, const char *base_url,
                                  char *out, size_t out_size)
{
    const char *p = html;
    while ((p = find_ci(p, "<img")) != NULL)
    {
        const char *end = strchr(p, '>');
        char itemprop[128], src[1024];
        if (end == NULL) return;
        itemprop[0] = src[0] = '\0';
        (void)get_attr(p, end, "itemprop", itemprop, sizeof(itemprop));
        if (find_ci(itemprop, "logo") != NULL &&
            get_attr(p, end, "src", src, sizeof(src)))
        {
            resolve_url(base_url, src, out, out_size);
            if (out[0] != '\0') return;
        }
        p = end + 1;
    }

    /* Some sites expose the logo as Schema.org metadata. */
    p = html;
    while ((p = find_ci(p, "<meta")) != NULL)
    {
        const char *end = strchr(p, '>');
        char itemprop[128], content[1024];
        if (end == NULL) return;
        itemprop[0] = content[0] = '\0';
        (void)get_attr(p, end, "itemprop", itemprop, sizeof(itemprop));
        if (find_ci(itemprop, "logo") != NULL &&
            get_attr(p, end, "content", content, sizeof(content)))
        {
            resolve_url(base_url, content, out, out_size);
            if (out[0] != '\0') return;
        }
        p = end + 1;
    }
}

static void extract_fonts(PGContext *ctx, const char *html, const char *base_url)
{
    const char *p;
    int fetched_stylesheets = 0;
    scan_font_families(&ctx->company, html);

    /* Bound network overhead: inspect at most two linked stylesheets. */
    p = html;
    while (fetched_stylesheets < 1 && (p = find_ci(p, "<link")) != NULL)
    {
        const char *end = strchr(p, '>');
        char rel[256], href[1024], stylesheet_url[1536];
        if (end == NULL) break;
        rel[0] = href[0] = stylesheet_url[0] = '\0';
        (void)get_attr(p, end, "rel", rel, sizeof(rel));
        if (find_ci(rel, "stylesheet") != NULL &&
            get_attr(p, end, "href", href, sizeof(href)))
        {
            PGHttpResponse response;
            resolve_url(base_url, href, stylesheet_url, sizeof(stylesheet_url));
            if (stylesheet_url[0] && pg_http_get(stylesheet_url, &response) == 0)
            {
                if (response.status >= 200 && response.status < 400 && response.body)
                    scan_font_families(&ctx->company, response.body);
                pg_http_free(&response);
            }
            fetched_stylesheets++;
        }
        p = end + 1;
    }
}

PGStatus pg_extract_branding(PGContext *ctx)
{
    const char *html;
    const char *base_url;
    char explicit_logo[1024] = "";
    if (ctx == NULL) return PG_STATUS_INVALID_ARGUMENT;
    html = ctx->response.body;
    if (html == NULL) return PG_STATUS_PARSE_ERROR;
    base_url = ctx->response.final_url[0] ? ctx->response.final_url : ctx->url;

    extract_favicon(html, base_url, ctx->company.favicon, sizeof(ctx->company.favicon));
    if (ctx->company.favicon[0] == '\0')
        resolve_url(base_url, "/favicon.ico", ctx->company.favicon, sizeof(ctx->company.favicon));
    extract_link(html, "apple-touch-icon", base_url,
                 ctx->company.apple_touch_icon, sizeof(ctx->company.apple_touch_icon));
    snprintf(ctx->company.apple_icon, sizeof(ctx->company.apple_icon), "%s",
             ctx->company.apple_touch_icon);

    extract_meta(html, "name", "theme-color", ctx->company.theme_color,
                 sizeof(ctx->company.theme_color));

    /* Prefer an explicit logo link before generic social-sharing images. */
    extract_link(html, "logo", base_url, explicit_logo, sizeof(explicit_logo));
    if (explicit_logo[0] == '\0')
        extract_explicit_logo(html, base_url, explicit_logo, sizeof(explicit_logo));
    if (explicit_logo[0] && strlen(explicit_logo) < sizeof(ctx->company.logo))
    {
        memcpy(ctx->company.logo, explicit_logo, strlen(explicit_logo) + 1);
    }
    else
    {
        char social_image[2048] = "";
        char resolved[2048] = "";
        extract_meta(html, "property", "og:image", social_image, sizeof(social_image));
        if (social_image[0] == '\0')
            extract_meta(html, "property", "twitter:image", social_image, sizeof(social_image));
        if (social_image[0])
        {
            resolve_url(base_url, social_image, resolved, sizeof(resolved));
            if (resolved[0] && strlen(resolved) < sizeof(ctx->company.logo))
                memcpy(ctx->company.logo, resolved, strlen(resolved) + 1);
        }
    }

    extract_fonts(ctx, html, base_url);
    return PG_STATUS_OK;
}
