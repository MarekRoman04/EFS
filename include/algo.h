#ifndef ALGO_H
#define ALGO_H

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "log.h"

#define NOT_FOUND ((size_t)-1)

//---------------------------------
//----BM SEARCH DEFINITIONS--------
//---------------------------------

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

//---------------------------------
//----RP SEARCH DEFINITIONS--------
//---------------------------------

#define RP_HASH_BASE 256
#define RP_HASH_PRIME 2

typedef struct rp_data
{
    const char **patterns;
    size_t *pattern_lengths;
    char *data;
    size_t data_length;
    _Bool ignore_case;
} rp_data;

//---------------------------------
//----HASH SET DEFITIONS-----------
//---------------------------------

#define TABLE_MIN_BUCKETS 16
#define FNV_OFFSET_BASIS 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct h_set h_set;
typedef struct h_set_entry h_set_entry;
typedef struct h_set_iterator h_set_iterator;

struct h_set
{
    h_set_entry *entries;
    int capacity;
    int length;
};

struct h_set_entry
{
    const char *data;
    size_t data_length;
    h_set_entry *next;
};

struct h_set_iterator
{
    h_set *hs;
    int idx;
    h_set_entry *entry;
};

/*
 * FNV-1a hash function
 */
uint64_t fnv_1a_hash(const char *data, size_t data_length);
/*
 * Inits empty hash set
 */
h_set *h_set_init();
/*
 * Check if hash set contains given data
 * if set contains data returns 0 otherwise -1
 */
int h_set_has(h_set *hs, const char *data, size_t data_length);
/*
 * Adds element to hash set,
 * if entries are 2/3 full set is automatically resized
 * if set already contains entry returns 1, set remains unchanged
 */
int h_set_add(h_set *hs, const char *data, size_t data_length);
/*
 * Frees all memory used by hash set
 */
void h_set_end(h_set *hs);
/*
 * Inits hash set iterator,
 * Iterator needs to be reset after adding elements
 * otherwise some elements may be skipped
 */
h_set_iterator *h_set_iterator_init(h_set *hs);
/*
 * Resets position of hash set iterator
 */
static inline void h_set_iterator_reset(h_set_iterator *hsi)
{
    hsi->idx = 0;
    hsi->entry = &hsi->hs->entries[0];
}
/*
 * Gets data from hash set,
 * sets out_length to size of data returned,
 * returns NULL if no data remain, out_length remains unchanged
 */
const char *h_set_iterator_get(h_set_iterator *hsi, size_t *out_length);
/*
 * Ends hash set iterator and frees memory used,
 */
void h_set_iterator_end(h_set_iterator *hsi);

#endif