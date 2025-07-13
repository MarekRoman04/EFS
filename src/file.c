#include "file.h"

file_list list_files(cli_args *args);

// Returns valid file paths, count and total file size
file_list list_files(cli_args *args)
{
    struct stat file_st;
    file_list fl;

    fl.file_count = args->file_count;
    fl.file_paths = args->files;
    fl.total_size = 0;
    int i = 0;

    while (i < fl.file_count)
    {
        errno = 0;
        if (stat(args->files[i], &file_st) == -1)
        {
            log_errno(0, args->files[i]);
            // Swaps invalid files to the end of args->files
            argv_swap(args->files, i, fl.file_count - 1);
            fl.file_count--;
        }
        else if (!S_ISREG(file_st.st_mode))
        {
            log_errno(0, args->files[i]);
            // Swaps invalid files to the end of args->files
            argv_swap(args->files, i, fl.file_count - 1);
            fl.file_count--;
        }
        else
        {
            fl.total_size += file_st.st_size;
            i++;
        }
    }

    return fl;
}
