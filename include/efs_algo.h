#ifndef EFS_ALGO_H
#define EFS_ALGO_H

#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <log.h>

#define NOT_FOUND ((size_t)SIZE_MAX)

static inline int is_word_char(char c) { return isalnum((unsigned char)c) || c == '_'; }

//---------------------------------
//----HASH SET DEFITIONS-----------
//---------------------------------

typedef struct h_set h_set;
typedef struct h_set_iterator h_set_iterator;

/*
 * Inits empty hash set
 */
h_set *h_set_init(int capacity);
/*
 * Check if hash set contains given data
 * if set contains data returns 0 otherwise -1
 */
int h_set_has(h_set *hs, const char *data, size_t data_length);
/*
 * Adds element to hash set,
 * if entries are 2/3 full set is automatically resized
 * if set already contains entry returns 1, set remains unchanged
 * on error return -1
 */
int h_set_add(h_set *hs, const char *data, size_t data_length);
/*
TODO
*/
char **h_set_move_end(h_set *hs, size_t **dest_lengths, size_t *count);
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
void h_set_iterator_reset(h_set_iterator *hsi);
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

//---------------------------------
//----RK SEARCH DEFINITIONS--------
//---------------------------------

#define RK_BASE 256ULL
#define RK_MOD 1000000007ULL

typedef struct rk_data rk_data;
typedef struct rk_search rk_search;
typedef struct rk_data_hash rk_data_hash;

struct rk_data
{
    const char **patterns;
    size_t *pattern_lengths;
    size_t count;
    char *data;
    size_t data_length;
};

struct rk_search
{
    h_set *patterns;
    h_set *patterns_hashes;
    h_set_iterator *patterns_hashes_i;
    rk_data_hash *data_hashes;
    size_t data_hashes_length;
};

struct rk_data_hash
{
    uint64_t data_hash;
    uint64_t mod_power;
    size_t data_length;
};

/*
 * Initializes hash sets and computes hashes for given patterns, required in rk search functions
 * patterns longer than data length have reserved memory for data hashes
 */
rk_search *rk_search_init(rk_data *rkd);
/*
 * Returns number of occurences of patterns in data,
 * patterns starting at same position are counted only once
 */
int rk_count(rk_search *rks, const char *data, size_t data_length, int ignore_case);
/*
 * Returns 0 if data contains any pattern from given set
 */
int rk_find(rk_search *rks, const char *data, size_t data_length, int ignore_case);
/*
 * Returns 0 if data contains any word from given set
 */
int rk_find_w(rk_search *rks, const char *data, size_t data_length, int ignore_case);
/*
 * Generates new rolling hashes for given data,
 * hashes longer than data are not changed,
 * expects patterns to remain the same
 */
void rk_rehash_data(rk_search *rks, const char *data, size_t data_length);
/*
 * Frees memory used by rk search
 */
void rk_search_end(rk_search *rk);

#endif
