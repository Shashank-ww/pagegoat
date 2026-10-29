
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "pagegoat/services/extractor/modules/technology.h"
#include "pagegoat/platform/http.h"

#define PG_MAX_SCRIPT_URLS 32
#define PG_MAX_SCRIPT_URL_SIZE 2048
#define PG_MAX_SCRIPTS_TO_FETCH 12

static const char *find_ignore_case(
    const char *text,
    const char *needle)
{
    if (text == NULL || needle == NULL || needle[0] == '\0')
        return NULL;

    for (const char *p = text; *p != '\0'; p++)
    {
        const char *a = p;
        const char *b = needle;

        while (*a != '\0' && *b != '\0' &&
               tolower((unsigned char)*a) ==
               tolower((unsigned char)*b))
        {
            a++;
            b++;
        }

        if (*b == '\0')
            return p;
    }

    return NULL;
}

static int contains_ignore_case(
    const char *text,
    const char *needle)
{
    return find_ignore_case(text, needle) != NULL;
}

static void add_technology(
    PGCompany *company,
    const char *name)
{
    for (int i = 0; i < company->technology_count; i++)
    {
        if (strcmp(company->technologies[i], name) == 0)
            return;
    }

    if (company->technology_count >= PG_MAX_TECHNOLOGIES)
        return;

    size_t len = strlen(name);

    if (len >= PG_TECHNOLOGY_SIZE)
        len = PG_TECHNOLOGY_SIZE - 1;

    memcpy(
        company->technologies[company->technology_count],
        name,
        len);

    company->technologies[company->technology_count][len] = '\0';
    company->technology_count++;
}

/*
 * Detect technologies in any HTML or JavaScript source.
 * Existing signatures are retained; new signatures can be
 * added here without changing the script-fetching logic.
 */
