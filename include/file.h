#ifndef FILE_H
#define FILE_H

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "arg_parser.h"
#include "log.h"

#define MAX_PATH 256

typedef struct file_list
{
    int file_count;
    char **file_paths;
    size_t total_size;
} file_list;

typedef struct dir_map
{
    int dir_count;
    char **dir_paths;
    int file_count;
    size_t total_file_size;
} dir_map;

typedef struct rec_map
{
    int file_count;
    int total_size;
} rec_dir_map;

file_list list_files(cli_args *args);

#endif