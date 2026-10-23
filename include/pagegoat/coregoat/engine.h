#ifndef PAGEGOAT_ENGINE_H
#define PAGEGOAT_ENGINE_H

#include "pagegoat/coregoat/context.h"
#include "pagegoat/coregoat/status.h"

PGStatus pg_engine_start(PGContext *ctx);

PGStatus pg_engine_shutdown(PGContext *ctx);

#endif