static void detect_technologies(
    PGCompany *company,
    const char *source)
{
    if (company == NULL || source == NULL)
        return;

    /* Advertising platforms and tag managers */

    if (contains_ignore_case(source, "googletagmanager.com/gtm.js") ||
        contains_ignore_case(source, "googletagmanager.com/gtm/") ||
        contains_ignore_case(source, "GTM-"))
        add_technology(company, "Google Tag Manager");

    if (contains_ignore_case(source, "google-analytics.com/analytics.js") ||
        contains_ignore_case(source, "googletagmanager.com/gtag/js") ||
        contains_ignore_case(source, "google-analytics.com/g/collect") ||
        contains_ignore_case(source, "gtag('config', 'G-") ||
        contains_ignore_case(source, "gtag(\"config\", \"G-") ||
        contains_ignore_case(source, "UA-"))
        add_technology(company, "Google Analytics");

    if (contains_ignore_case(source, "googleadservices.com/pagead/") ||
        contains_ignore_case(source, "google.com/pagead/1p-conversion") ||
        contains_ignore_case(source, "google_conversion_id") ||
        contains_ignore_case(source, "gtag('config', 'AW-") ||
        contains_ignore_case(source, "gtag(\"config\", \"AW-"))
        add_technology(company, "Google Ads");

    if (contains_ignore_case(source, "doubleclick.net/activity") ||
        contains_ignore_case(source, "fls.doubleclick.net"))
        add_technology(company, "Google Floodlight");

    if ((contains_ignore_case(source, "connect.facebook.net") &&
         contains_ignore_case(source, "fbq(")) ||
        contains_ignore_case(source, "facebook.com/tr"))
        add_technology(company, "Meta Pixel");

    if (contains_ignore_case(source, "snap.licdn.com/li.lms-analytics") ||
        contains_ignore_case(source, "_linkedin_partner_id") ||
        contains_ignore_case(source, "linkedin.com/px"))
        add_technology(company, "LinkedIn Insight Tag");

    if (contains_ignore_case(source, "analytics.tiktok.com") ||
        contains_ignore_case(source, "ttq.load") ||
        contains_ignore_case(source, "ttq.track"))
        add_technology(company, "TikTok Pixel");

    if (contains_ignore_case(source, "bat.bing.com/bat.js") ||
        contains_ignore_case(source, "bat.bing.com/action") ||
        contains_ignore_case(source, "uetq"))
        add_technology(company, "Microsoft Advertising UET");

    if (contains_ignore_case(source, "static.ads-twitter.com/uwt.js") ||
        contains_ignore_case(source, "analytics.twitter.com") ||
        contains_ignore_case(source, "twq('init'") ||
        contains_ignore_case(source, "twq(\"init\""))
        add_technology(company, "X Ads Pixel");

    if (contains_ignore_case(source, "s.pinimg.com/ct/core.js") ||
        contains_ignore_case(source, "pintrk('load'") ||
        contains_ignore_case(source, "pintrk(\"load\""))
        add_technology(company, "Pinterest Tag");

    if (contains_ignore_case(source, "sc-static.net/scevent.min.js") ||
        contains_ignore_case(source, "snaptr('init'") ||
        contains_ignore_case(source, "snaptr(\"init\""))
        add_technology(company, "Snapchat Pixel");

    if (contains_ignore_case(source, "alb.reddit.com/snoo") ||
        contains_ignore_case(source, "reddit.com/pixel") ||
        contains_ignore_case(source, "rdt('init'"))
        add_technology(company, "Reddit Pixel");

    if (contains_ignore_case(source, "aax.amazon-adsystem.com") ||
        contains_ignore_case(source, "amazon-adsystem.com"))
        add_technology(company, "Amazon Ads");

    if (contains_ignore_case(source, "connect.getadroll.com") ||
        contains_ignore_case(source, "s.adroll.com"))
        add_technology(company, "AdRoll");

    if (contains_ignore_case(source, "static.criteo.net") ||
        contains_ignore_case(source, "criteo.com"))
        add_technology(company, "Criteo");

    if (contains_ignore_case(source, "trc.taboola.com") ||
        contains_ignore_case(source, "cdn.taboola.com"))
        add_technology(company, "Taboola");

    if (contains_ignore_case(source, "outbrain.com") ||
        contains_ignore_case(source, "widgets.outbrain.com"))
        add_technology(company, "Outbrain");

    if (contains_ignore_case(source, "pixel.quantserve.com") ||
        contains_ignore_case(source, "quantserve.com/quant.js"))
        add_technology(company, "Quantcast");

    if (contains_ignore_case(source, "demdex.net") ||
        contains_ignore_case(source, "omtrdc.net") ||
        contains_ignore_case(source, "adobedtm.com"))
        add_technology(company, "Adobe Experience Cloud");

    /* Marketing automation and CRM */

    if (contains_ignore_case(source, "js.hs-scripts.com") ||
        contains_ignore_case(source, "js.hs-analytics.net") ||
        contains_ignore_case(source, "hubspotutk"))
        add_technology(company, "HubSpot");

    if (contains_ignore_case(source, "munchkin.marketo.net") ||
        contains_ignore_case(source, "marketo.com"))
        add_technology(company, "Adobe Marketo");

    if (contains_ignore_case(source, "pardot.com") ||
        contains_ignore_case(source, "pi.pardot.com"))
        add_technology(company, "Salesforce Account Engagement");

    if (contains_ignore_case(source, "salesforceliveagent.com") ||
        contains_ignore_case(source, "force.com"))
        add_technology(company, "Salesforce");

    /* Analytics and attribution */

    if (contains_ignore_case(source, "clarity.ms/tag/"))
        add_technology(company, "Microsoft Clarity");

    if (contains_ignore_case(source, "static.hotjar.com") ||
        contains_ignore_case(source, "script.hotjar.com") ||
        contains_ignore_case(source, "hotjar.com"))
        add_technology(company, "Hotjar");

    if (contains_ignore_case(source, "cdn.segment.com/analytics.js") ||
        contains_ignore_case(source, "cdn.segment.io"))
        add_technology(company, "Segment");

    if (contains_ignore_case(source, "cdn.mxpnl.com") ||
        contains_ignore_case(source, "mixpanel.init"))
        add_technology(company, "Mixpanel");

    if (contains_ignore_case(source, "cdn.amplitude.com") ||
        contains_ignore_case(source, "amplitude.getinstance"))
        add_technology(company, "Amplitude");

    if (contains_ignore_case(source, "static.heap.io") ||
        contains_ignore_case(source, "heap.load"))
        add_technology(company, "Heap");

    if (contains_ignore_case(source, "matomo.js") ||
        contains_ignore_case(source, "piwik.js"))
        add_technology(company, "Matomo");

    if (contains_ignore_case(source, "plausible.io/js/"))
        add_technology(company, "Plausible Analytics");

    if (contains_ignore_case(source, "us.posthog.com") ||
        contains_ignore_case(source, "app.posthog.com") ||
        contains_ignore_case(source, "posthog.init"))
        add_technology(company, "PostHog");

    /* Web frameworks */

    if (contains_ignore_case(source, "/_next/") ||
        contains_ignore_case(source, "__NEXT_DATA__") ||
        contains_ignore_case(source, "self.__next_f.push"))
        add_technology(company, "Next.js");

    if (contains_ignore_case(source, "react-dom") ||
        contains_ignore_case(source, "react.production.min.js") ||
        contains_ignore_case(source, "__REACT"))
        add_technology(company, "React");

    if (contains_ignore_case(source, "vue.global.js") ||
        contains_ignore_case(source, "vue.runtime") ||
        contains_ignore_case(source, "vue.min.js"))
        add_technology(company, "Vue.js");

    if (contains_ignore_case(source, "ng-version=") ||
        contains_ignore_case(source, "angular.min.js"))
        add_technology(company, "Angular");

    if (contains_ignore_case(source, "svelte/internal") ||
        contains_ignore_case(source, "svelte-h"))
        add_technology(company, "Svelte");

    if (contains_ignore_case(source, "__NUXT__") ||
        contains_ignore_case(source, "/_nuxt/"))
        add_technology(company, "Nuxt");

    if (contains_ignore_case(source, "gatsby-chunk-mapping") ||
        contains_ignore_case(source, "___gatsby"))
        add_technology(company, "Gatsby");

    if (contains_ignore_case(source, "astro-island") ||
        contains_ignore_case(source, "/_astro/"))
        add_technology(company, "Astro");

    if (contains_ignore_case(source, "__remixContext"))
        add_technology(company, "Remix");

    /* Existing frontend libraries */

    if (contains_ignore_case(source, "jquery.min.js") ||
        contains_ignore_case(source, "jquery.js") ||
        contains_ignore_case(source, "code.jquery.com/jquery"))
        add_technology(company, "jQuery");

    if (contains_ignore_case(source, "bootstrap.min.css") ||
        contains_ignore_case(source, "bootstrap.min.js"))
        add_technology(company, "Bootstrap");

    /* CMS platforms */

    if (contains_ignore_case(source, "/wp-content/") ||
        contains_ignore_case(source, "/wp-includes/") ||
        contains_ignore_case(source, "wp-json"))
        add_technology(company, "WordPress");

    if (contains_ignore_case(source, "cdn.shopify.com") ||
        contains_ignore_case(source, "shopify.theme") ||
        contains_ignore_case(source, "myshopify.com"))
        add_technology(company, "Shopify");

    if (contains_ignore_case(source, "drupal.js") ||
        contains_ignore_case(source, "/sites/default/files/"))
        add_technology(company, "Drupal");

    if (contains_ignore_case(source, "joomla!") ||
        contains_ignore_case(source, "/media/system/js/"))
        add_technology(company, "Joomla");

    if (contains_ignore_case(source, "/typo3conf/"))
        add_technology(company, "TYPO3");

    if (contains_ignore_case(source, "ghost-url"))
        add_technology(company, "Ghost");

    if (contains_ignore_case(source, "cdn.sanity.io") ||
        contains_ignore_case(source, "sanity-client"))
        add_technology(company, "Sanity");

    if (contains_ignore_case(source, "images.ctfassets.net"))
        add_technology(company, "Contentful");

    /* Website builders */

    if (contains_ignore_case(source, "wixstatic.com") ||
        contains_ignore_case(source, "wix.com"))
        add_technology(company, "Wix");

    if (contains_ignore_case(source, "static1.squarespace.com") ||
        contains_ignore_case(source, "squarespace.com"))
        add_technology(company, "Squarespace");

    if (contains_ignore_case(source, "webflow.js") ||
        contains_ignore_case(source, "data-wf-page"))
        add_technology(company, "Webflow");

    if (contains_ignore_case(source, "framerusercontent.com") ||
        contains_ignore_case(source, "framer.website"))
        add_technology(company, "Framer");

    if (contains_ignore_case(source, "elementor-frontend") ||
        contains_ignore_case(source, "elementor/assets/"))
        add_technology(company, "Elementor");

    if (contains_ignore_case(source, "et-builder") ||
        contains_ignore_case(source, "et_pb_"))
        add_technology(company, "Divi");

    if (contains_ignore_case(source, "editmysite.com"))
        add_technology(company, "Weebly");

    if (contains_ignore_case(source, "duda.co") ||
        contains_ignore_case(source, "dmncdn.com"))
        add_technology(company, "Duda");

    /* Hosting and content delivery */

    if (contains_ignore_case(source, "vercel.app") ||
        contains_ignore_case(source, "vercel.live") ||
        contains_ignore_case(source, "_vercel"))
        add_technology(company, "Vercel");

    if (contains_ignore_case(source, "netlify.app") ||
        contains_ignore_case(source, "netlify.com"))
        add_technology(company, "Netlify");

    if (contains_ignore_case(source, "cloudflareinsights.com") ||
        contains_ignore_case(source, "cdn-cgi/"))
        add_technology(company, "Cloudflare");

    if (contains_ignore_case(source, "cloudfront.net"))
        add_technology(company, "Amazon CloudFront");

    if (contains_ignore_case(source, "amazonaws.com"))
        add_technology(company, "Amazon Web Services");

    if (contains_ignore_case(source, "azureedge.net") ||
        contains_ignore_case(source, "azurewebsites.net"))
        add_technology(company, "Microsoft Azure");

    if (contains_ignore_case(source, "fastly.net"))
        add_technology(company, "Fastly");

    if (contains_ignore_case(source, "akamaihd.net") ||
        contains_ignore_case(source, "akamaized.net"))
        add_technology(company, "Akamai");

    if (contains_ignore_case(source, "firebaseapp.com") ||
        contains_ignore_case(source, "firebaseio.com"))
        add_technology(company, "Firebase");

    /* Payments */

    if (contains_ignore_case(source, "js.stripe.com") ||
        contains_ignore_case(source, "checkout.stripe.com"))
        add_technology(company, "Stripe");

    if (contains_ignore_case(source, "paypal.com/sdk/js") ||
        contains_ignore_case(source, "paypalobjects.com"))
        add_technology(company, "PayPal");

    if (contains_ignore_case(source, "checkout.razorpay.com") ||
        contains_ignore_case(source, "razorpay.com/v1/"))
        add_technology(company, "Razorpay");

    if (contains_ignore_case(source, "sdk.cashfree.com"))
        add_technology(company, "Cashfree");

    if (contains_ignore_case(source, "secure.payu") ||
        contains_ignore_case(source, "payu.in"))
        add_technology(company, "PayU");

    if (contains_ignore_case(source, "checkoutshopper-live.adyen.com"))
        add_technology(company, "Adyen");

    if (contains_ignore_case(source, "js.braintreegateway.com"))
        add_technology(company, "Braintree");

    if (contains_ignore_case(source, "web.squarecdn.com") ||
        contains_ignore_case(source, "squareup.com/checkout"))
        add_technology(company, "Square");
}

