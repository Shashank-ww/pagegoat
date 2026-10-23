#include "pagegoat/platform/config.h"

PGStatus pg_config_load(PGConfig *config)
{
    if (config == 0)
        return PG_STATUS_INVALID_ARGUMENT;

    config->timeout_seconds = 20;
    config->max_redirects = 5;

    return PG_STATUS_OK;
}