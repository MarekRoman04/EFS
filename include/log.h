#ifndef LOG_H
#define LOG_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_errno(int level, const char *path);

#endif