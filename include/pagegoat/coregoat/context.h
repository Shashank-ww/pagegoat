#ifndef PAGEGOAT_CONTEXT_H
#define PAGEGOAT_CONTEXT_H

#include "pagegoat/platform/http.h"
#include "pagegoat/coregoat/company.h"
#include "pagegoat/coregoat/status.h"

typedef void (*PGEventCallback)(
    void *user_data,
    const char *type,
    const char *message,
    const char *url);

typedef struct
{
    char url[512];

    PGHttpResponse response;

    PGCompany company;

    PGEventCallback event_callback;
    void *event_user_data;

} PGContext;

void pg_context_emit_event(
    PGContext *ctx,
    const char *type,
    const char *message,
    const char *url);

PGStatus
pg_context_init(
    PGContext *ctx);

PGStatus
pg_context_reset(
    PGContext *ctx);

PGStatus
pg_context_destroy(
    PGContext *ctx);

#endif