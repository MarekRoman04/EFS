#ifndef SEARCH_H
#define SEARCH_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "algo.h"
#include "arg_parser.h"
#include "file_stream.h"
#include "log.h"

#define DEFAULT_BUFFER_SIZE 16384 // 16KB
#define MIN_BUFFER_SIZE 4096      // 4KB
#define RK_PATTERNS_MIN_SIZE 16
#define DEFAULT_OUT_PATH stdout;

typedef struct char_buffer
{
    char *data;
    size_t size;
} char_buffer;

typedef struct bm_search_data
{
    file_stream *fs_searched;
    line_stream *ls_searched;
    char *pattern;
    size_t pattern_length;
    uint8_t *bad_char_table;
    uint8_t *good_suffix_table;
    char_buffer buffer;
    int (*bmh_search)(bm_data *);
    // Cli arguments
    unsigned int flags;
    FILE *out_p;
} bm_search_data;

typedef struct rk_patterns
{
    char **patterns;
    size_t *pattern_lengths;
    size_t pattern_count;
    size_t max_count;
} rk_patterns;

typedef struct rk_search_data
{
    char_buffer buffer;
    file_stream *fs_searched;
    line_stream *ls_searched;
    rk_patterns patterns;
    rk_search *rks;
    int (*rk_search_function)(rk_search *, const char *, size_t);
    // Cli arguments
    unsigned int flags;
    FILE *out_p;
} rk_search_data;

int bm_quiet_search(bm_search_data *sd);
int bm_list_search(bm_search_data *sd);
int bm_count_search(bm_search_data *sd);
int bm_line_number_search(bm_search_data *sd);
int bm_print_search(bm_search_data *sd);

int rk_quiet_search(rk_search_data *rsd);
int rk_list_search(rk_search_data *rsd);
int rk_count_search(rk_search_data *rsd);
int rk_line_number_search(rk_search_data *rsd);
int rk_print_search(rk_search_data *rsd);

int start_pattern_search(cli_args *args);
int start_file_search(cli_args *args);

#endif