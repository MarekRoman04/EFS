#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "algo.h"
#include "arg_parser.h"
#include "file.h"
#include "log.h"
#include "search.h"

#ifndef DEBUG

int main(int argc, char *argv[])
{
    cli_args args = {.flags = 0};
    int arg_count = parse_args(argc, argv, &args);
    int fc = args.file_count;

    if (FLAG_SET(args.flags, FLAG_DIRECTORY))
    {
        dir_map dm = map_direct_directories(&args);
    }
    else
    {
        file_map fm = map_direct_files(&args);
    }

    process_data(&args);

    return FLAG_SET(args.flags, FLAG_QUIET) ? 1 : 0;
}

#else

int main(int argc, char *argv[])
{
    printf("-----------------\n");
    printf("---Arg parsing---\n");
    printf("-----------------\n");

    cli_args args = {.flags = 0};
    int arg_count = parse_args(argc, argv, &args);
    int fc = args.file_count;

    printf("Valid args count: %d\n", arg_count);
    printf("Flags: %x\n", args.flags);
    printf("Pattern: %s\n", args.pattern);
    printf("Files found: %d\n", args.file_count);
    for (size_t i = 0; i < args.file_count; i++)
    {
        printf("%s\n", args.files[i]);
    }

    if (FLAG_SET(args.flags, FLAG_DIRECTORY))
    {
        printf("-----------------\n");
        printf("---Dir mapping---\n");
        printf("-----------------\n");
        dir_map dm = map_direct_directories(&args);
        printf("Directories found: %d\n", dm.dir_count);
        for (size_t i = 0; i < dm.dir_count; i++)
        {
            printf("%s\n", dm.dir_paths[i]);
        }
        printf("File count: %d\n", dm.file_count);
        printf("Total size: %ld\n", dm.total_file_size);
        printf("Directories full: %d\n", args.file_count);
        for (size_t i = 0; i < fc; i++)
        {
            printf("%s\n", args.files[i]);
        }
    }
    else
    {
        printf("-----------------\n");
        printf("--File mapping---\n");
        printf("-----------------\n");
        file_map fm = map_direct_files(&args);
        printf("Files found: %d\n", fm.file_count);
        for (size_t i = 0; i < fm.file_count; i++)
        {
            printf("%s\n", fm.file_paths[i]);
        }
        printf("Total size: %ld\n", fm.total_size);

        printf("Files full: %d\n", args.file_count);
        for (size_t i = 0; i < fc; i++)
        {
            printf("%s\n", args.files[i]);
        }
    }

    printf("-----------------\n");
    printf("----Searching----\n");
    printf("-----------------\n");
    process_data(&args);

    return FLAG_SET(args.flags, FLAG_QUIET) ? 1 : 0;
}

#endif