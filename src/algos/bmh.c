#include "algo.h"

static inline size_t bmh_search(const bmh_table *table, const char *pattern, unsigned char pattern_length,
                                const char *data, size_t data_length, unsigned char start_idx, unsigned char *end_idx);
static inline size_t bmh_search_i(const bmh_table *table, const char *pattern, unsigned char pattern_length,
                                  const char *data, size_t data_length, unsigned char start_idx, unsigned char *end_idx);
bmh_table *bmh_pre_process(const char *pattern, unsigned char pattern_length);
int bmh_count(bmh_search_data *bmh_sd);
int bmh_find(bmh_search_data *bmh_sd);

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
 * and sets idx to index of last matched character
 */
static inline size_t bmh_search(const bmh_table *table, const char *pattern, unsigned char pattern_length,
                                const char *data, size_t data_length, unsigned char start_idx, unsigned char *end_idx)
{
    if (data_length < pattern_length)
        return BMH_NOT_FOUND;

    size_t loc = 0;
    unsigned char j = 0;

    while (loc <= data_length - pattern_length)
    {
        j = start_idx;
        start_idx = 0;

        for (unsigned char i = 0; j < pattern_length; i++, j++)
        {
            if (data[loc + i] != pattern[j])
                break;
        }
        if (j == pattern_length)
        {
            *end_idx = 0;
            return loc;
        }

        loc += table[(unsigned char)data[loc + pattern_length - 1]];
    }

    *end_idx = j - 1;
    return BMH_NOT_FOUND;
}

/*
 * Returns location of pattern in given data, if not found returns BHM_NOT_FOUND,
 * and sets idx to index of last matched character,
 * ignores case, expects lowercase pattern
 */
static inline size_t bmh_search_i(const bmh_table *table, const char *pattern, unsigned char pattern_length,
                                  const char *data, size_t data_length, unsigned char start_idx, unsigned char *end_idx)
{
    if (data_length < pattern_length)
        return BMH_NOT_FOUND;

    size_t loc = 0;
    unsigned char j = 0;
    char c;

    while (loc <= data_length - pattern_length)
    {
        j = start_idx;
        start_idx = 0;

        for (unsigned char i = 0; j < pattern_length; i++, j++)
        {
            c = (char)tolower((unsigned char)data[loc + i]);
            if (c != pattern[j])
                break;
        }
        if (j == pattern_length)
        {
            *end_idx = 0;
            return loc;
        }

        c = (char)tolower((unsigned char)data[loc + pattern_length - 1]);
        loc += table[(unsigned char)c];
    }

    *end_idx = j - 1;
    return BMH_NOT_FOUND;
}

/*
 * Returns number of occurences of pattern in data, starts search from idx-th character in pattern
 * sets idx to last matched character index in pattern before end of data
 */
int bmh_count(bmh_search_data *bmh_sd)
{
    int found = 0;
    size_t data_loc = 0;

    while (data_loc < bmh_sd->data_length)
    {
        size_t loc;
        if ((loc = bmh_search(bmh_sd->table, bmh_sd->pattern, bmh_sd->pattern_length, bmh_sd->data + data_loc,
                              bmh_sd->data_length - data_loc, bmh_sd->idx, &(bmh_sd->idx))) == BMH_NOT_FOUND)
            return found;

        found++;
        data_loc += loc + bmh_sd->pattern_length;
    }

    return found;
}

/*
 * Returns 0 if data contains pattern, starts search from idx-th character in pattern
 * sets end_idx to last matched character index in pattern before end of data
 */
int bmh_find(bmh_search_data *bmh_sd)
{
    if (FLAG_SET(bmh_sd->flags, BMH_IGNORE_CASE))
        return bmh_search_i(bmh_sd->table, bmh_sd->pattern, bmh_sd->pattern_length, bmh_sd->data,
                            bmh_sd->data_length, bmh_sd->idx, &(bmh_sd->idx)) == BMH_NOT_FOUND
                   ? 1
                   : 0;
    else
        return bmh_search(bmh_sd->table, bmh_sd->pattern, bmh_sd->pattern_length, bmh_sd->data,
                          bmh_sd->data_length, bmh_sd->idx, &(bmh_sd->idx)) == BMH_NOT_FOUND
                   ? 1
                   : 0;
}
