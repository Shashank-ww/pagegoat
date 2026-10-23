#ifndef PAGEGOAT_CONFIG_H
#define PAGEGOAT_CONFIG_H

#include "pagegoat/coregoat/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PGConfig
{
    int timeout_seconds;
    int max_redirects;
} PGConfig;

PGStatus pg_config_load(PGConfig *config);

#ifdef __cplusplus
}
#endif

#endif