#ifndef ALGO_H
#define ALGO_H

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "log.h"

#define NOT_FOUND ((size_t)-1)

typedef struct bm_data
{
    const char *pattern;
    uint8_t pattern_length;
    uint8_t *bad_char_table;
    uint8_t *good_suffix_table;
    char *data;
    size_t data_length;
    _Bool ignore_case;
    uint8_t idx;
} bm_data;

/*
 *Computes bad char table for given pattern,
 *required in bm and bmh search functions
 */
uint8_t *bm_bad_char_table(const char *pattern, uint8_t pattern_length);
/*
 *Computes good suffix table for given pattern,
 *required in bm search functions
 */
uint8_t *bm_good_suffix_table(const char *pattern, uint8_t pattern_length);
/*
 * Uses the Boyer-Moore string search algorith if both tables are set
 * otherwise uses the Boyer–Moore–Horspool string search algorithm
 * Returns number of occurences of pattern in data, starts search from idx-th character in pattern
 * sets idx to last matched character index in pattern before end of data
 */
int bm_count(bm_data *bmd);
/*
 * Uses the Boyer-Moore string search algorith if both tables are set
 * otherwise uses the Boyer–Moore–Horspool string search algorithm
 * Returns 0 if data contains pattern, starts search from idx-th character in pattern
 * sets end_idx to last matched character index in pattern before end of data
 */
int bm_find(bm_data *bmd);
/*
 * Uses the Boyer-Moore string search algorith if both tables are set
 * otherwise uses the Boyer–Moore–Horspool string search algorithm
 * Returns 0 if data contains word, starts search from idx-th character in word
 * sets end_idx to last matched character index in word before end of data
 */
int bm_find_w(bm_data *bmd);

#endif