#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "pagegoat/coregoat/context.h"
#include "pagegoat/platform/http.h"
#include "pagegoat/platform/logger.h"
#include "pagegoat/services/crawl.h"
#include "pagegoat/services/extractor/modules/contact.h"
#include "pagegoat/services/extractor/modules/social.h"
#include "pagegoat/services/normalize/normalize.h"

#define PG_CRAWL_MAX_CANDIDATES 96
#define PG_CRAWL_MAX_CONTACT_PAGES 9
#define PG_CRAWL_MAX_URL 1024

typedef struct
{
    char url[PG_CRAWL_MAX_URL];
    int priority;
} PGCrawlCandidate;

static int contains_ci(const char *text, const char *needle)
{
    size_t n;

    if (text == NULL || needle == NULL)
        return 0;

    n = strlen(needle);
    if (n == 0)
        return 1;

    for (; *text; text++)
    {
        size_t i = 0;
        while (i < n && text[i] != '\0' &&
               tolower((unsigned char)text[i]) ==
               tolower((unsigned char)needle[i]))
            i++;

        if (i == n)
            return 1;
    }

    return 0;
}

/*
 * Page priority follows the locked PageGoat crawl order:
 * Contact -> About -> Leadership -> Investor Relations ->
 * Careers -> Products -> Media -> Blog -> Sitemap.
 */
static int classify_page(const char *text)
{
    if (contains_ci(text, "contact") ||
        contains_ci(text, "get-in-touch") ||
        contains_ci(text, "reach-us"))
        return 1;

    if (contains_ci(text, "about") ||
        contains_ci(text, "our-story") ||
        contains_ci(text, "company"))
        return 2;

    if (contains_ci(text, "leadership") ||
        contains_ci(text, "management") ||
        contains_ci(text, "executive") ||
        contains_ci(text, "founder") ||
        contains_ci(text, "team") ||
        contains_ci(text, "people"))
        return 3;

    if (contains_ci(text, "investor") ||
        contains_ci(text, "shareholder") ||
        contains_ci(text, "/ir"))
        return 4;

    if (contains_ci(text, "career") ||
        contains_ci(text, "jobs") ||
        contains_ci(text, "work-with-us"))
        return 5;

    if (contains_ci(text, "product") ||
        contains_ci(text, "solutions"))
        return 6;

    if (contains_ci(text, "media") ||
        contains_ci(text, "press") ||
        contains_ci(text, "newsroom"))
        return 7;

    if (contains_ci(text, "blog") ||
        contains_ci(text, "article") ||
        contains_ci(text, "insights"))
        return 8;

    if (contains_ci(text, "sitemap"))
        return 9;

    return 0;
}

/* Reject linked files that cannot be useful HTML contact pages. */
static int is_non_html_asset_url(const char *url)
{
    static const char *blocked_extensions[] = {
        ".7z", ".avi", ".css", ".doc", ".docx", ".eot", ".flv",
        ".gif", ".gz", ".ico", ".jpeg", ".jpg", ".js", ".m4a",
        ".m4s", ".m4v", ".map", ".mov", ".mp3", ".mp4", ".mpeg",
        ".mpg", ".m3u8", ".ogg", ".otf", ".pdf", ".png", ".ppt",
        ".pptx", ".rar", ".svg", ".tar", ".ts", ".ttf", ".wav",
        ".webm", ".webp", ".wmv", ".woff", ".woff2", ".xls",
        ".xlsx", ".zip"
    };
    const char *path_end;
    const char *last_slash;
    const char *last_dot;

    if (url == NULL || url[0] == '\0')
        return 1;

    path_end = strpbrk(url, "?#");
    if (path_end == NULL)
        path_end = url + strlen(url);

    last_slash = NULL;
    for (const char *p = url; p < path_end; p++)
    {
        if (*p == '/')
            last_slash = p;
    }

    last_dot = NULL;
    for (const char *p = last_slash != NULL ? last_slash + 1 : url;
         p < path_end; p++)
    {
        if (*p == '.')
            last_dot = p;
    }

    if (last_dot == NULL)
        return 0;

    for (size_t i = 0;
         i < sizeof(blocked_extensions) / sizeof(blocked_extensions[0]); i++)
    {
        size_t extension_length = strlen(blocked_extensions[i]);
        if ((size_t)(path_end - last_dot) == extension_length &&
            curl_strnequal(last_dot, blocked_extensions[i], extension_length))
            return 1;
    }

    return 0;
}

