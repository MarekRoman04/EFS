#include "algo.h"

static inline size_t bmh_search(const bmh_table *table, const char *pattern, unsigned char pattern_length,
                                const char *data, size_t data_length, unsigned char start_idx, unsigned char *end_idx);
bmh_table *bmh_pre_process(const char *pattern, unsigned char pattern_length);
int bmh_count(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
              size_t data_length, unsigned char start_idx, unsigned char *end_idx);
int bmh_find(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
             size_t data_length, unsigned char start_idx, unsigned char *end_idx);

// Returns table containing skippable characters for each char in pattern
bmh_table *bmh_pre_process(const char *pattern, unsigned char pattern_length)
{
    bmh_table *table = (bmh_table *)malloc(sizeof(unsigned char) * 256);

    if (!table)
        log_error("Memory allocation failed!");

    for (unsigned char i = 0; i < 255; i++)
        table[i] = pattern_length;

    for (unsigned char i = 0; i < pattern_length - 1; i++)
        table[(unsigned char)pattern[i]] = pattern_length - 1 - i;

    return table;
}

/*
 * Returns location of pattern in given data, if not found returns BHM_NOT_FOUND,
 * and sets end_idx to number of match characters
 */
static inline size_t bmh_search(const bmh_table *table, const char *pattern, unsigned char pattern_length,
                                const char *data, size_t data_length, unsigned char start_idx, unsigned char *end_idx)
{
    if (data_length < pattern_length)
        return BMH_NOT_FOUND;

    size_t loc = 0;
    size_t j = 0;

    while (loc <= data_length - pattern_length)
    {
        j = start_idx;
        start_idx = 0;

        for (; j < pattern_length; j++)
        {
            if (data[loc + j] != pattern[j])
                break;
        }
        if (j == pattern_length)
        {
            *end_idx = 0;
            return loc;
        }

        loc += table[(unsigned char)data[loc + pattern_length - 1]];
    }

    *end_idx = j;
    return BMH_NOT_FOUND;
}

/*
 * Returns number of occurences of pattern in given data,
 * sets end_idx to last matched character index in pattern before end of data
 */
int bmh_count(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
              size_t data_length, unsigned char start_idx, unsigned char *end_idx)
{
    int found = 0;
    size_t data_loc = 0;

    while (data_loc < data_length)
    {
        size_t loc;
        if ((loc = bmh_search(table, pattern, pattern_length, data + data_loc, data_length - data_loc, start_idx, end_idx)) == BMH_NOT_FOUND)
            return found;

        found++;
        data_loc += loc + pattern_length;
    }

    return found;
}

/*
 * Returns 0 if data contains pattern,
 * sets end_idx to last matched character index in pattern before end of data
 */
int bmh_find(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
             size_t data_length, unsigned char start_idx, unsigned char *end_idx)
{
    return bmh_search(table, pattern, pattern_length, data, data_length, start_idx, end_idx) == BMH_NOT_FOUND ? 1 : 0;
}
