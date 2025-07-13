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
    cli_args args = parse_args(argc, argv);

    switch (args.flags)
    {
    case FLAG_RECURSIVE:
        // return start_rec_search(args);
        break;
    case FLAG_DIRECTORY:
        // return start_dir_search(args);
        break;
    default:
        return start_file_search(&args);
    }
}

#else

int main(int argc, char *argv[])
{
    printf("-----------------\n");
    printf("---Arg parsing---\n");
    printf("-----------------\n");

    cli_args args = parse_args(argc, argv);

    printf("Pattern: %s\n", args.pattern);
    printf("Files: %d\n", args.file_count);
    for (int i = 0; i < args.file_count; i++)
    {
        printf("%s\n", args.files[i]);
    }
    printf("Flags: %x\n", args.flags);
    printf("Buffer: %ld\n", args.buffer_size);
    printf("Out path: %s\n", args.out_path);
    printf("Threads: %d\n", args.thread_count);

    switch (args.flags)
    {
    case FLAG_RECURSIVE:
        // return start_rec_search(args);
        break;
    case FLAG_DIRECTORY:
        // return start_dir_search(args);
        break;
    default:
        return start_file_search(&args);
    }
}

#endif