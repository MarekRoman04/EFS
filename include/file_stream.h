#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <stdio.h>

typedef struct file_stream
{
    int file_count;
    char **file_paths;
    char **current_file;
    FILE *current_fp;
} file_stream;

file_stream *fs_init(char **f_paths, int f_count);
int fs_open_file(file_stream *fs, const char *mode);
size_t fs_read_file(file_stream *fs, char *buffer, size_t buffer_size);
int fs_close_file(file_stream *fs);
int fs_end(file_stream *fs);

// Check if there is next file in stream
static inline int fs_has_file(file_stream *fs)
{
    return fs->current_file + 1 < fs->file_paths + fs->file_count;
}

// Resest current file pointer to the beginning of the stream
static inline void fs_reset(file_stream *fs)
{
    fs->current_file = fs->file_paths;
}

// Moves current file pointer to next file
static inline void fs_skip_file(file_stream *fs)
{
    if (!fs->current_file)
        return;

    if (fs_has_file(fs))
        fs->current_file++;
    else
        fs->current_file = NULL;
}

#endif