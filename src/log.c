#include "log.h"

void log_info(const char *fmt, ...)
{
    #ifdef DEBUG
    fprintf(stderr, "[INFO] %s:%d\n", __FILE__, __LINE__);
    #endif
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...)
{
    #ifdef DEBUG
    fprintf(stderr, "[ERROR] %s:%d\n", __FILE__, __LINE__);
    #endif
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}
