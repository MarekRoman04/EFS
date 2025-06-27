#include <stdio.h>
#include <stdlib.h>

#include "algo.h"
#include "utils.h"

int string_search_count(const char *pattern, const char *f_path)
{
    char* data = load_file(f_path);
    int count = bmh_count(pattern, data);

    free(data);
    return count;
}