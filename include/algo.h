#ifndef ALGO_H
#define ALGO_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

#define BMH_NOT_FOUND ((size_t)-1)

unsigned char *bmh_pre_process(const char *pattern, const size_t length);
size_t bmh_search(const unsigned char *table, const char *pattern, const size_t pattern_length, const char *string, const size_t string_length);
int bmh_count(const char *pattern, const char *string);

int string_search_count(const char *pattern, const char *f_path);

#endif