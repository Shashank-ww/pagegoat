
#include "pagegoat/services/normalize/normalize.h"

#include <ctype.h>
#include <string.h>

#define PG_MAX_INPUT_LENGTH 511

static void pg_trim(char *text)
{
    char *start = text;
    size_t length;

    while (*start && isspace((unsigned char)*start))
    {
        start++;
    }

    if (start != text)
    {
        memmove(text, start, strlen(start) + 1);
    }

    length = strlen(text);

    while (length > 0 &&
           isspace((unsigned char)text[length - 1]))
    {
        text[--length] = '\0';
    }
}

static int pg_starts_with_ci(
    const char *text,
    const char *prefix)
{
    while (*prefix)
    {
        if (tolower((unsigned char)*text) !=
            tolower((unsigned char)*prefix))
        {
            return 0;
        }

        text++;
        prefix++;
    }

    return 1;
}

PGStatus pg_validate_url(const char *url)
{
    const char *host;
    const char *host_end;
    const char *p;
    size_t host_length;
    int has_dot = 0;

    if (url == NULL || url[0] == '\0')
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    if (pg_starts_with_ci(url, "https://"))
    {
        host = url + 8;
    }
    else if (pg_starts_with_ci(url, "http://"))
    {
        host = url + 7;
    }
    else
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    if (*host == '\0' || *host == '/' ||
        *host == '?' || *host == '#')
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    host_end = host;

    while (*host_end &&
           *host_end != '/' &&
           *host_end != '?' &&
           *host_end != '#')
    {
        host_end++;
    }

    host_length = (size_t)(host_end - host);

    if (host_length == 0 || host_length > 253)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    /* Reject whitespace and URL user-info in the authority. */
    for (p = host; p < host_end; p++)
    {
        if (isspace((unsigned char)*p) || *p == '@')
        {
            return PG_STATUS_INVALID_ARGUMENT;
        }

        if (*p == '.')
        {
            has_dot = 1;
        }
    }

    /*
     * Require a dotted hostname for this first version.
     * Localhost and raw IP address support can be added
     * separately if the product requires them.
     */
    if (!has_dot)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    /* Validate ordinary DNS hostname characters. */
    for (p = host; p < host_end; p++)
    {
        unsigned char c = (unsigned char)*p;

        if (!(isalnum(c) || c == '.' || c == '-' ||
              c == ':'))
        {
            return PG_STATUS_INVALID_ARGUMENT;
        }
    }

    return PG_STATUS_OK;
}

PGStatus pg_normalize_input(
    const char *input,
    char *output,
    size_t output_size)
{
    char buffer[PG_MAX_INPUT_LENGTH + 1];
    const char *host;
    char *host_end;
    size_t input_length;
    size_t prefix_length;
    size_t host_length;
    size_t required;
    int explicit_scheme;

    if (input == NULL ||
        output == NULL ||
        output_size == 0)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    output[0] = '\0';

    input_length = strlen(input);

    if (input_length == 0 ||
        input_length > PG_MAX_INPUT_LENGTH)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    memcpy(buffer, input, input_length + 1);
    pg_trim(buffer);

    if (buffer[0] == '\0')
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    /*
     * This CLI accepts website input, not company names
     * or search queries. Bare single words retain the
     * existing convenience behavior: "apple" -> apple.com.
     */
    explicit_scheme =
        pg_starts_with_ci(buffer, "https://") ||
        pg_starts_with_ci(buffer, "http://");

    if (explicit_scheme)
    {
        if (pg_starts_with_ci(buffer, "http://"))
        {
            memmove(
                buffer + 8,
                buffer + 7,
                strlen(buffer + 7) + 1);

            memcpy(buffer, "https://", 8);
        }

        prefix_length = 8;
        host = buffer + prefix_length;
    }
    else if (pg_starts_with_ci(buffer, "www."))
    {
        prefix_length = 8;

        if (strlen(buffer) + prefix_length - 4 >=
            sizeof(buffer))
        {
            return PG_STATUS_INVALID_ARGUMENT;
        }

        memmove(
            buffer + prefix_length,
            buffer + 4,
            strlen(buffer + 4) + 1);

        memcpy(buffer, "https://", prefix_length);
        host = buffer + prefix_length;
    }
    else
    {
        /*
         * A bare domain gets www. A single word gets .com.
         * Paths and query strings are preserved.
         */
        const char *first_separator = strpbrk(
            buffer,
            "/?#");

        size_t bare_host_length = first_separator
            ? (size_t)(first_separator - buffer)
            : strlen(buffer);

        int has_dot = 0;

        for (size_t i = 0; i < bare_host_length; i++)
        {
            if (buffer[i] == '.')
            {
                has_dot = 1;
                break;
            }
        }

        if (bare_host_length == 0)
        {
            return PG_STATUS_INVALID_ARGUMENT;
        }

        if (has_dot)
        {
            required = 8 + 4 + strlen(buffer) + 1;

            if (required > output_size)
            {
                return PG_STATUS_INVALID_ARGUMENT;
            }

            memcpy(output, "https://www.", 12);
            memcpy(output + 12, buffer, strlen(buffer) + 1);
        }
        else
        {
            required = 12 + strlen(buffer) + 4 + 1;

            if (required > output_size)
            {
                return PG_STATUS_INVALID_ARGUMENT;
            }

            memcpy(output, "https://www.", 12);
            memcpy(output + 12, buffer, strlen(buffer) + 1);

            /*
             * Insert ".com" after the hostname, before
             * any path, query string or fragment.
             */
            host_end = output + 12;

            while (*host_end &&
                   *host_end != '/' &&
                   *host_end != '?' &&
                   *host_end != '#')
            {
                host_end++;
            }

            memmove(
                host_end + 4,
                host_end,
                strlen(host_end) + 1);

            memcpy(host_end, ".com", 4);
        }

        return pg_validate_url(output);
    }

    /* Lowercase only the hostname, never the path/query. */
    host_end = (char *)host;

    while (*host_end &&
           *host_end != '/' &&
           *host_end != '?' &&
           *host_end != '#')
    {
        *host_end = (char)tolower(
            (unsigned char)*host_end);

        host_end++;
    }

    required = strlen(buffer) + 1;

    if (required > output_size)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    memcpy(output, buffer, required);

    return pg_validate_url(output);
}
