#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void log_info(const char *fmt, ...);
void log_error(const char *fmt, ...);

#endif