/* Return true for absolute HTTP(S) URLs. */
static int is_absolute_http_url(const char *url)
{
    return url != NULL &&
           (strncmp(url, "https://", 8) == 0 ||
            strncmp(url, "http://", 7) == 0);
}

/*
 * Resolve absolute, protocol-relative, root-relative and ordinary
 * relative script URLs. The input page URL should be the final URL
 * returned by the HTTP client where available.
 */
static int resolve_script_url(
    const char *base_url,
    const char *src,
    char *out,
    size_t out_size)
{
    if (base_url == NULL || src == NULL ||
        out == NULL || out_size == 0)
        return 0;

    while (isspace((unsigned char)*src))
        src++;

    size_t src_len = strlen(src);

    while (src_len > 0 &&
           isspace((unsigned char)src[src_len - 1]))
        src_len--;

    if (src_len == 0 || src_len >= PG_MAX_SCRIPT_URL_SIZE)
        return 0;

    char clean_src[PG_MAX_SCRIPT_URL_SIZE];
    memcpy(clean_src, src, src_len);
    clean_src[src_len] = '\0';

    if (clean_src[0] == '#' ||
        clean_src[0] == '\0' ||
        strncmp(clean_src, "data:", 5) == 0 ||
        strncmp(clean_src, "javascript:", 11) == 0)
        return 0;

    /* Absolute HTTP(S) URL. */
    if (is_absolute_http_url(clean_src))
    {
        if (strlen(clean_src) >= out_size)
            return 0;

        strcpy(out, clean_src);
        return 1;
    }

    /* Extract the scheme and authority from the base URL. */
    const char *scheme_end = strstr(base_url, "://");
    if (scheme_end == NULL)
        return 0;

    size_t origin_len = (size_t)(scheme_end - base_url) + 3;
    const char *authority = scheme_end + 3;
    const char *path = strpbrk(authority, "/?#");

    size_t host_len = path == NULL
        ? strlen(base_url)
        : (size_t)(path - base_url);

    if (host_len >= out_size || origin_len >= out_size)
        return 0;

    char origin[PG_MAX_SCRIPT_URL_SIZE];
    memcpy(origin, base_url, host_len);
    origin[host_len] = '\0';

    /* Protocol-relative URL. */
    if (clean_src[0] == '/' && clean_src[1] == '/')
    {
        int n = snprintf(
            out, out_size, "%.*s:%s",
            (int)(scheme_end - base_url),
            base_url,
            clean_src);

        return n > 0 && (size_t)n < out_size;
    }

    /* Root-relative URL. */
    if (clean_src[0] == '/')
    {
        int n = snprintf(out, out_size, "%s%s", origin, clean_src);
        return n > 0 && (size_t)n < out_size;
    }

    /* Ignore unsupported schemes rather than treating them as paths. */
    if (strchr(clean_src, ':') != NULL &&
        (strchr(clean_src, '/') == NULL ||
         strchr(clean_src, ':') < strchr(clean_src, '/')))
        return 0;

    /* Relative URL: append to the base page's directory. */
    const char *query = strpbrk(authority, "?#");
    const char *base_path_end = query != NULL
        ? query
        : base_url + strlen(base_url);

    const char *last_slash = NULL;
    for (const char *p = authority; p < base_path_end; p++)
    {
        if (*p == '/')
            last_slash = p;
    }

    size_t directory_len = last_slash == NULL
        ? host_len
        : (size_t)(last_slash - base_url);

    int n = snprintf(
        out,
        out_size,
        "%.*s/%s",
        (int)directory_len,
        base_url,
        clean_src);

    return n > 0 && (size_t)n < out_size;
}

