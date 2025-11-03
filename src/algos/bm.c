#include "algo.h"

static inline size_t bmh_search(const uint8_t *table, const uint8_t *_table, const char *pattern, uint8_t pattern_length,
                                const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx);
static inline size_t bmh_search_i(const uint8_t *table, const uint8_t *_table, const char *pattern, uint8_t pattern_length,
                                  const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx);
static inline size_t bm_search(const uint8_t *bad_table, const uint8_t *good_table, const char *pattern, uint8_t pattern_length,
                               const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx);
static inline size_t bm_search_i(const uint8_t *bad_table, const uint8_t *good_table, const char *pattern, uint8_t pattern_length,
                                 const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx);
static inline int is_word_char(char c);

typedef size_t (*bm_search_function)(const uint8_t *, const uint8_t *, const char *, uint8_t, const char *, size_t, uint8_t, uint8_t *);
bm_search_function bm_functions[2][2] = {
    {bmh_search, bmh_search_i},
    {bm_search, bm_search_i}};

uint8_t *bm_bad_char_table(const char *pattern, uint8_t pattern_length);
uint8_t *bm_good_suffix_table(const char *pattern, uint8_t pattern_length);
int bm_count(bm_data *bmd);
int bm_find(bm_data *bmd);
int bm_find_w(bm_data *bmd);

/*
 * Returns location of pattern in given data, if not found returns BHM_NOT_FOUND,
 * and sets idx to index of last matched character
 */
static inline size_t bmh_search(const uint8_t *table, const uint8_t *_table, const char *pattern, uint8_t pattern_length,
                                const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx)
{
    (void)_table; // Used only in BM functions;

    if (data_length < pattern_length)
        return NOT_FOUND;

    size_t loc = 0;
    int j = 0;

    while (loc <= data_length - pattern_length)
    {
        j = start_idx;
        start_idx = 0;

        for (int i = 0; j < pattern_length; i++, j++)
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
    return NOT_FOUND;
}

/*
 * Returns location of pattern in given data, if not found returns BHM_NOT_FOUND,
 * and sets idx to index of last matched character,
 * ignores case, expects lowercase pattern
 */
static inline size_t bmh_search_i(const uint8_t *table, const uint8_t *_table, const char *pattern, uint8_t pattern_length,
                                  const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx)
{
    (void)_table; // Used only in BM functions;

    if (data_length < pattern_length)
        return NOT_FOUND;

    size_t loc = 0;
    int j = 0;
    char c;

    while (loc <= data_length - pattern_length)
    {
        j = start_idx;
        start_idx = 0;

        for (int i = 0; j < pattern_length; i++, j++)
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
    return NOT_FOUND;
}

static inline size_t bm_search(const uint8_t *bad_table, const uint8_t *good_table, const char *pattern, uint8_t pattern_length,
                               const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx)
{
    log_error("TO-DO!");
}

static inline size_t bm_search_i(const uint8_t *bad_table, const uint8_t *good_table, const char *pattern, uint8_t pattern_length,
                                 const char *data, size_t data_length, uint8_t start_idx, uint8_t *end_idx)
{
    log_error("TO-DO!");
}

static inline int is_word_char(char c)
{
    return isalnum((unsigned char)c) || c == '_' || c == '-';
}

/*
 *Computes bad char table for given pattern,
 *required in bm and bmh search functions
 */
uint8_t *bm_bad_char_table(const char *pattern, uint8_t pattern_length)
{
    uint8_t *table = malloc(256);
    if (!table)
        return NULL;

    for (int i = 0; i < 256; i++)
        table[i] = pattern_length;

    for (int i = 0; i < pattern_length - 1; i++)
        table[(unsigned char)pattern[i]] = pattern_length - 1 - i;

    return table;
}

/*
 *Computes good suffix table for given pattern,
 *required in bm search functions
 */
uint8_t *bm_good_suffix_table(const char *pattern, uint8_t pattern_length)
{
    log_error("TO-DO!");
}

int bm_count(bm_data *bmd)
{
    bm_search_function search = bm_functions[bmd->good_suffix_table != NULL][bmd->ignore_case];
    int found = 0;
    size_t data_loc = 0;

    while (data_loc < bmd->data_length)
    {
        size_t loc;
        if ((loc = search(bmd->bad_char_table, bmd->good_suffix_table, bmd->pattern, bmd->pattern_length,
                          bmd->data + data_loc, bmd->data_length - data_loc, bmd->idx, &(bmd->idx))) == NOT_FOUND)
            return found;

        found++;
        data_loc += loc + bmd->pattern_length;
    }

    return found;
}

int bm_find(bm_data *bmd)
{
    bm_search_function search = bm_functions[(_Bool)(bmd->good_suffix_table != NULL)][bmd->ignore_case];
    return search(bmd->bad_char_table, bmd->good_suffix_table, bmd->pattern, bmd->pattern_length,
                  bmd->data, bmd->data_length, bmd->idx, &(bmd->idx)) == NOT_FOUND
               ? 1
               : 0;
}

int bm_find_w(bm_data *bmd)
{
    bm_search_function search = bm_functions[(_Bool)(bmd->good_suffix_table != NULL)][bmd->ignore_case];
    size_t data_loc = 0;
    size_t bmh_result;

    while (data_loc < bmd->data_length)
    {
        bmh_result = search(bmd->bad_char_table, bmd->good_suffix_table, bmd->pattern, bmd->pattern_length,
                            bmd->data + data_loc, bmd->data_length - data_loc, bmd->idx, &(bmd->idx));

        if (bmh_result == NOT_FOUND)
            return 1;

        size_t loc = data_loc + bmh_result;
        if ((!loc || !is_word_char(bmd->data[loc - 1])) &&
            (loc + bmd->pattern_length >= bmd->data_length || !is_word_char(bmd->data[loc + bmd->pattern_length])))
            return 0;

        data_loc += bmh_result + bmd->pattern_length;
    }

    return 1;
}
