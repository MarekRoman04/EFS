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

    if (FLAG_SET(args.flags, FLAG_RECURSIVE))
        ;
    // return start_rec_search(args);
    else
        return start_file_search(&args);
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

    if (FLAG_SET(args.flags, FLAG_RECURSIVE))
        ;
    // return start_rec_search(args);
    else
        return start_file_search(&args);

    return 0;
}

#endif