static int is_priority_script_url(const char *url)
{
    static const char *signals[] = {
        "googletagmanager.com",
        "google-analytics.com",
        "googleadservices.com",
        "doubleclick.net",
        "facebook.net",
        "facebook.com",
        "linkedin.com",
        "licdn.com",
        "tiktok.com",
        "bing.com",
        "clarity.ms",
        "ads-twitter.com",
        "pinterest.com",
        "sc-static.net",
        "reddit.com",
        "hotjar.com",
        "hubspot.com",
        "hs-scripts.com",
        "marketo.net",
        "adroll.com",
        "criteo.com",
        "taboola.com",
        "outbrain.com",
        "segment.com",
        "segment.io",
        "mixpanel.com",
        "amplitude.com",
        "heap.io"
    };

    for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); i++)
    {
        if (contains_ignore_case(url, signals[i]))
            return 1;
    }

    return 0;
}

static int add_script_url(
    char urls[PG_MAX_SCRIPT_URLS][PG_MAX_SCRIPT_URL_SIZE],
    int *count,
    const char *url)
{
    if (url == NULL || count == NULL ||
        *count >= PG_MAX_SCRIPT_URLS)
        return 0;

    for (int i = 0; i < *count; i++)
    {
        if (strcmp(urls[i], url) == 0)
            return 0;
    }

    size_t len = strlen(url);
    if (len == 0 || len >= PG_MAX_SCRIPT_URL_SIZE)
        return 0;

    memcpy(urls[*count], url, len + 1);
    (*count)++;
    return 1;
}