static int get_url_part(CURLU *url, CURLUPart part, char *out, size_t cap)
{
    char *value = NULL;
    CURLUcode code = curl_url_get(url, part, &value, CURLU_DEFAULT_PORT);

    if (code != CURLUE_OK || value == NULL)
    {
        if (value != NULL)
            curl_free(value);
        return -1;
    }

    size_t n = strlen(value);
    if (n >= cap)
    {
        curl_free(value);
        return -1;
    }

    memcpy(out, value, n + 1);
    curl_free(value);
    return 0;
}

/* Resolve a discovered href against the current page and keep it same-origin. */
static int resolve_same_origin(
    const char *base_url,
    const char *href,
    char *resolved,
    size_t resolved_size)
{
    CURLU *base = NULL;
    CURLU *candidate = NULL;
    char base_scheme[16], candidate_scheme[16];
    char base_host[256], candidate_host[256];
    char base_port[16], candidate_port[16];
    char *full_url = NULL;
    int ok = 0;

    if (base_url == NULL || href == NULL ||
        href[0] == '\0' || resolved == NULL || resolved_size == 0)
        return -1;

    /* Ignore non-navigation links and mail/telephone links. */
    if (href[0] == '#' ||
        contains_ci(href, "javascript:") ||
        contains_ci(href, "mailto:") ||
        contains_ci(href, "tel:") ||
        contains_ci(href, "data:"))
        return -1;

    base = curl_url();
    candidate = curl_url();

    if (base == NULL || candidate == NULL)
        goto done;

    if (curl_url_set(base, CURLUPART_URL, base_url, 0) != CURLUE_OK ||
        curl_url_set(candidate, CURLUPART_URL, base_url, 0) != CURLUE_OK ||
        curl_url_set(candidate, CURLUPART_URL, href, 0) != CURLUE_OK)
        goto done;

    /* Query strings and fragments are not needed for contact discovery. */
    (void)curl_url_set(candidate, CURLUPART_QUERY, NULL, 0);
    (void)curl_url_set(candidate, CURLUPART_FRAGMENT, NULL, 0);

    if (get_url_part(base, CURLUPART_SCHEME,
                     base_scheme, sizeof(base_scheme)) != 0 ||
        get_url_part(candidate, CURLUPART_SCHEME,
                     candidate_scheme, sizeof(candidate_scheme)) != 0 ||
        get_url_part(base, CURLUPART_HOST,
                     base_host, sizeof(base_host)) != 0 ||
        get_url_part(candidate, CURLUPART_HOST,
                     candidate_host, sizeof(candidate_host)) != 0 ||
        get_url_part(base, CURLUPART_PORT,
                     base_port, sizeof(base_port)) != 0 ||
        get_url_part(candidate, CURLUPART_PORT,
                     candidate_port, sizeof(candidate_port)) != 0)
        goto done;

    if (strcmp(base_scheme, candidate_scheme) != 0 ||
        curl_strequal(base_host, candidate_host) == 0 ||
        strcmp(base_port, candidate_port) != 0)
        goto done;

    if (curl_url_get(candidate, CURLUPART_URL,
                     &full_url, 0) != CURLUE_OK ||
        full_url == NULL)
        goto done;

    if (strlen(full_url) >= resolved_size ||
        is_non_html_asset_url(full_url) ||
        pg_validate_url(full_url) != PG_STATUS_OK)
        goto done;

    strcpy(resolved, full_url);
    ok = 1;

done:
    if (full_url != NULL)
        curl_free(full_url);
    if (base != NULL)
        curl_url_cleanup(base);
    if (candidate != NULL)
        curl_url_cleanup(candidate);

    return ok ? 0 : -1;
}

