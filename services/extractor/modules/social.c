#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "pagegoat/services/extractor/modules/social.h"

#define PG_SOCIAL_URL_CAPACITY 1024

typedef struct
{
    const char *domain;
    char *destination;
    size_t destination_size;
    const char *platform;
} PGSocialTarget;

static int contains_ci(const char *text, const char *needle)
{
    size_t n;

    if (text == NULL || needle == NULL)
        return 0;

    n = strlen(needle);
    if (n == 0)
        return 1;

    for (; *text != '\0'; text++)
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

static int host_matches(const char *host, const char *domain)
{
    if (host == NULL || domain == NULL)
        return 0;

    /* Accept the exact domain or its www subdomain only. */
    if (strcmp(host, domain) == 0)
        return 1;

    return strncmp(host, "www.", 4) == 0 &&
           strcmp(host + 4, domain) == 0;
}

/* Extract the hostname from an absolute or protocol-relative URL. */
static int get_host_and_path(
    const char *url,
    char *host,
    size_t host_size,
    const char **path)
{
    const char *authority;
    const char *authority_end;
    const char *host_start;
    const char *host_end;
    size_t host_length;

    if (url == NULL || host == NULL || host_size == 0 || path == NULL)
        return 0;

    if (strncmp(url, "https://", 8) == 0)
        authority = url + 8;
    else if (strncmp(url, "http://", 7) == 0)
        authority = url + 7;
    else if (strncmp(url, "//", 2) == 0)
        authority = url + 2;
    else
        return 0;

    authority_end = authority + strcspn(authority, "/?#");
    host_start = authority;

    /* Discard optional user information, if present. */
    {
        const char *at = memchr(authority, '@',
                                (size_t)(authority_end - authority));
        if (at != NULL)
            host_start = at + 1;
    }

    host_end = host_start;
    while (host_end < authority_end && *host_end != ':')
        host_end++;

    host_length = (size_t)(host_end - host_start);
    if (host_length == 0 || host_length >= host_size)
        return 0;

    for (size_t i = 0; i < host_length; i++)
        host[i] = (char)tolower((unsigned char)host_start[i]);
    host[host_length] = '\0';

    *path = authority_end;
    return 1;
}

static size_t clean_social_path(
    const char *path,
    char *out,
    size_t out_size)
{
    size_t length;

    if (path == NULL || out == NULL || out_size == 0)
        return 0;

    length = strcspn(path, "?#");

    if (length >= out_size)
        return 0;

    memcpy(out, path, length);
    out[length] = '\0';

    /* Ignore trailing slashes when validating a profile path. */
    while (length > 1 && out[length - 1] == '/')
        out[--length] = '\0';

    return length;
}

static size_t path_segment_count(const char *path)
{
    size_t count = 0;
    const char *p;

    if (path == NULL || path[0] != '/' || path[1] == '\0')
        return 0;

    p = path + 1;

    while (*p != '\0')
    {
        count++;

        p = strchr(p, '/');
        if (p == NULL)
            break;

        p++;
    }

    return count;
}

static int first_segment_is(
    const char *path,
    const char *expected)
{
    const char *start;
    size_t length;
    size_t expected_length;

    if (path == NULL || expected == NULL || path[0] != '/')
        return 0;

    start = path + 1;
    length = strcspn(start, "/");
    expected_length = strlen(expected);

    if (length != expected_length)
        return 0;

    for (size_t i = 0; i < length; i++)
    {
        if (tolower((unsigned char)start[i]) !=
            tolower((unsigned char)expected[i]))
            return 0;
    }

    return 1;
}

static int first_segment_is_reserved(
    const char *path,
    const char *const reserved[],
    size_t reserved_count)
{
    for (size_t i = 0; i < reserved_count; i++)
    {
        if (first_segment_is(path, reserved[i]))
            return 1;
    }

    return 0;
}

static int is_valid_social_profile(
    const char *platform,
    const char *host,
    const char *path,
    const char *url)
{
    char clean_path[PG_SOCIAL_URL_CAPACITY];
    size_t segments;
    size_t length;

    static const char *const instagram_reserved[] = {
        "accounts", "direct", "explore", "p", "reel",
        "reels", "stories", "share", "tv"
    };

    static const char *const twitter_reserved[] = {
        "about", "account", "compose", "explore", "hashtag",
        "home", "i", "intent", "jobs", "login", "logout",
        "messages", "notifications", "search", "settings",
        "share", "tos", "privacy"
    };

    static const char *const facebook_reserved[] = {
        "business", "events", "gaming", "groups", "help",
        "login", "marketplace", "pages", "plugins", "policies",
        "privacy", "reel", "share", "sharer", "watch"
    };

    static const char *const github_reserved[] = {
        "about", "apps", "collections", "enterprise", "features",
        "issues", "login", "marketplace", "new", "notifications",
        "orgs", "pricing", "search", "security", "sessions",
        "settings", "signup", "sponsors", "topics"
    };

    static const char *const pinterest_reserved[] = {
        "business", "categories", "explore", "ideas", "login",
        "pin", "search", "settings", "shopping", "signup", "today"
    };

    if (platform == NULL || host == NULL || path == NULL)
        return 0;

    length = clean_social_path(
        path, clean_path, sizeof(clean_path));

    if (length == 0)
        return 0;

    segments = path_segment_count(clean_path);

    if (strcmp(platform, "LinkedIn") == 0)
    {
        return segments == 2 &&
               strncmp(clean_path, "/company/", 9) == 0;
    }

    if (strcmp(platform, "Facebook") == 0)
    {
        if (first_segment_is(clean_path, "profile.php"))
            return segments == 1 &&
                   url != NULL && contains_ci(url, "id=");

        return segments == 1 &&
               !first_segment_is_reserved(
                   clean_path,
                   facebook_reserved,
                   sizeof(facebook_reserved) /
                       sizeof(facebook_reserved[0]));
    }

    if (strcmp(platform, "Instagram") == 0)
    {
        return segments == 1 &&
               !first_segment_is_reserved(
                   clean_path,
                   instagram_reserved,
                   sizeof(instagram_reserved) /
                       sizeof(instagram_reserved[0]));
    }

    if (strcmp(platform, "Twitter/X") == 0)
    {
        return segments == 1 &&
               !first_segment_is_reserved(
                   clean_path,
                   twitter_reserved,
                   sizeof(twitter_reserved) /
                       sizeof(twitter_reserved[0]));
    }

    if (strcmp(platform, "YouTube") == 0)
    {
        /* youtu.be links are videos, not channel profiles. */
        if (!host_matches(host, "youtube.com"))
            return 0;

        if (clean_path[1] == '@')
            return segments == 1;

        return segments == 2 &&
               (first_segment_is(clean_path, "channel") ||
                first_segment_is(clean_path, "c") ||
                first_segment_is(clean_path, "user"));
    }

    if (strcmp(platform, "GitHub") == 0)
    {
        return segments == 1 &&
               !first_segment_is_reserved(
                   clean_path,
                   github_reserved,
                   sizeof(github_reserved) /
                       sizeof(github_reserved[0]));
    }

    if (strcmp(platform, "TikTok") == 0)
    {
        return segments == 1 && clean_path[1] == '@';
    }

    if (strcmp(platform, "Pinterest") == 0)
    {
        return segments == 1 &&
               !first_segment_is_reserved(
                   clean_path,
                   pinterest_reserved,
                   sizeof(pinterest_reserved) /
                       sizeof(pinterest_reserved[0]));
    }

    return 0;
}


static void store_first_url(
    char *destination,
    size_t destination_size,
    const char *url,
    size_t url_length)
{
    if (destination == NULL || destination_size == 0 || url == NULL ||
        destination[0] != '\0' || url_length == 0 ||
        url_length >= destination_size)
        return;

    memcpy(destination, url, url_length);
    destination[url_length] = '\0';
}


static void inspect_social_url(
    const char *url,
    size_t url_length,
    PGSocialTarget *targets,
    size_t target_count)
{
    char candidate[PG_SOCIAL_URL_CAPACITY];
    char host[256];
    const char *path;

    if (url == NULL || url_length == 0 ||
        url_length >= sizeof(candidate))
        return;

    memcpy(candidate, url, url_length);
    candidate[url_length] = '\0';

    if (!get_host_and_path(
            candidate, host, sizeof(host), &path))
        return;

    for (size_t i = 0; i < target_count; i++)
    {
        if (targets[i].destination == NULL ||
            targets[i].destination_size == 0 ||
            targets[i].destination[0] != '\0')
            continue;

        if (!host_matches(host, targets[i].domain))
            continue;

        if (!is_valid_social_profile(
                targets[i].platform, host, path, candidate))
            continue;

        store_first_url(
            targets[i].destination,
            targets[i].destination_size,
            candidate,
            url_length);

        return;
    }
}


PGStatus pg_extract_social(PGContext *ctx)
{
    PGSocialTarget targets[] = {
        {"linkedin.com", ctx != NULL ? ctx->company.linkedin : NULL,
         ctx != NULL ? sizeof(ctx->company.linkedin) : 0, "LinkedIn"},
        {"facebook.com", ctx != NULL ? ctx->company.facebook : NULL,
         ctx != NULL ? sizeof(ctx->company.facebook) : 0, "Facebook"},
        {"instagram.com", ctx != NULL ? ctx->company.instagram : NULL,
         ctx != NULL ? sizeof(ctx->company.instagram) : 0, "Instagram"},
        {"twitter.com", ctx != NULL ? ctx->company.twitter : NULL,
         ctx != NULL ? sizeof(ctx->company.twitter) : 0, "Twitter/X"},
        {"x.com", ctx != NULL ? ctx->company.twitter : NULL,
         ctx != NULL ? sizeof(ctx->company.twitter) : 0, "Twitter/X"},
        {"youtube.com", ctx != NULL ? ctx->company.youtube : NULL,
         ctx != NULL ? sizeof(ctx->company.youtube) : 0, "YouTube"},
        {"youtu.be", ctx != NULL ? ctx->company.youtube : NULL,
         ctx != NULL ? sizeof(ctx->company.youtube) : 0, "YouTube"},
        {"github.com", ctx != NULL ? ctx->company.github : NULL,
         ctx != NULL ? sizeof(ctx->company.github) : 0, "GitHub"},
        {"tiktok.com", ctx != NULL ? ctx->company.tiktok : NULL,
         ctx != NULL ? sizeof(ctx->company.tiktok) : 0, "TikTok"},
        {"pinterest.com", ctx != NULL ? ctx->company.pinterest : NULL,
         ctx != NULL ? sizeof(ctx->company.pinterest) : 0, "Pinterest"}
    };
    const char *html;
    size_t target_count = sizeof(targets) / sizeof(targets[0]);
    size_t html_length;

    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    html = ctx->response.body;
    if (html == NULL)
        return PG_STATUS_PARSE_ERROR;

    html_length = strlen(html);

    /* Read href attributes without assuming a particular attribute order. */
    for (size_t offset = 0; offset + 5 < html_length; offset++)
    {
        const char *p = html + offset;
        const char *value;
        const char *end;
        char quote;

        if (tolower((unsigned char)p[0]) != 'h' ||
            tolower((unsigned char)p[1]) != 'r' ||
            tolower((unsigned char)p[2]) != 'e' ||
            tolower((unsigned char)p[3]) != 'f' || p[4] != '=')
            continue;

        value = p + 5;
        quote = *value;
        if (quote != '\'' && quote != '"')
            continue;

        value++;
        end = strchr(value, quote);
        if (end == NULL)
            break;

        inspect_social_url(value, (size_t)(end - value), targets, target_count);
        offset = (size_t)(end - html);
    }

    return PG_STATUS_OK;
}
