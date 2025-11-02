#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

void* log_malloc(size_t size, const char *file, int line);
void* log_calloc(size_t elems, size_t size, const char *file, int line);
void log_error_impl(const char *file, int line, const char *fmt, ...);
void log_info_impl(const char *file, int line, const char *fmt, ...);
void log_errno_impl(int level, const char *path, const char *file, int line);


#ifdef DEBUG
#define malloc(size) log_malloc(size, __FILE__, __LINE__)
#define calloc(num, size) log_calloc(num, size, __FILE__, __LINE__)
#define log_error(fmt, ...) \
    log_error_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) \
    log_info_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_errno(level, path) \
    log_errno_impl(level, path, __FILE__, __LINE__)
#else
#define log_error(fmt, ...) \
    log_error_impl(NULL, 0, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) \
    log_info_impl(NULL, 0, fmt, ##__VA_ARGS__)
#define log_errno(level, path) \
    log_errno_impl(level, path, NULL, 0)
#endif

#endif