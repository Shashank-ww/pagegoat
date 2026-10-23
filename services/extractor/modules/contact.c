
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "pagegoat/services/extractor/modules/contact.h"

#define MAX_PAGE_TEXT 2000000

static int is_email_local_char(char c)
{
    return isalnum((unsigned char)c) ||
           c == '.' || c == '_' || c == '%' ||
           c == '+' || c == '-';
}

static int is_email_domain_char(char c)
{
    return isalnum((unsigned char)c) ||
           c == '.' || c == '-';
}

static int is_phone_separator(char c)
{
    return c == ' ' || c == '-' ||
           c == '(' || c == ')' ||
           c == '.';
}

static int contains_ci(const char *text, const char *word)
{
    size_t n;

    if (text == NULL || word == NULL)
        return 0;

    n = strlen(word);

    if (n == 0)
        return 1;

    for (; *text; text++)
    {
        size_t i = 0;

        while (i < n &&
               text[i] != '\0' &&
               tolower((unsigned char)text[i]) ==
               tolower((unsigned char)word[i]))
        {
            i++;
        }

        if (i == n)
            return 1;
    }

    return 0;
}

static int email_exists(PGContext *ctx, const char *email)
{
    for (int i = 0; i < ctx->company.email_count; i++)
    {
        if (strcmp(ctx->company.emails[i], email) == 0)
            return 1;
    }

    return 0;
}

static int phone_exists(PGContext *ctx, const char *phone)
{
    for (int i = 0; i < ctx->company.phone_count; i++)
    {
        if (strcmp(ctx->company.phones[i], phone) == 0)
            return 1;
    }

    return 0;
}

static void add_email(PGContext *ctx, const char *candidate)
{
    size_t len;
    const char *at;
    const char *domain;
    const char *dot;
    char email[256];

    if (candidate == NULL)
        return;

    len = strlen(candidate);

    if (len < 6 || len >= sizeof(email))
        return;

    at = strchr(candidate, '@');

    if (at == NULL || at == candidate || strchr(at + 1, '@') != NULL)
        return;

    /* Validate the local part. */
    if (candidate[0] == '.' ||
        at[-1] == '.' ||
        strstr(candidate, "..") != NULL)
        return;

    for (const char *p = candidate; p < at; p++)
    {
        if (!is_email_local_char(*p))
            return;
    }

    domain = at + 1;

    if (*domain == '\0' || *domain == '.' || strchr(domain, '.') == NULL)
        return;

    for (const char *p = domain; *p; p++)
    {
        if (!is_email_domain_char(*p))
            return;

        if (*p == '.' && (p[1] == '\0' || p[1] == '.'))
            return;

        if (*p == '-' &&
            (p == domain || p[1] == '\0' ||
             p[-1] == '.' || p[1] == '.'))
            return;
    }

    dot = strrchr(domain, '.');

    if (dot == NULL || strlen(dot + 1) < 2)
        return;

    for (const char *p = dot + 1; *p; p++)
    {
        if (!isalpha((unsigned char)*p))
            return;
    }

    memcpy(email, candidate, len + 1);

    if (email_exists(ctx, email))
        return;

    if (ctx->company.email_count < PG_MAX_EMAILS)
    {
        strcpy(
            ctx->company.emails[ctx->company.email_count],
            email);

        ctx->company.email_count++;
    }
}

static void extract_emails_from_text(PGContext *ctx, const char *text)
{
    const char *p = text;

    while ((p = strchr(p, '@')) != NULL)
    {
        const char *start = p;
        const char *end = p + 1;
        char candidate[256];
        size_t len;

        while (start > text && is_email_local_char(start[-1]))
            start--;

        while (*end && is_email_domain_char(*end))
            end++;

        len = (size_t)(end - start);

        if (len > 0 && len < sizeof(candidate))
        {
            memcpy(candidate, start, len);
            candidate[len] = '\0';
            add_email(ctx, candidate);
        }

        p++;
    }
}

static void extract_mailto_links(PGContext *ctx, const char *html)
{
    const char *p = html;

    while (*p)
    {
        if (tolower((unsigned char)p[0]) == 'm' &&
            tolower((unsigned char)p[1]) == 'a' &&
            tolower((unsigned char)p[2]) == 'i' &&
            tolower((unsigned char)p[3]) == 'l' &&
            tolower((unsigned char)p[4]) == 't' &&
            tolower((unsigned char)p[5]) == 'o' &&
            p[6] == ':')
        {
            char email[256];
            size_t i = 0;
            p += 7;

            while (*p && *p != '"' && *p != '\'' &&
                   *p != '>' && *p != '?' &&
                   !isspace((unsigned char)*p) &&
                   i < sizeof(email) - 1)
            {
                email[i++] = *p++;
            }

            email[i] = '\0';
            add_email(ctx, email);
        }
        else
        {
            p++;
        }
    }
}

static void add_phone(
    PGContext *ctx,
    const char *candidate,
    int from_tel_link,
    int has_context)
{
    char phone[64];
    size_t len;
    int digits = 0;
    int formatted = 0;
    size_t out = 0;

    if (candidate == NULL)
        return;

    len = strlen(candidate);

    if (len == 0 || len >= sizeof(phone))
        return;

    for (size_t i = 0; i < len; i++)
    {
        unsigned char c = (unsigned char)candidate[i];

        if (isdigit(c))
        {
            digits++;
        }
        else if (is_phone_separator((char)c) || c == '+')
        {
            formatted = 1;
        }
        else
        {
            return;
        }
    }

    if (digits < 10 || digits > 15)
        return;

    /* Plain digit sequences need stronger evidence than formatting alone. */
    if (!from_tel_link && !formatted && !has_context)
        return;

    for (size_t i = 0; i < len; i++)
    {
        if (isdigit((unsigned char)candidate[i]) ||
            (candidate[i] == '+' && out == 0))
        {
            phone[out++] = candidate[i];
        }
    }

    phone[out] = '\0';

    if (phone_exists(ctx, phone))
        return;

    if (ctx->company.phone_count < PG_MAX_PHONES)
    {
        strcpy(
            ctx->company.phones[ctx->company.phone_count],
            phone);

        ctx->company.phone_count++;
    }
}

