#ifndef ALGO_H
#define ALGO_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define BMH_NOT_FOUND ((size_t)-1)

typedef unsigned char bmh_table;

typedef struct bmh_stream
{
    const bmh_table *table;
    const char *pattern;
    unsigned char pattern_length;
    const char *data;
    size_t data_length;
    unsigned char end_idx;
} bmh_stream;

bmh_table *bmh_pre_process(const char *pattern, unsigned char pattern_length);
int bmh_count(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
              size_t data_length, unsigned char start_idx, unsigned char *end_idx);
int bmh_find(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data,
             size_t data_length, unsigned char start_idx, unsigned char *end_idx);

bmh_stream *bmhs_init(const bmh_table *table, const char *pattern, unsigned char pattern_length, const char *data, size_t data_length);
size_t bmhs_loc(bmh_stream *bmhs);
void bmhs_end(bmh_stream *bmhs);

// Sets new data to search in
static inline void bmhs_new_data(bmh_stream *bmhs, const char *data, size_t data_length)
{
    bmhs->data = data;
    bmhs->data_length = data_length;
    bmhs->end_idx = 0;
}

// Adds data to search in, resumes search with last matched character
static inline void bmhs_add_data(bmh_stream *bmhs, const char *data, size_t data_length)
{
    bmhs->data = data;
    bmhs->data_length = data_length;
}

#endif