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

typedef struct file_map
{
    int file_count;
    char **file_paths;
    size_t total_size;
} file_map;

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

file_map map_direct_files(cli_args *args);
dir_map map_direct_directories(cli_args *args);

static inline int read_file_chunk(const search_buffer *buffer, size_t offset)
{
    fseek(buffer->fp, -offset, SEEK_CUR);
    size_t read = fread(buffer->buffer, buffer->buffer_size, buffer->buffer_size - 1, buffer->fp);
    buffer->buffer[read] = '\0';
    return read;
}

#endif