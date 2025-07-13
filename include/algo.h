#ifndef ALGO_H
#define ALGO_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define BMH_NOT_FOUND ((size_t)-1)

typedef unsigned char bmh_table;

bmh_table *bmh_pre_process(const char *pattern, unsigned char pattern_length);
int bmh_count(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
              size_t data_length, unsigned char start_idx, unsigned char *end_idx);
int bmh_find(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
             size_t data_length, unsigned char start_idx, unsigned char *end_idx);

#endif