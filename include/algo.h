#ifndef ALGO_H
#define ALGO_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "log.h"

#define BMH_NOT_FOUND ((size_t)-1)

int bmh_count(const string *str_pattern, const string *str_string);
int bmh_find(const string *str_pattern, const string *str_string);

#endif