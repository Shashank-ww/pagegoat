#ifndef PAGEGOAT_LOGGER_H
#define PAGEGOAT_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

void pg_logger_init(void);

void pg_log_info(const char *message);

void pg_log_ok(const char *message);

void pg_log_warning(const char *message);

void pg_log_error(const char *message);

void pg_logger_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif