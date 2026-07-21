#ifndef PAGEGOAT_STATUS_H
#define PAGEGOAT_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    PG_STATUS_OK = 0,

    PG_STATUS_WARNING,

    PG_STATUS_ERROR,

    PG_STATUS_INVALID_ARGUMENT,

    PG_STATUS_NETWORK_ERROR,

    PG_STATUS_PARSE_ERROR,

    PG_STATUS_STORAGE_ERROR,

    PG_STATUS_EXPORT_ERROR

} PGStatus;

const char *pg_status_string(PGStatus status);

#ifdef __cplusplus
}
#endif

#endif