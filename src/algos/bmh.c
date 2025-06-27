#include "algo.h"

int *bmh_pre_process(const char *pattern, const size_t length)
{
    int *char_table = (int *)malloc(sizeof(int) * 256);

    if (!char_table)
        LOG_ERROR("Memory allocation failed!");

    for (size_t i = 0; i < 256; i++)
        char_table[i] = length;

    if (length <= 1)
        return char_table;

    for (size_t i = 0; i < length - 1; i++)
        char_table[(unsigned char)pattern[i]] = length - 1 - i;

    return char_table;
}

size_t bmh_search(const int *table, const char *pattern, const size_t pattern_length, const char *string, const size_t string_length)
{
    size_t skip = 0;

    while (skip <= string_length - pattern_length)
    {
        if (!memcmp(string + skip, pattern, pattern_length))
            return skip;

        skip += table[(unsigned char)string[skip + pattern_length - 1]];
    }

    return BMH_NOT_FOUND;
}

int bmh_count(const char *pattern, const char *string)
{
    size_t pattern_length = strlen(pattern);
    size_t string_length = strlen(string);
    int *char_table = bmh_pre_process(pattern, pattern_length);
    size_t loc = 0;
    int count = 0;

    while (loc < string_length - pattern_length)
    {
        size_t match = bmh_search(char_table, pattern, pattern_length, string + loc, string_length - loc);

        if (match != BMH_NOT_FOUND)
            count++;

        loc += match + pattern_length;
    }

    free(char_table);
    return count;
}
