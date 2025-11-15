#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "algo.h"
#include "arg_parser.h"
#include "log.h"
#include "search.h"

#include <assert.h>

#ifndef DEBUG

int main(int argc, char *argv[])
{
    cli_args args = parse_args(argc, argv);

    if (FLAG_SET(args.flags, FLAG_FILE))
        return start_file_search(&args);
    else
        return start_pattern_search(&args);
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

    printf("-----------------\n");
    printf("----Searching----\n");
    printf("-----------------\n");

    if (FLAG_SET(args.flags, FLAG_FILE))
        return start_file_search(&args);
    else
        return start_pattern_search(&args);

    return 0;
}

// RK Search tests;
// int main()
// {
//     const char *data[] = {"aa", "ab", "abc", "abcd", "ad", "bac", "cab", "bacd", "cadb", "adbc"};
//     size_t data_lengths[] = {3, 3, 4, 5, 3, 4, 4, 5, 5, 5};
//     char data2[] = {"abcdefgh"};
//     const char *entry;
//     size_t entry_size;

//     rk_data rkd = {
//         .patterns = data,
//         .pattern_lengths = data_lengths,
//         .count = 10,
//         .data = data2,
//         .data_length = 9,
//     };

//     rk_search *rk = rk_search_init(&rkd);
//     assert(rk != NULL);

//     h_set_iterator *hsi = h_set_iterator_init(rk->patterns);
//     while ((entry = h_set_iterator_get(hsi, &entry_size)))
//     {
//         printf("Pattern: %s, %lu\n", entry, entry_size);
//     }

//     while ((entry = h_set_iterator_get(rk->patterns_hashes_i, &entry_size)))
//     {
//         uint64_t *hash = (uint64_t *)entry;
//         printf("Pattern hash: %lu\n", *hash);
//     }

//     for (size_t i = 0; i < rk->data_hashes_length; i++)
//     {
//         rk_data_hash rkd = rk->data_hashes[i];
//         printf("Hash: %lu, %lu, %lu\n", rkd.data_hash, rkd.mod_power, rkd.data_length);
//     }

//     rk_search_end(rk);
//     h_set_iterator_end(hsi);

//     return 0;
// }

#endif