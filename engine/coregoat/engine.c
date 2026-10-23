#include "pagegoat/coregoat/engine.h"

#include "pagegoat/platform/logger.h"

PGStatus pg_engine_start(PGContext *ctx)
{
    (void)ctx;

    pg_log_ok("Engine");

    return PG_STATUS_OK;
}

PGStatus pg_engine_shutdown(PGContext *ctx)
{
    (void)ctx;

    pg_log_ok("Engine Shutdown");

    return PG_STATUS_OK;
}