static int candidate_exists(
    const PGCrawlCandidate *items,
    size_t count,
    const char *url)
{
    for (size_t i = 0; i < count; i++)
    {
        if (strcmp(items[i].url, url) == 0)
            return 1;
    }

    return 0;
}

static void add_candidate(
    PGCrawlCandidate *items,
    size_t *count,
    const char *url,
    int priority)
{
    if (priority < 1 || priority > 9 ||
        *count >= PG_CRAWL_MAX_CANDIDATES ||
        candidate_exists(items, *count, url))
        return;

    PGCrawlCandidate *item = &items[*count];
    snprintf(item->url, sizeof(item->url), "%s", url);
    item->priority = priority;
    (*count)++;
}

static void collect_link_candidates(
    const char *html,
    const char *base_url,
    PGCrawlCandidate *items,
    size_t *count)
{
    const char *p = html;

    while (p != NULL && *p != '\0' &&
           *count < PG_CRAWL_MAX_CANDIDATES)
    {
        const char *anchor = NULL;
        const char *tag_end;
        const char *href_attr = NULL;
        const char *value_end;
        char href[PG_CRAWL_MAX_URL];
        char resolved[PG_CRAWL_MAX_URL];
        char context[1536];
        size_t href_len, context_len;
        int priority;

        /* Find the next opening anchor tag, case-insensitively. */
        for (const char *q = p; *q; q++)
        {
            if (q[0] == '<' && tolower((unsigned char)q[1]) == 'a' &&
                (isspace((unsigned char)q[2]) || q[2] == '>'))
            {
                anchor = q;
                break;
            }
        }

        if (anchor == NULL)
            break;

        tag_end = strchr(anchor, '>');
        if (tag_end == NULL)
            break;

        for (const char *q = anchor; q < tag_end; q++)
        {
            if ((q == anchor || isspace((unsigned char)q[-1])) &&
                (size_t)(tag_end - q) >= 4 &&
                tolower((unsigned char)q[0]) == 'h' &&
                tolower((unsigned char)q[1]) == 'r' &&
                tolower((unsigned char)q[2]) == 'e' &&
                tolower((unsigned char)q[3]) == 'f')
            {
                const char *v = q + 4;
                while (v < tag_end && isspace((unsigned char)*v))
                    v++;
                if (v < tag_end && *v == '=')
                {
                    v++;
                    while (v < tag_end && isspace((unsigned char)*v))
                        v++;
                    if (v < tag_end && (*v == '"' || *v == '\''))
                    {
                        char quote = *v++;
                        href_attr = v;
                        value_end = v;
                        while (value_end < tag_end && *value_end != quote)
                            value_end++;
                        break;
                    }
                    else if (v < tag_end)
                    {
                        /* HTML also permits unquoted href values. */
                        href_attr = v;
                        value_end = v;
                        while (value_end < tag_end &&
                               !isspace((unsigned char)*value_end) &&
                               *value_end != '>')
                            value_end++;
                        break;
                    }
                }
            }
        }

        if (href_attr != NULL)
        {
            href_len = (size_t)(value_end - href_attr);
            if (href_len > 0 && href_len < sizeof(href))
            {
                memcpy(href, href_attr, href_len);
                href[href_len] = '\0';

                const char *text_end = strstr(tag_end + 1, "</a");
                context_len = text_end
                    ? (size_t)(text_end - (tag_end + 1))
                    : 0;

                if (context_len > sizeof(context) - href_len - 2)
                    context_len = sizeof(context) - href_len - 2;

                memcpy(context, href, href_len);
                context[href_len] = ' ';
                if (context_len > 0)
                    memcpy(context + href_len + 1,
                           tag_end + 1, context_len);
                context[href_len + 1 + context_len] = '\0';

                priority = classify_page(context);

                if (priority > 0 &&
                    resolve_same_origin(base_url, href,
                                        resolved, sizeof(resolved)) == 0)
                    add_candidate(items, count, resolved, priority);
            }
        }

        p = tag_end + 1;
    }
}

