#ifndef ALGO_H
#define ALGO_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define BMH_NOT_FOUND ((size_t)-1)

#define BMH_IGNORE_CASE (1 << 0)
#define BHM_WORD (1 << 1)

#define FLAG_SET(flags, bit) (!!(flags & bit))

typedef unsigned char bmh_table;

typedef struct bmh_search_data
{
    const bmh_table *table;
    const char *pattern;
    unsigned char pattern_length;
    char *data;
    size_t data_length;
    unsigned int flags;
    unsigned char idx;
} bmh_search_data;

bmh_table *bmh_pre_process(const char *pattern, unsigned char pattern_length);
int bmh_count(bmh_search_data *bmh_sd);
int bmh_find(bmh_search_data *bmh_sd);

#endif