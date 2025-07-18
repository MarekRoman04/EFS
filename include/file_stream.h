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

typedef struct line_stream line_stream;
typedef struct ls_internal ls_internal;
typedef struct line_buffer line_buffer;
typedef struct line_data line_data;

struct line_stream
{
    const char *file_path;
    line_data *line;
    ls_internal *lsi;
};

// Contains internal data used in line stream
struct ls_internal
{
    FILE *fp;
    // File data buffer
    char *buffer;
    size_t buffer_idx;
    size_t buffer_size;
    size_t read;
    // Empty buffer link list
    line_buffer *empty_lb_head;
    line_buffer *empty_lb_tail;
};

// Line buffer linked list node
struct line_buffer
{
    char *buffer;
    size_t buffer_idx;
    size_t buffer_size;
    line_buffer *next_buffer;
};

// Contains line data in line buffer linked list
struct line_data
{
    const line_buffer *lb_head;
    line_buffer *lb;
    size_t line_length;
};

line_stream *fs_ls_init(const char *file_path, char *buffer, size_t buffer_size);
int fs_ls_read_line(line_stream *ls);
int fs_ls_end(line_stream *ls);

// Resets line data buffer in line stream
static inline void fs_ls_line_reset(line_stream *ls)
{
    ls->line->lb = (line_buffer *)ls->line->lb_head;
    ls->line->lb->buffer_idx = 0;

    // Moves unused buffers to ls_internal empty buffers
    if (ls->line->lb->next_buffer)
    {
        if (ls->lsi->empty_lb_tail)
            ls->lsi->empty_lb_tail->next_buffer = ls->line->lb->next_buffer;
        else
        {
            ls->lsi->empty_lb_head = ls->line->lb->next_buffer;
            ls->lsi->empty_lb_tail = ls->line->lb->next_buffer;
        }

        while (ls->lsi->empty_lb_tail->next_buffer)
        {
            ls->lsi->empty_lb_tail = ls->lsi->empty_lb_tail->next_buffer;
            ls->lsi->empty_lb_tail->buffer_idx = 0;
        }

        ls->line->lb->next_buffer = NULL;
    }
}

// Checks if there is next line in line stream
static inline int fs_ls_has_line(line_stream *ls)
{
    if (ls->lsi->read)
        return 0;

    if ((ls->lsi->read = fs_read(ls->lsi->buffer, ls->lsi->buffer_size, ls->lsi->fp, ls->file_path)))
    {
        ls->lsi->buffer_idx = 0;
        return 0;
    }

    return 1;
}

#endif
