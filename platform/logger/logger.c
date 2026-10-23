#include "pagegoat/platform/logger.h"

#include <stdio.h>

void pg_logger_init(void)
{
    puts("Logger initialized.");
}

void pg_log_info(const char *message)
{
    printf("[INFO] %s\n", message);
}

void pg_log_ok(const char *message)
{
    printf("[ OK ] %s\n", message);
}

void pg_log_warning(const char *message)
{
    printf("[WARN] %s\n", message);
}

void pg_log_error(const char *message)
{
    printf("[FAIL] %s\n", message);
}

void pg_logger_shutdown(void)
{
    puts("Logger shutdown.");
}