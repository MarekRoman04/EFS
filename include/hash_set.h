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
typedef struct h_set_iterator
{
    h_set *hs;
    int idx;
} h_set_iterator;

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
int h_set_insert(h_set *hs, const char *data, size_t data_length);
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
void h_set_iterator_init(h_set_iterator *hsi, h_set *hs);
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

#endif