static void sort_candidates(PGCrawlCandidate *items, size_t count)
{
    for (size_t i = 1; i < count; i++)
    {
        PGCrawlCandidate current = items[i];
        size_t j = i;

        while (j > 0 && items[j - 1].priority > current.priority)
        {
            items[j] = items[j - 1];
            j--;
        }

        items[j] = current;
    }
}

/* Read a quoted JSON string property within a bounded object. */
static int pg_json_string(
    const char *start,
    const char *end,
    const char *key,
    char *out,
    size_t out_size)
{
    char pattern[64];
    const char *p;

    if (start == NULL || end == NULL || key == NULL ||
        out == NULL || out_size == 0)
        return 0;

    out[0] = '\0';

    if (snprintf(pattern, sizeof(pattern), "\"%s\"", key) >=
        (int)sizeof(pattern))
        return 0;

    p = start;

    while (p < end)
    {
        const char *found = strstr(p, pattern);
        const char *v;
        size_t n = 0;

        if (found == NULL || found >= end)
            return 0;

        v = found + strlen(pattern);

        while (v < end && isspace((unsigned char)*v))
            v++;

        if (v >= end || *v != ':')
        {
            p = found + strlen(pattern);
            continue;
        }

        v++;

        while (v < end && isspace((unsigned char)*v))
            v++;

        if (v >= end || *v != '"')
        {
            p = v;
            continue;
        }

        v++;

        while (v < end && *v != '"' && n + 1 < out_size)
        {
            if (*v == '\\' && v + 1 < end)
                v++;

            out[n++] = *v++;
        }

        out[n] = '\0';
        return n > 0;
    }

    return 0;
}

/* Find the closing brace while respecting quoted JSON strings. */
static const char *pg_json_object_end(const char *start)
{
    int depth = 0;
    int in_string = 0;
    int escaped = 0;

    for (const char *p = start; *p; p++)
    {
        char c = *p;

        if (in_string)
        {
            if (escaped)
                escaped = 0;
            else if (c == '\\')
                escaped = 1;
            else if (c == '"')
                in_string = 0;

            continue;
        }

        if (c == '"')
            in_string = 1;
        else if (c == '{')
            depth++;
        else if (c == '}')
        {
            depth--;

            if (depth == 0)
                return p + 1;
        }
    }

    return NULL;
}


/* Find the matching closing div, accounting for nested div elements. */
static const char *pg_find_div_end(
    const char *open,
    const char *limit)
{
    const char *tag_end;
    const char *p;
    int depth = 1;

    if (open == NULL || limit == NULL || open >= limit)
        return NULL;

    tag_end = memchr(open, '>', (size_t)(limit - open));

    if (tag_end == NULL)
        return NULL;

    p = tag_end + 1;

    while (p < limit)
    {
        const char *next_open = strstr(p, "<div");
        const char *next_close = strstr(p, "</div");

        if (next_close == NULL || next_close >= limit)
            next_close = NULL;

        if (next_open == NULL || next_open >= limit)
            next_open = NULL;

        if (next_open == NULL && next_close == NULL)
            return NULL;

        if (next_close != NULL &&
            (next_open == NULL || next_close < next_open))
        {
            depth--;

            if (depth == 0)
                return next_close;

            p = next_close + 5;
        }
        else
        {
            char after;
            const char *end;

            after = next_open[4];

            if (after != '>' && !isspace((unsigned char)after))
            {
                p = next_open + 4;
                continue;
            }

            end = memchr(
                next_open, '>', (size_t)(limit - next_open));

            if (end == NULL)
                return NULL;

            depth++;
            p = end + 1;
        }
    }

    return NULL;
}

