#include "arg_parser.h"

#define PRINT_HELP printf("%s", HELP_MESSAGE)
#define ERROR_MISSING_ARGS log_error(MISSING_ARGS_MESSAGE)
#define INFO_INVALID_ARG(arg) log_info("Ignoring: %s, unknown option given!", arg)
#define ERROR_INVALID_ARG_VALUE(arg) log_error("Invalid %s, value given!", arg)
#define ERROR_MISSING_ARG_VALUE(arg) log_error("Missing %s", arg);

static inline void set_out_path(cli_args *args, const char *value);
static inline void set_buffer_size(cli_args *args, const char *value);
static inline void set_thread_count(cli_args *args, const char *value);
static inline int parse_option(cli_args *args, char *argv[]);
cli_args parse_args(int argc, char *argv[]);

const char *HELP_MESSAGE =
    "USAGE: efs [OPTIONS] [PATTERN] [FILES...]\n"
    "\n"
    "Options:\n"
    "  -c, --count           Print only a count of matching lines per file\n"
    "  -d, --directory       Search all files in directory\n"
    "  -f, --file            Search patterns from file (pattern = line in file)\n"
    "  -i, --ignore-case     Perform case-insensitive matching\n"
    "  -n, --line-number     Show line number for each match\n"
    "  -l, --list            List only names of matching files\n"
    "  -q, --quiet           Suppress normal output\n"
    "  -r, --recursive       Recursively search directories\n"
    "  -v, --invert-match    Select non-matching lines (ignored in combination with -q or -l)\n"
    "  -w, --word            Match only whole words\n"
    "      --output=FILE     Write output to FILE instead of standard output\n"
    "      --buffer-size=N   Set internal buffer size in bytes (default: 8192)\n"
    "      --thread-count=N  Set number of worker threads\n"
    "      --single-thread   Force single-threaded operation\n"
    "      --help            Display this help and exit\n";

const char *MISSING_ARGS_MESSAGE = "Few or no arguments given!, try --help for more info!";

const long_opt long_opts_map[] = {
    {"buffer-size", 11, set_buffer_size},
    {"output", 6, set_out_path},
    {"thread-count", 12, set_thread_count},
    NULL,
};

const long_flag long_flags_map[] = {
    {"count", FLAG_COUNT},
    {"directory", FLAG_DIRECTORY},
    {"file", FLAG_FILE},
    {"ignore-case", FLAG_IGNORE_CASE},
    {"line-number", FLAG_LINE_NUMBER},
    {"list", FLAG_LIST},
    {"quiet", FLAG_QUIET},
    {"recursive", FLAG_RECURSIVE},
    {"invert-match", FLAG_INVERT},
    {"word", FLAG_WORD},
    {"single-thread", FLAG_SINGLE_THREAD},
    {"", FLAGS_END},
    NULL,
};

static inline void set_out_path(cli_args *args, const char *value)
{
    args->out_path = value;
}

static inline void set_buffer_size(cli_args *args, const char *value)
{
    char *endptr = NULL;
    size_t size = strtoull(value, &endptr, 10);

    if (endptr == value || *endptr != '\0')
        ERROR_INVALID_ARG_VALUE("--buffer-size");
    else
        args->buffer_size = size;
}

static inline void set_thread_count(cli_args *args, const char *value)
{
    char *endptr = NULL;
    size_t count = strtoul(value, &endptr, 10);

    if (endptr == value || *endptr != '\0')
        ERROR_INVALID_ARG_VALUE("--buffer-size");
    else
        args->thread_count = (unsigned int)count;
}
/*
 * Sets cli_arg option based on argv, returns 1 if option consist of 2 argv items,
 * returns -1 on flags end
 */
static inline int parse_option(cli_args *args, char *argv[])
{
    const char *arg = argv[0] + 2;
    const char *equal = strchr(arg, '=');

    if (!equal)
    {
        for (int j = 0; long_flags_map[j].flag; j++)
        {
            if (!strcmp(arg, long_flags_map[j].flag))
            {
                args->flags |= long_flags_map[j].bit;
                return long_flags_map[j].bit ? 0 : -1;
            }
        }
    }

    for (int j = 0; long_opts_map[j].opt_name; j++)
    {
        const char *opt_name = long_opts_map[j].opt_name;
        int opt_len = long_opts_map[j].opt_name_length;

        if (equal && !strncmp(arg, opt_name, opt_len) && arg[opt_len] == '=')
        {
            long_opts_map[j].set_opt(args, equal + 1);
            return 0;
        }
        else if (!equal && !strcmp(arg, opt_name))
        {
            if (!argv[1])
            {
                ERROR_MISSING_ARG_VALUE(argv[0]);
                return 0;
            }
            long_opts_map[j].set_opt(args, argv[1]);
            return 1;
        }
    }

    INFO_INVALID_ARG(argv[0]);
    return 0;
}

cli_args parse_args(int argc, char *argv[])
{
    if (argc == 1)
    {
        ERROR_MISSING_ARGS;
        exit(EXIT_FAILURE);
    }
    if (!strcmp(argv[1], "--help"))
    {
        PRINT_HELP;
        exit(EXIT_SUCCESS);
    }
    if (argc < 3)
    {
        ERROR_MISSING_ARGS;
        exit(EXIT_FAILURE);
    }

    cli_args args = {.pattern = NULL,
                     .files = NULL,
                     .file_count = 0,
                     .flags = 0,
                     .buffer_size = 0,
                     .out_path = NULL,
                     .thread_count = 0};

    int flags_end = 0;

    for (int i = 1; i < argc; i++)
    {
        if (!flags_end && !strncmp(argv[i], "--", 2))
        {
            int ret_val = parse_option(&args, argv + i);
            if (ret_val == -1)
                flags_end = 1;
            else
                i += parse_option(&args, argv + i);
        }
        else if (!flags_end && argv[i][0] == '-')
        {
            for (int j = 1; j < strlen(argv[i]); j++)
            {
                switch (argv[i][j])
                {
                case 'c':
                    args.flags |= FLAG_COUNT;
                    break;
                case 'd':
                    args.flags |= FLAG_DIRECTORY;
                    break;
                case 'f':
                    args.flags |= FLAG_FILE;
                    break;
                case 'i':
                    args.flags |= FLAG_IGNORE_CASE;
                    break;
                case 'l':
                    args.flags |= FLAG_LIST;
                    break;
                case 'n':
                    args.flags |= FLAG_LINE_NUMBER;
                    break;
                case 'q':
                    args.flags |= FLAG_QUIET;
                    break;
                case 'r':
                    args.flags |= FLAG_RECURSIVE;
                    break;
                case 'v':
                    args.flags |= FLAG_INVERT;
                    break;
                case 'w':
                    args.flags |= FLAG_WORD;
                    break;

                default:
                    char flag[2] = {argv[i][j], '\0'};
                    INFO_INVALID_ARG(flag);
                    break;
                }
            }
        }
        else
        {
            if (!args.pattern)
            {
                argv[1] = argv[i];
                args.pattern = argv[1];
                args.files = argv + 2;
            }
            else
            {
                args.file_count++;
                argv[1 + args.file_count] = argv[i];
            }
        }
    }

    if (!args.pattern || !args.files)
        ERROR_MISSING_ARGS;

    return args;
}