/*
 * Extract external script src attributes from script opening tags.
 * This does not execute JavaScript or parse script-generated URLs.
 */
static int collect_script_urls(
    const char *html,
    const char *base_url,
    char urls[PG_MAX_SCRIPT_URLS][PG_MAX_SCRIPT_URL_SIZE])
{
    int count = 0;
    const char *cursor = html;

    while (cursor != NULL && *cursor != '\0' &&
           count < PG_MAX_SCRIPT_URLS)
    {
        const char *tag = find_ignore_case(cursor, "<script");
        if (tag == NULL)
            break;

        const char *tag_end = strchr(tag, '>');
        if (tag_end == NULL)
            break;

        const char *src = find_ignore_case(tag, "src");
        if (src == NULL || src >= tag_end)
        {
            cursor = tag_end + 1;
            continue;
        }

        /* Ensure "src" begins an attribute name. */
        if (src > tag && (isalnum((unsigned char)src[-1]) ||
                          src[-1] == '-' || src[-1] == '_'))
        {
            cursor = tag_end + 1;
            continue;
        }

        const char *p = src + 3;
        while (p < tag_end && isspace((unsigned char)*p))
            p++;

        if (p >= tag_end || *p != '=')
        {
            cursor = tag_end + 1;
            continue;
        }

        p++;
        while (p < tag_end && isspace((unsigned char)*p))
            p++;

        char quote = '\0';
        if (p < tag_end && (*p == '"' || *p == '\''))
        {
            quote = *p;
            p++;
        }

        const char *value_start = p;

        if (quote != '\0')
        {
            while (p < tag_end && *p != quote)
                p++;
        }
        else
        {
            while (p < tag_end && !isspace((unsigned char)*p))
                p++;
        }

        size_t value_len = (size_t)(p - value_start);

        if (value_len > 0 && value_len < PG_MAX_SCRIPT_URL_SIZE)
        {
            char raw_src[PG_MAX_SCRIPT_URL_SIZE];
            char resolved[PG_MAX_SCRIPT_URL_SIZE];

            memcpy(raw_src, value_start, value_len);
            raw_src[value_len] = '\0';

            if (resolve_script_url(
                    base_url, raw_src, resolved, sizeof(resolved)))
            {
                add_script_url(urls, &count, resolved);
            }
        }

        cursor = tag_end + 1;
    }

    return count;
}

