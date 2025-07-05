#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdlib.h>
#include <string.h>
#include "log.h"

#define FLAG_COUNT (1 << 0)       // -c or --count flag
#define FLAG_DIRECTORY (1 << 1)   // -d or --directory flag
#define FLAG_FILE (1 << 2)        // -f or --file flag
#define FLAG_IGNORE_CASE (1 << 3) // -i or --ignore-case flag
#define FLAG_LINE_NUMBER (1 << 4) // -n or --line_number flag
#define FLAG_LIST (1 << 5)        // -l or --list
#define FLAG_QUIET (1 << 6)       // -q or --quiet flag
#define FLAG_RECURSIVE (1 << 7)   // -r or --recursive flag
#define FLAG_INVERT (1 << 8)      // -v or --invert-match flag
#define FLAG_WORD (1 << 9)        // -w or --word flag

#define FLAG_SET(flags, bit) (flags & bit)

typedef struct arg_parser
{
    int flags;
    const char *pattern;
    char **files;
    int file_count;
} cli_args;

typedef struct long_option_map
{
    const char *opt;
    int bit;
} long_option_map;

extern const long_option_map long_opts[];

static inline void argv_swap(char *argv[], int from, int to)
{
    char *temp = argv[to];
    argv[to] = argv[from];
    argv[from] = temp;
}

int parse_args(int argc, char *argv[], cli_args *args);

#endif