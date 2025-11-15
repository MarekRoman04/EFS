#include "log.h"

void log_error_impl(const char *file, int line, const char *fmt, ...)
{
    if (file)
        fprintf(stderr, "[ERROR] %s:%d: ", file, line);
    else
        fprintf(stderr, "[ERROR]: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
    exit(1);
}

void log_info_impl(const char *file, int line, const char *fmt, ...)
{
    if (file)
        fprintf(stderr, "[INFO] %s:%d: ", file, line);
    else
        fprintf(stderr, "[INFO]: ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

void log_errno_impl(int level, const char *path, const char *file, int line)
{
    void (*log_func)(const char *file, int line, const char *fmt, ...) = level ? log_error_impl : log_info_impl;

    switch (errno)
    {
    case ENOENT:
        log_func(file, line, "Ignoring: %s, path does not exist!", path);
        break;
    case EACCES:
        log_func(file, line, "Ignoring: %s, permission denied!", path);
        break;
    case EISDIR:
        log_func(file, line, "Ignoring: %s, is a directory!", path);
        break;
    case ENOTDIR:
        log_func(file, line, "Ignoring: %s, not a directory!", path);
        break;
    default:
        log_func(file, line, "Ignoring: %s, error (%s)", path, strerror(errno));
        break;
    }
}

void *log_malloc(size_t size, const char *file, int line)
{
#undef malloc
    void *p = malloc(size);
#define malloc(size) log_malloc(size, __FILE__, __LINE__)
    printf("malloc(%zu) at %s:%d -> %p\n", size, file, line, p);
    return p;
}

void *log_calloc(size_t elems, size_t size, const char *file, int line)
{
#undef calloc
    void *p = calloc(elems, size);
#define calloc(num, size) log_calloc(num, size, __FILE__, __LINE__)
    printf("calloc(%zu) at %s:%d -> %p\n", elems * size, file, line, p);
    return p;
}

void *log_realloc(void *ptr, size_t size, const char *file, int line)
{
#undef realloc
    printf("realloc(%p %zu) ", ptr, size);
    void *p = realloc(ptr, size);
#define realloc(ptr, size) log_realloc(ptr, size, __FILE__, __LINE__)
    printf("at %s:%d -> %p\n", file, line, p);
    return p;
}

void log_free(void *ptr, const char *file, int line)
{
#undef free
    printf("free(%p) at %s:%d\n", ptr, file, line);
    free(ptr);
#define free(ptr) log_free(ptr, __FILE__, __LINE__)
}