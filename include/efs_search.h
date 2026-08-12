#ifndef EFS_SEARCH_H
#define EFS_SEARCH_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <arg_parser.h>
#include <file_stream.h>
#include <hash_set.h>
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

#endif
