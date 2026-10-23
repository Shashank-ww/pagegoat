
#ifndef PAGEGOAT_EXPORT_H
#define PAGEGOAT_EXPORT_H

#include "pagegoat/coregoat/context.h"

PGStatus pg_export_company_json(
    const PGContext *ctx,
    const char *path);

#endif
