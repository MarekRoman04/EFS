#ifndef SEARCH_H
#define SEARCH_H

#include <stdio.h>
#include <stdlib.h>

#include "algo.h"
#include "arg_parser.h"
#include "file.h"
#include "log.h"

#define DEFAULT_BUFFER_SIZE 16384
#define MIN_BUFFER_SIZE 4096
#define DEFAULT_OUT_PATH stdout;

typedef struct search_data
{
    const char *pattern;
    size_t pattern_length;
    bmh_table *table;
    char *buffer;
    size_t buffer_size;
    // Cli arguments
    int flags;
    FILE* out_p;
} search_data;

int start_file_search(cli_args *args);

#endif