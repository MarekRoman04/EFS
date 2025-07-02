#include "file.h"

size_t map_files(cli_args *args)
{
    struct stat file_st;
    size_t file_size = 0;
    int i = 0;

    while (i < args->file_count)
    {
        errno = 0;
        if (stat(args->files[i], &file_st) == -1)
        {
            switch (errno)
            {
            case ENOENT:
                log_info("Ignoring: File %s, file does not exist!\n", args->files[i]);
                break;

            case EACCES:
                log_info("Ignoring: File %s, missing file permission!\n", args->files[i]);
                break;

            default:
                log_info("Ignoring: File %s, error (%s)\n", args->files[i], strerror(errno));
                break;
            }

            // Swaps invalid files to the end of args->files
            argv_swap(args->files, i, args->file_count - 1);
            args->file_count--;
        }
        else if (!S_ISREG(file_st.st_mode))
        {
            log_info("Ignoring: File %s, not a regular file!\n", args->files[i]);
            argv_swap(args->files, i, args->file_count - 1);
            args->file_count--;
        }
        else
        {
            file_size += file_st.st_size;
            i++;
        }
    }

    return file_size;
}