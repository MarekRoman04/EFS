#ifndef EFS_SEARCH_H
#define EFS_SEARCH_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <efs_algo.h>
#include <arg_parser.h>
#include <file_stream.h>
#include <log.h>

#define RK_PATTERNS_MIN_SIZE 16

typedef struct pattern_set
{
    size_t count;
    const char **patterns;
    size_t *lengths;
} pattern_set;

typedef struct algo algo;

algo *algo_init(const pattern_set *patterns, unsigned int flags);
int algo_search(const algo *a, const line *l);
void algo_end(algo *a);

int search(cli_args *args);

// typedef struct rk_patterns
// {
//     char **patterns;
//     size_t *pattern_lengths;
//     size_t pattern_count;
//     size_t max_count;
// } rk_patterns;

// typedef struct rk_search_data
// {
//     char_buffer buffer;
//     file_stream *fs_searched;
//     line_stream *ls_searched;
//     rk_patterns patterns;
//     rk_search *rks;
//     int (*rk_search_function)(rk_search *, const char *, size_t, int);
//     // Cli arguments
//     unsigned int flags;
//     FILE *out_p;
// } rk_search_data;

// int rk_quiet_search(rk_search_data *rsd);
// int rk_list_search(rk_search_data *rsd);
// int rk_count_search(rk_search_data *rsd);
// int rk_line_number_search(rk_search_data *rsd);
// int rk_print_search(rk_search_data *rsd);

// int start_pattern_search(cli_args *args);
// int start_file_search(cli_args *args);

#endif