static void extract_tel_links(PGContext *ctx, const char *html)
{
    const char *p = html;

    while (*p)
    {
        if (tolower((unsigned char)p[0]) == 't' &&
            tolower((unsigned char)p[1]) == 'e' &&
            tolower((unsigned char)p[2]) == 'l' &&
            p[3] == ':')
        {
            char phone[64];
            size_t i = 0;
            p += 4;

            while (*p && *p != '"' && *p != '\'' &&
                   *p != '>' && *p != '?' &&
                   !isspace((unsigned char)*p) &&
                   i < sizeof(phone) - 1)
            {
                phone[i++] = *p++;
            }

            phone[i] = '\0';
            add_phone(ctx, phone, 1, 0);
        }
        else
        {
            p++;
        }
    }
}

static char *visible_text_from_html(const char *html)
{
    size_t input_len = strlen(html);
    char *text = malloc(input_len + 1);
    size_t out = 0;
    int in_tag = 0;
    int in_script = 0;
    int in_style = 0;

    if (text == NULL)
        return NULL;

    for (size_t i = 0; i < input_len;)
    {
        if (!in_tag && html[i] == '<')
        {
            if (contains_ci(html + i, "<script"))
            {
                const char *end = strstr(html + i, ">");
                if (end != NULL)
                {
                    const char *close = end + 1;
                    while (*close &&
                           !(close[0] == '<' &&
                             close[1] == '/' &&
                             contains_ci(close, "</script")))
                        close++;

                    if (*close)
                    {
                        const char *close_end = strchr(close, '>');
                        i = close_end ? (size_t)(close_end - html) + 1
                                      : input_len;
                        continue;
                    }
                }
            }

            if (contains_ci(html + i, "<style"))
            {
                const char *end = strstr(html + i, ">");
                if (end != NULL)
                {
                    const char *close = end + 1;
                    while (*close &&
                           !(close[0] == '<' &&
                             close[1] == '/' &&
                             contains_ci(close, "</style")))
                        close++;

                    if (*close)
                    {
                        const char *close_end = strchr(close, '>');
                        i = close_end ? (size_t)(close_end - html) + 1
                                      : input_len;
                        continue;
                    }
                }
            }

            in_tag = 1;
            if (out > 0 && text[out - 1] != ' ')
                text[out++] = ' ';
            i++;
            continue;
        }

        if (in_tag)
        {
            if (html[i] == '>')
                in_tag = 0;
            i++;
            continue;
        }

        if (!in_script && !in_style)
        {
            unsigned char c = (unsigned char)html[i];

            if (isspace(c))
            {
                if (out > 0 && text[out - 1] != ' ')
                    text[out++] = ' ';
            }
            else if (c >= 32)
            {
                text[out++] = (char)c;
            }
        }

        i++;
    }

    text[out] = '\0';
    return text;
}

static void extract_phones_from_text(PGContext *ctx, const char *text)
{
    const char *p = text;

    while (*p)
    {
        if (isdigit((unsigned char)*p) ||
            (*p == '+' && isdigit((unsigned char)p[1])))
        {
            char candidate[64];
            size_t n = 0;
            int digits = 0;

            while (*p &&
                   (isdigit((unsigned char)*p) ||
                    is_phone_separator(*p) ||
                    (*p == '+' && n == 0)) &&
                   n < sizeof(candidate) - 1)
            {
                if (isdigit((unsigned char)*p))
                    digits++;

                candidate[n++] = *p++;
            }

            candidate[n] = '\0';

            if (digits >= 10 && digits <= 15)
            {
                const char *context_start = p - n - 40;
                const char *context_end = p + 40;
                char context[160];
                size_t context_len;

                if (context_start < text)
                    context_start = text;

                context_len = (size_t)(context_end - context_start);

                if (context_len >= sizeof(context))
                    context_len = sizeof(context) - 1;

                memcpy(context, context_start, context_len);
                context[context_len] = '\0';

                int has_context =
                    contains_ci(context, "phone") ||
                    contains_ci(context, "call") ||
                    contains_ci(context, "contact") ||
                    contains_ci(context, "sales") ||
                    contains_ci(context, "support") ||
                    contains_ci(context, "tel:");

                add_phone(ctx, candidate, 0, has_context);
            }
        }
        else
        {
            p++;
        }
    }
}

PGStatus pg_extract_contact(PGContext *ctx)
{
    char *visible_text;

    if (ctx == NULL)
        return PG_STATUS_INVALID_ARGUMENT;

    if (ctx->response.body == NULL)
        return PG_STATUS_PARSE_ERROR;

    extract_mailto_links(ctx, ctx->response.body);
    extract_tel_links(ctx, ctx->response.body);

    visible_text = visible_text_from_html(ctx->response.body);

    if (visible_text == NULL)
        return PG_STATUS_ERROR;

    extract_emails_from_text(ctx, visible_text);
    extract_phones_from_text(ctx, visible_text);

    free(visible_text);

    return PG_STATUS_OK;
}