/* Copy readable text from a bounded HTML element, without decoding entities. */
static int pg_visible_element_text(
    const char *start,
    const char *limit,
    const char *open_tag,
    const char *close_tag,
    char *out,
    size_t out_size)
{
    const char *open;
    const char *tag_end;
    const char *close;
    const char *p;
    size_t used = 0;
    int pending_space = 0;

    if (start == NULL || limit == NULL ||
        open_tag == NULL || close_tag == NULL ||
        out == NULL || out_size == 0)
        return 0;

    out[0] = '\0';

    open = strstr(start, open_tag);

    if (open == NULL || open >= limit)
        return 0;

    tag_end = memchr(open, '>', (size_t)(limit - open));

    if (tag_end == NULL)
        return 0;

    close = strstr(tag_end + 1, close_tag);

    if (close == NULL || close >= limit)
        return 0;

    p = tag_end + 1;

    while (p < close && used + 1 < out_size)
    {
        if (*p == '<')
        {
            const char *end = memchr(
                p, '>', (size_t)(close - p));

            if (end == NULL)
                break;

            pending_space = used > 0;
            p = end + 1;
            continue;
        }

        if (isspace((unsigned char)*p))
        {
            pending_space = used > 0;
            p++;
            continue;
        }

        if (pending_space && used + 1 < out_size)
            out[used++] = ' ';

        pending_space = 0;
        out[used++] = *p++;
    }

    while (used > 0 && out[used - 1] == ' ')
        used--;

    out[used] = '\0';

    return used > 0;
}

/* Extract names and roles from explicitly marked team-card containers. */
static void pg_extract_visible_team_cards(
    PGContext *ctx,
    const char *source_url)
{
    const char *html;
    const char *html_end;
    const char *p;

    if (ctx == NULL || ctx->response.body == NULL ||
        source_url == NULL)
        return;

    html = ctx->response.body;
    html_end = html + strlen(html);
    p = html;

    while (p < html_end &&
           ctx->company.people_count < PG_MAX_PEOPLE)
    {
        const char *open = strstr(p, "<div");
        const char *tag_end;
        const char *card_end;
        const char *info;
        char tag[512];
        size_t tag_length;
        char name[PG_PERSON_NAME_SIZE] = "";
        char role[PG_PERSON_ROLE_SIZE] = "";
        int duplicate = 0;

        if (open == NULL || open >= html_end)
            break;

        tag_end = memchr(open, '>', (size_t)(html_end - open));

        if (tag_end == NULL)
            break;

        tag_length = (size_t)(tag_end - open) + 1;

        if (tag_length >= sizeof(tag))
        {
            p = tag_end + 1;
            continue;
        }

        memcpy(tag, open, tag_length);
        tag[tag_length] = '\0';

        p = tag_end + 1;

if (!contains_ci(tag, "team-card"))
    continue;

card_end = pg_find_div_end(open, html_end);

if (card_end == NULL)
{

    continue;
}

        info = strstr(tag_end + 1, "team-info");

        if (info == NULL || info >= card_end)
            continue;

if (!pg_visible_element_text(
        info, card_end, "<h4", "</h4>",
        name, sizeof(name)))
{
    continue;
}

if (!pg_visible_element_text(
        info, card_end, "<span", "</span>",
        role, sizeof(role)))
{

    continue;
}

        /* Avoid obvious testimonial/customer review containers. */
        {
            const char *review = strstr(open, "review-card");

            if (review != NULL && review < card_end)
                continue;
        }

        for (int i = 0; i < ctx->company.people_count; i++)
        {
            if (strcmp(ctx->company.people[i].name, name) == 0 &&
                strcmp(ctx->company.people[i].role, role) == 0)
            {
                duplicate = 1;
                break;
            }
        }

        if (!duplicate &&
            ctx->company.people_count < PG_MAX_PEOPLE)
        {
            PGPerson *person =
                &ctx->company.people[ctx->company.people_count];

            snprintf(person->name, sizeof(person->name), "%s", name);
            snprintf(person->role, sizeof(person->role), "%s", role);
            person->email[0] = '\0';

            snprintf(person->source_url, sizeof(person->source_url),
                     "%s", source_url);

            snprintf(person->evidence, sizeof(person->evidence),
                     "Visible team card: name=%s; role=%s",
                     name, role);

            ctx->company.people_count++;
        }

        p = card_end + 5;
    }
}