static void inspect_external_scripts(
    PGCompany *company,
    const char *html,
    const char *base_url)
{
    char urls[PG_MAX_SCRIPT_URLS][PG_MAX_SCRIPT_URL_SIZE];

    int count = collect_script_urls(html, base_url, urls);
    int fetched = 0;

    /*
     * Pass 1: advertising and analytics scripts first.
     * Pass 2: remaining scripts, up to the configured limit.
     */
    for (int pass = 0; pass < 2 && fetched < PG_MAX_SCRIPTS_TO_FETCH; pass++)
    {
        for (int i = 0; i < count && fetched < PG_MAX_SCRIPTS_TO_FETCH; i++)
        {
            int priority = is_priority_script_url(urls[i]);

            if ((pass == 0 && !priority) ||
                (pass == 1 && priority))
                continue;

            PGHttpResponse script_response;

            if (pg_http_get(urls[i], &script_response) != 0)
                continue;

            fetched++;

            if (script_response.status >= 200 &&
                script_response.status < 300 &&
                script_response.body != NULL)
            {
                detect_technologies(company, script_response.body);
            }

            pg_http_free(&script_response);
        }
    }
}

PGStatus pg_extract_technology(PGContext *ctx)
{
    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    const char *html = ctx->response.body;

    if (html == NULL)
        return PG_STATUS_PARSE_ERROR;

    PGCompany *company = &ctx->company;
    company->technology_count = 0;

    /* First inspect the initial HTML. */
    detect_technologies(company, html);

    /*
     * Then inspect a bounded number of referenced external scripts.
     * Prefer the effective URL after redirects.
     */
    const char *base_url =
        ctx->response.final_url[0] != '\0'
            ? ctx->response.final_url
            : ctx->url;

    inspect_external_scripts(company, html, base_url);

    return PG_STATUS_OK;
}
