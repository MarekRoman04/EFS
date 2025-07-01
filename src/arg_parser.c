#include "arg_parser.h"

const long_option_map long_opts[] = {
    {"count", FLAG_COUNT},
    {"directory", FLAG_DIRECTORY},
    {"file", FLAG_FILE},
    {"ignore-case", FLAG_IGNORE_CASE},
    {"line-number", FLAG_LINE_NUMBER},
    {"recursive", FLAG_RECURSIVE},
    {"invert-match", FLAG_INVERT},
    {"word", FLAG_WORD},
    {NULL, 0},
};

static inline void argv_swap(char *argv[], int from, int to)
{
    char *temp = argv[to];
    argv[to] = argv[from];
    argv[from] = temp;
}

/*
Parses input args int cli_args struct,
first non flag arg is parsed as pattern,
other args are parsed as search locations,
returns number of valid arguments
*/

int parse_args(int argc, char *argv[], cli_args *args)
{
    // Orders args, first --second -last, idx points one past arg type
    int invalid = 0;
    int arg_idx, long_opt_idx;

    arg_idx = 1;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            if (arg_idx != i)
                argv_swap(argv, i, arg_idx);
            arg_idx++;
        }
    }

    long_opt_idx = arg_idx;
    for (int i = arg_idx; i < argc; i++)
    {
        if (!strncmp(argv[i], "--", 2))
        {
            if (long_opt_idx != i)
                argv_swap(argv, i, long_opt_idx);
            long_opt_idx++;
        }
    }

    // Parses --long-args flags
    for (int i = arg_idx; i < long_opt_idx; i++)
    {
        int found = 0;
        for (int j = 0; long_opts[j].opt != NULL; j++)
        {
            if (!strcmp(argv[i] + 2, long_opts[j].opt))
            {
                args->flags |= long_opts[j].bit;
                found = 1;
                break;
            }
        }

        if (!found)
        {
            invalid++;
            // TODO: LOG_ERROR
        }
    }

    // Parses -x flags
    for (int i = long_opt_idx; i < argc; i++)
    {
        for (int j = 1; j < strlen(argv[i]); j++)
        {
            switch (argv[i][j])
            {
            case 'c':
                args->flags |= FLAG_COUNT;
                break;
            case 'd':
                args->flags |= FLAG_DIRECTORY;
                break;
            case 'f':
                args->flags |= FLAG_FILE;
                break;
            case 'i':
                args->flags |= FLAG_IGNORE_CASE;
                break;
            case 'n':
                args->flags |= FLAG_LINE_NUMBER;
                break;
            case 'q':
                args->flags |= FLAG_QUIET;
                break;
            case 'r':
                args->flags |= FLAG_RECURSIVE;
                break;
            case 'v':
                args->flags |= FLAG_INVERT;
                break;
            case 'w':
                args->flags |= FLAG_WORD;
                break;

            default:
                invalid++;
                // TODO: LOG_ERROR
                break;
            }
        }
    }

    args->pattern = argv[1];
    args->files = argv + 2;
    args->file_count = arg_idx - 2;

    return argc - invalid;
}
