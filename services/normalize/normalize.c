#include "pagegoat/services/normalize/normalize.h"

#include <ctype.h>
#include <string.h>
#include <stdint.h>

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

typedef struct
{
    const char *name;
    uint32_t value;
} PGHtmlEntity;

static const PGHtmlEntity pg_html_entities[] =
{
    {"amp", 38},
    {"AMP", 38},
    {"lt", 60},
    {"LT", 60},
    {"gt", 62},
    {"GT", 62},
    {"quot", 34},
    {"QUOT", 34},
    {"apos", 39},
    {"nbsp", 160},
    {"NBSP", 160},

    {"copy", 169},
    {"COPY", 169},
    {"reg", 174},
    {"REG", 174},
    {"trade", 8482},

    {"cent", 162},
    {"pound", 163},
    {"yen", 165},
    {"euro", 8364},

    {"ndash", 8211},
    {"mdash", 8212},
    {"hellip", 8230},
    {"bull", 8226},
    {"middot", 183},

    {"lsquo", 8216},
    {"rsquo", 8217},
    {"ldquo", 8220},
    {"rdquo", 8221},

    {"laquo", 171},
    {"raquo", 187},

    {"times", 215},
    {"divide", 247},
    {"deg", 176},
    {"plusmn", 177}
};

static int pg_html_entity_lookup(
    const char *name,
    size_t name_length,
    uint32_t *value)
{
    size_t count =
        sizeof(pg_html_entities) /
        sizeof(pg_html_entities[0]);

    for (size_t i = 0; i < count; i++)
    {
        if (strlen(pg_html_entities[i].name) == name_length &&
            memcmp(
                pg_html_entities[i].name,
                name,
                name_length) == 0)
        {
            *value = pg_html_entities[i].value;
            return 1;
        }
    }

    return 0;
}

static int pg_html_numeric_entity(
    const char *name,
    size_t name_length,
    uint32_t *value)
{
    size_t position = 1;
    unsigned int base = 10;
    uint32_t result = 0;

    if (name_length < 2 || name[0] != '#')
    {
        return 0;
    }

    if (position < name_length &&
        (name[position] == 'x' ||
         name[position] == 'X'))
    {
        base = 16;
        position++;
    }

    if (position >= name_length)
    {
        return 0;
    }

    for (; position < name_length; position++)
    {
        unsigned char c = (unsigned char)name[position];
        unsigned int digit;

        if (c >= '0' && c <= '9')
        {
            digit = c - '0';
        }
        else if (base == 16 &&
                 c >= 'a' && c <= 'f')
        {
            digit = c - 'a' + 10;
        }
        else if (base == 16 &&
                 c >= 'A' && c <= 'F')
        {
            digit = c - 'A' + 10;
        }
        else
        {
            return 0;
        }

        if (digit >= base ||
            result > (0x10FFFF - digit) / base)
        {
            return 0;
        }

        result = result * base + digit;
    }

    /*
     * Reject invalid Unicode scalar values.
     */
    if (result == 0 ||
        result > 0x10FFFF ||
        (result >= 0xD800 && result <= 0xDFFF))
    {
        return 0;
    }

    *value = result;
    return 1;
}

static size_t pg_utf8_encode(
    uint32_t codepoint,
    char output[4])
{
    if (codepoint <= 0x7F)
    {
        output[0] = (char)codepoint;
        return 1;
    }

    if (codepoint <= 0x7FF)
    {
        output[0] = (char)(0xC0 | (codepoint >> 6));
        output[1] = (char)(0x80 | (codepoint & 0x3F));
        return 2;
    }

    if (codepoint <= 0xFFFF)
    {
        output[0] = (char)(0xE0 | (codepoint >> 12));
        output[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
        output[2] = (char)(0x80 | (codepoint & 0x3F));
        return 3;
    }

    output[0] = (char)(0xF0 | (codepoint >> 18));
    output[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
    output[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
    output[3] = (char)(0x80 | (codepoint & 0x3F));

    return 4;
}

PGStatus pg_decode_html_entities(
    const char *input,
    char *output,
    size_t output_size)
{
    size_t input_position = 0;
    size_t output_position = 0;
    size_t input_length;

    if (input == NULL ||
        output == NULL ||
        output_size == 0)
    {
        return PG_STATUS_INVALID_ARGUMENT;
    }

    output[0] = '\0';

    input_length = strlen(input);

    while (input_position < input_length)
    {
        uint32_t codepoint = 0;
        int decoded = 0;

        if (input[input_position] == '&')
        {
            size_t entity_start = input_position + 1;
            size_t entity_end = entity_start;

            while (entity_end < input_length &&
                   input[entity_end] != ';' &&
                   entity_end - entity_start <= 32)
            {
                entity_end++;
            }

            if (entity_end < input_length &&
                input[entity_end] == ';' &&
                entity_end > entity_start &&
                entity_end - entity_start <= 32)
            {
                size_t entity_length =
                    entity_end - entity_start;

                if (input[entity_start] == '#')
                {
                    decoded = pg_html_numeric_entity(
                        input + entity_start,
                        entity_length,
                        &codepoint);
                }
                else
                {
                    decoded = pg_html_entity_lookup(
                        input + entity_start,
                        entity_length,
                        &codepoint);
                }

                if (decoded)
                {
                    char utf8[4];

                    size_t utf8_length =
                        pg_utf8_encode(codepoint, utf8);

                    if (output_position + utf8_length >= output_size)
                    {
                        output[output_position] = '\0';
                        return PG_STATUS_WARNING;
                    }

                    memcpy(
                        output + output_position,
                        utf8,
                        utf8_length);

                    output_position += utf8_length;

                    input_position = entity_end + 1;
                    continue;
                }
            }
        }

        /*
         * Copy the original character when no valid
         * entity was found. This preserves unknown
         * and malformed entities.
         */
        if (output_position + 1 >= output_size)
        {
            output[output_position] = '\0';
            return PG_STATUS_WARNING;
        }

        output[output_position++] = input[input_position++];
    }

    output[output_position] = '\0';

    return PG_STATUS_OK;
}
