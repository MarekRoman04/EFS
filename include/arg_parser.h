#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdlib.h>
#include <string.h>
#include "log.h"

#define FLAGS_END 0
#define FLAG_COUNT (1 << 0)          // -c or --count flag
#define FLAG_DIRECTORY (1 << 1)      // -d or --directory flag
#define FLAG_FILE (1 << 2)           // -f or --file flag
#define FLAG_IGNORE_CASE (1 << 3)    // -i or --ignore-case flag
#define FLAG_LINE_NUMBER (1 << 4)    // -n or --line_number flag
#define FLAG_LIST (1 << 5)           // -l or --list
#define FLAG_QUIET (1 << 6)          // -q or --quiet flag
#define FLAG_RECURSIVE (1 << 7)      // -r or --recursive flag
#define FLAG_INVERT (1 << 8)         // -v or --invert-match flag
#define FLAG_WORD (1 << 9)           // -w or --word flag
#define FLAG_SINGLE_THREAD (1 << 10) // --signle-thread flag

#define FLAG_EMPTY 0
#define FLAG_SET(flags, bit) (!!(flags & bit))

typedef struct arg_parser
{
    const char *pattern;
    char **files;
    int file_count;
    // optional arguments
    unsigned int flags;
    size_t buffer_size;        // --buffer-size
    const char *out_path;      // -o or --output
    unsigned int thread_count; // --thread-count or --single-thread
} cli_args;

typedef struct long_opt
{
    const char *opt_name;
    int opt_name_length;
    void (*set_opt)(cli_args *args, const char *value);
} long_opt;

typedef struct long_flag
{
    const char *flag;
    unsigned int bit;
} long_flag;

extern const long_flag long_flags_map[];
extern const long_opt long_opts_map[];

static inline void argv_swap(char *argv[], int from, int to)
{
    char *temp = argv[to];
    argv[to] = argv[from];
    argv[from] = temp;
}

cli_args parse_args(int argc, char *argv[]);

#endif