#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "algo.h"

int main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "+cw")) != -1)
    {
        switch (opt)
        {
        case 'c':
            int c = string_search_count(argv[optind], argv[optind + 1]);
            printf("%d\n", c);
            break;
        case 'w':
            LOG_ERROR("Missing implementation!");
            break;
        default:
            LOG_INFO("Ignoring invalid option!");
            break;
        }
    }

    return 0;
}
