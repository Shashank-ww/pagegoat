#include "pagegoat/coregoat/status.h"

const char *pg_status_string(PGStatus status)
{
    switch (status)
    {
        case PG_STATUS_OK:
            return "OK";

        case PG_STATUS_WARNING:
            return "WARNING";

        case PG_STATUS_ERROR:
            return "ERROR";

        case PG_STATUS_INVALID_ARGUMENT:
            return "INVALID_ARGUMENT";

        case PG_STATUS_NETWORK_ERROR:
            return "NETWORK_ERROR";

        case PG_STATUS_PARSE_ERROR:
            return "PARSE_ERROR";

        case PG_STATUS_STORAGE_ERROR:
            return "STORAGE_ERROR";

        case PG_STATUS_EXPORT_ERROR:
            return "EXPORT_ERROR";

        default:
            return "UNKNOWN";
    }
}