/* Check only the opening script tag for its JSON-LD type. */
static int pg_is_json_ld_script(
    const char *open,
    const char *close)
{
    const char *tag_end;
    size_t tag_length;
    char tag[512];

    if (open == NULL || close == NULL || close <= open)
        return 0;

    tag_end = memchr(open, '>', (size_t)(close - open));

    if (tag_end == NULL)
        return 0;

    tag_length = (size_t)(tag_end - open) + 1;

    if (tag_length >= sizeof(tag))
        return 0;

    memcpy(tag, open, tag_length);
    tag[tag_length] = '\0';

    return contains_ci(tag, "application/ld+json");
}


/* Add a person only when a name and explicit role are available. */
static void pg_extract_people(PGContext *ctx, const char *source_url)
{
    const char *html;
    const char *script;
    const char *html_end;

    if (ctx == NULL || ctx->response.body == NULL ||
        source_url == NULL)
        return;

    html = ctx->response.body;
    html_end = html + strlen(html);
    script = html;

    while (script < html_end)
    {
        const char *open = NULL;
        const char *close;
        const char *p;

        for (p = script; p + 7 <= html_end; p++)
        {
            if (p[0] == '<' &&
                tolower((unsigned char)p[1]) == 's' &&
                tolower((unsigned char)p[2]) == 'c' &&
                tolower((unsigned char)p[3]) == 'r' &&
                tolower((unsigned char)p[4]) == 'i' &&
                tolower((unsigned char)p[5]) == 'p' &&
                tolower((unsigned char)p[6]) == 't')
            {
                open = p;
                break;
            }
        }

        if (open == NULL)
            break;

        close = open + 7;

        while (close + 8 <= html_end &&
               !(*close == '<' && close[1] == '/' &&
                 tolower((unsigned char)close[2]) == 's' &&
                 tolower((unsigned char)close[3]) == 'c' &&
                 tolower((unsigned char)close[4]) == 'r' &&
                 tolower((unsigned char)close[5]) == 'i' &&
                 tolower((unsigned char)close[6]) == 'p' &&
                 tolower((unsigned char)close[7]) == 't'))
            close++;

        if (close + 8 > html_end)
            break;

        /* Only inspect JSON-LD script blocks. */
        if (pg_is_json_ld_script(open, close))
        {
            const char *type = open;

            while ((type = strstr(type, "\"@type\"")) != NULL &&
                   type < close)
            {
                const char *value = strchr(type, ':');
                const char *object_start;
                const char *object_end;
                char person_type[64] = "";
                char name[PG_PERSON_NAME_SIZE] = "";
                char role[PG_PERSON_ROLE_SIZE] = "";
                char email[256] = "";
                char evidence[PG_PERSON_EVIDENCE_SIZE] = "";
                int duplicate = 0;

                if (value == NULL || value >= close)
                    break;

                value++;

                while (value < close &&
                       isspace((unsigned char)*value))
                    value++;

                if (value >= close || *value != '"')
                {
                    type += 7;
                    continue;
                }

                value++;

                {
                    size_t n = 0;

                    while (value < close && *value != '"' &&
                           n + 1 < sizeof(person_type))
                        person_type[n++] = *value++;

                    person_type[n] = '\0';
                }

                if (strcmp(person_type, "Person") != 0)
                {
                    type += 7;
                    continue;
                }

                object_start = type;

                while (object_start > open && *object_start != '{')
                    object_start--;

                if (*object_start != '{')
                {
                    type += 7;
                    continue;
                }

                object_end = pg_json_object_end(object_start);

                if (object_end == NULL || object_end > close)
                {
                    type += 7;
                    continue;
                }

                if (!pg_json_string(object_start, object_end,
                                    "name", name, sizeof(name)) ||
                    !pg_json_string(object_start, object_end,
                                    "jobTitle", role, sizeof(role)))
                {
                    type += 7;
                    continue;
                }

                (void)pg_json_string(
                    object_start, object_end,
                    "email", email, sizeof(email));

                for (int i = 0; i < ctx->company.people_count; i++)
                {
                    if (strcmp(ctx->company.people[i].name, name) == 0 &&
                        strcmp(ctx->company.people[i].role, role) == 0)
                    {
                        duplicate = 1;
                        break;
                    }
                }

                if (!duplicate &&
                    ctx->company.people_count < PG_MAX_PEOPLE)
                {
                    PGPerson *person =
                        &ctx->company.people[ctx->company.people_count];

                    snprintf(person->name, sizeof(person->name), "%s", name);
                    snprintf(person->role, sizeof(person->role), "%s", role);
                    snprintf(person->email, sizeof(person->email), "%s", email);
                    snprintf(person->source_url, sizeof(person->source_url),
                             "%s", source_url);
                    snprintf(evidence, sizeof(evidence),
                             "Schema.org Person: name=%s; jobTitle=%s",
                             name, role);
                    snprintf(person->evidence, sizeof(person->evidence),
                             "%s", evidence);

                    ctx->company.people_count++;
                }

                type = object_end;
            }
        }

        script = close + 8;
    }

    /* Also extract people from visible team cards on this page. */
    pg_extract_visible_team_cards(ctx, source_url);
}

