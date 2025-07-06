#include "algo.h"
#include "data.h"

static inline size_t bmh_search(const unsigned char *table, const char *pattern, const size_t pattern_length, const char *string, const size_t string_length);
unsigned char *bmh_pre_process(const char *pattern, const size_t length);
int bmh_count(unsigned char *table, const string *str_pattern, const string *str_string);
int bmh_find(unsigned char *table, const string *str_pattern, const string *str_string);

unsigned char *bmh_pre_process(const char *pattern, const size_t length)
{
    unsigned char *table = (unsigned char *)malloc(256);

    if (!table)
        log_error("Memory allocation failed!");

    for (unsigned char i = 0; i < 255; i++)
        table[i] = length;

    for (unsigned char i = 0; i < length - 1; i++)
        table[(unsigned char)pattern[i]] = length - 1 - i;

    return table;
}

size_t bmh_search(const unsigned char *table, const char *pattern, const size_t pattern_length, const char *string, const size_t string_length)
{
    size_t skip = 0;

    while (skip <= string_length - pattern_length)
    {
        size_t j = 0;
        for (; j < pattern_length; j++)
        {
            if (string[skip + j] != pattern[j])
                break;
        }
        if (j == pattern_length)
            return skip;

        skip += table[(unsigned char)string[skip + pattern_length - 1]];
    }

    return BMH_NOT_FOUND;
}

int bmh_count(unsigned char *table, const string *str_pattern, const string *str_string)
{
    const char *pattern = str_pattern->data;
    size_t pattern_length = str_pattern->length;
    const char *string = str_string->data;
    size_t string_length = str_string->length;
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

int bmh_find(unsigned char *table, const string *str_pattern, const string *str_string)
{
    const char *pattern = str_pattern->data;
    size_t pattern_length = str_pattern->length;
    const char *string = str_string->data;
    size_t string_length = str_string->length;
    unsigned char *char_table = bmh_pre_process(pattern, pattern_length);

    return bmh_search(char_table, pattern, pattern_length, string, string_length);
}