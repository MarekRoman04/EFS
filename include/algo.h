#ifndef ALGO_H
#define ALGO_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "log.h"

#define BMH_NOT_FOUND ((size_t)-1)

unsigned char *bmh_pre_process(const char *pattern, const size_t length);
int bmh_count(unsigned char *table, const string *str_pattern, const string *str_string);
int bmh_find(unsigned char *table, const string *str_pattern, const string *str_string);

#endif