PGStatus pg_crawl_contact_pages(PGContext *ctx)
{
    PGCrawlCandidate candidates[PG_CRAWL_MAX_CANDIDATES];
    size_t candidate_count = 0;
    size_t attempted = 0;
    PGHttpResponse homepage_response;
    const char *base_url;

    if (ctx == NULL || ctx->response.body == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    memset(candidates, 0, sizeof(candidates));

    /* Prefer the final URL after any homepage redirects. */
    base_url = ctx->response.final_url[0] != '\0'
        ? ctx->response.final_url
        : ctx->url;

    /* The homepage is already downloaded; extract its contacts first. */
    (void)pg_extract_contact(ctx);

    pg_extract_people(ctx, base_url);

    collect_link_candidates(
        ctx->response.body, base_url, candidates, &candidate_count);

    if (candidate_count == 0)
    {
        pg_context_emit_event(ctx, "contact_discovery_empty",
                              "No prioritized same-site contact links found.", base_url);
        return PG_STATUS_OK;
    }

    {
        char message[160];
        snprintf(message, sizeof(message),
                 "Discovered %zu unique same-site page candidates.", candidate_count);
        pg_context_emit_event(ctx, "contact_candidates_discovered", message, base_url);
    }

    sort_candidates(candidates, candidate_count);
    homepage_response = ctx->response;

    for (size_t i = 0;
         i < candidate_count && attempted < PG_CRAWL_MAX_CONTACT_PAGES;
         i++)
    {
        PGHttpResponse page_response;
        PGStatus extract_status;

        if (strcmp(candidates[i].url, base_url) == 0)
            continue;

        memset(&page_response, 0, sizeof(page_response));

        attempted++;
        pg_context_emit_event(ctx, "contact_page_fetch_started",
                              "Fetching a prioritized same-site page.",
                              candidates[i].url);

        if (pg_http_get(candidates[i].url, &page_response) != 0)
        {
            pg_context_emit_event(ctx, "contact_page_fetch_failed",
                                  "Could not download the candidate page.",
                                  candidates[i].url);
            pg_http_free(&page_response);
            continue;
        }

        if (page_response.status < 200 || page_response.status >= 400 ||
            page_response.body == NULL)
        {
            pg_context_emit_event(ctx, "contact_page_fetch_failed",
                                  "Candidate page returned an unsuccessful HTTP status.",
                                  page_response.final_url[0] ? page_response.final_url : candidates[i].url);
            pg_http_free(&page_response);
            continue;
        }

        ctx->response = page_response;

        pg_extract_people(
            ctx,
            page_response.final_url[0] != '\0'
                ? page_response.final_url
                : candidates[i].url);

        extract_status = pg_extract_contact(ctx);
        (void)pg_extract_social(ctx);

        pg_http_free(&ctx->response);
        ctx->response = homepage_response;

        if (extract_status == PG_STATUS_OK)
        {
            pg_context_emit_event(ctx, "contact_page_scanned",
                                  "Scanned a prioritized same-site contact page.",
                                  page_response.final_url[0] ? page_response.final_url : candidates[i].url);
        }
    }

    /* The homepage response remains owned by the caller. */
    ctx->response = homepage_response;

    return PG_STATUS_OK;
}
