#include "algo.h"

size_t *bm_pre_process(const char *pattern, const size_t length)
{   
    size_t *table = (size_t *)malloc(sizeof(size_t) * 256);

    if (!table)
        log_error("Memory allocation failed!\n");

    for (size_t i = 0; i < 255; i++)
        table[i] = length;

    for (size_t i = 0; i < length - 1; i++)
        table[(unsigned char)pattern[i]] = length - 1 - i;

    return table;
}