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

void log_errno(int level, const char *path)
{
    void (*log_func)(const char *fmt, ...) = (level ? log_error : log_info);

    switch (errno)
    {
    case ENOENT:
        log_func("Ignoring: %s, path does not exist!\n", path);
        break;
    case EACCES:
        log_func("Ignoring: %s, permission denied!\n", path);
        break;
    case ENOTDIR:
        log_func("Ignoring: %s, not a directory!\n", path);
        break;
    default:
        log_func("Ignoring: %s, error (%s)\n", path, strerror(errno));
        break;
    }
}