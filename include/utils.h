#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <string.h>
#include "log.h"

#define FILE_SIZE(fp, size_var)       \
    do {                              \
        long _cur = ftell(fp);        \
        fseek(fp, 0L, SEEK_END);      \
        size_var = ftell(fp);         \
        fseek(fp, _cur, SEEK_SET);    \
    } while (0)

char *load_file(const char *f_path);

#endif