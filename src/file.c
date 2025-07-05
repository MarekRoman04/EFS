#include "data.h"
#include "file.h"

file_map map_direct_files(cli_args *args)
{
    struct stat file_st;
    file_map fm;

    fm.file_count = args->file_count;
    fm.file_paths = args->files;
    fm.total_size = 0;
    int i = 0;

    while (i < fm.file_count)
    {
        errno = 0;
        if (stat(args->files[i], &file_st) == -1)
        {
            log_errno(0, args->files[i]);
            // Swaps invalid files to the end of args->files
            argv_swap(args->files, i, fm.file_count - 1);
            fm.file_count--;
        }
        else if (!S_ISREG(file_st.st_mode))
        {
            log_errno(0, args->files[i]);
            // Swaps invalid files to the end of args->files
            argv_swap(args->files, i, fm.file_count - 1);
            fm.file_count--;
        }
        else
        {
            fm.total_size += file_st.st_size;
            i++;
        }
    }

    return fm;
}

dir_map map_direct_directories(cli_args *args)
{
    dir_map dm;
    dm.dir_count = args->file_count;
    dm.dir_paths = args->files;
    dm.file_count = 0;
    dm.total_file_size = 0;
    int i = 0;

    while (i < dm.dir_count)
    {
        // Removes trailling /
        int path_len = strlen(dm.dir_paths[i]);
        if (dm.dir_paths[i][path_len - 1] == '/')
            dm.dir_paths[i][path_len - 1] = '\0';

        DIR *dir = opendir(dm.dir_paths[i]);

        if (dir == NULL)
        {
            log_errno(0, dm.dir_paths[i]);
            argv_swap(args->files, i, dm.dir_count - 1);
            dm.dir_count--;
        }
        else
        {
            int dir_file_count = 0;
            struct dirent *dirp;
            struct stat st;

            while ((dirp = readdir(dir)) != NULL)
            {
                char f_path[MAX_PATH];
                snprintf(f_path, MAX_PATH, "%s/%s", dm.dir_paths[i], dirp->d_name);

                if (!stat(f_path, &st) && S_ISREG(st.st_mode))
                {
                    dir_file_count++;
                    dm.total_file_size += st.st_size;
                }
            }

            if (!dir_file_count)
            {
                log_info("Ignoring: %s, no files in directory!\n", dm.dir_paths[i]);
                argv_swap(args->files, i, dm.dir_count - 1);
                dm.dir_count--;
            }
            else
            {
                dm.file_count += dir_file_count;
                i++;
            }

            closedir(dir);
        }
    }

    return dm;
}

rec_dir_map map_recursive(cli_args *args)
{
}

