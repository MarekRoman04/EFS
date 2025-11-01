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
#define DEFAULT_OUT_PATH stdout;

typedef struct char_buffer
{
    char *data;
    size_t size;
} char_buffer;

typedef struct search_data
{
    file_stream *fs_searched;
    line_stream *ls_searched;
    char *pattern;
    size_t pattern_length;
    uint8_t *bad_char_table;
    uint8_t *good_suffix_table;
    char_buffer buffer;
    int (*bmh_search)(bm_data*);
    // Cli arguments
    unsigned int flags;
    FILE *out_p;
} search_data;


int quiet_search(search_data *sd);
int list_search(search_data *sd);
int count_search(search_data *sd);
int line_number_search(search_data *sd);
int print_search(search_data *sd);
int start_file_search(cli_args *args);
int start_rec_search(cli_args *args);

#endif