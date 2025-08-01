#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <stdio.h>
#include "log.h"

//---------------------------------
//----FILE STREAM DEFINITIONS------
//---------------------------------

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

static inline size_t fs_read(char *buffer, size_t buffer_size, FILE *fp, const char *file_path)
{
    size_t read = fread(buffer, 1, buffer_size, fp);
    if (read < buffer_size && ferror(fp))
        log_errno(0, file_path);

    return read;
}

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

//---------------------------------
//----LINE STREAM DEFINITIONS------
//---------------------------------

#define DEFAULT_LINE_BUFFER_SIZE 256
#define LINE_BUFFER_EXP_GROW_LIMIT 64 * 1024 * 1024 // 64 MB
#define LINE_BUFFER_LINEAR_GROW_SIZE 64 * 1024 * 1024 // 64 MB

typedef struct line_stream line_stream;
typedef struct ls_internal ls_internal;

struct ls_internal
{
    const char *file_path;
    FILE *fp;
    char *buffer;
    size_t buffer_idx;
    size_t buffer_read;
    size_t buffer_size;
    size_t line_buffer_size;
};

struct line_stream
{
    struct ls_internal lsi;
    char *line;
    size_t line_length;
};

line_stream *fs_ls_init(const char *file_path, char *buffer, size_t buffer_size);
int fs_ls_read(line_stream *ls);
int fs_ls_file(line_stream *ls, const char *file_path);
int fs_ls_end(line_stream *ls);

#endif