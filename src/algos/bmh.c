#include "algo.h"

unsigned char *bmh_pre_process(const char *pattern, const size_t length)
{   
    unsigned char *table = (unsigned char *)malloc(sizeof(char) * 256);

    if (!table)
        LOG_ERROR("Memory allocation failed!");

    for (size_t i = 0; i < 256; i++)
        table[i] = length;

    for (size_t i = 0; i < length - 1; i++)
        table[(unsigned char)pattern[i]] = length - 1 - i;

    return table;
}

size_t bmh_search(const unsigned char *table, const char *pattern, const size_t pattern_length, const char *string, const size_t string_length)
{
    size_t skip = 0;

    while (skip <= string_length - pattern_length)
    {
        size_t j = 0;
        for (; j < pattern_length; j++) {
            if (string[skip + j] != pattern[j])
                break;
        }
        if (j == pattern_length)
            return skip;

        skip += table[(unsigned char)string[skip + pattern_length - 1]];
    }

    return BMH_NOT_FOUND;
}

int bmh_count(const char *pattern, const char *string)
{
    size_t pattern_length = strlen(pattern);
    size_t string_length = strlen(string);
    unsigned char *char_table = bmh_pre_process(pattern, pattern_length);
    size_t loc = 0;
    int count = 0;

    while (loc <= string_length - pattern_length)
    {
        size_t match = bmh_search(char_table, pattern, pattern_length, string + loc, string_length - loc);

        if (match == BMH_NOT_FOUND)
            break;

        count++;
        loc += match + pattern_length;
    }

    free(char_table);
    return count;
}
