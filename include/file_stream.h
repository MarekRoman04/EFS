#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <dirent.h>
#include <stdio.h>
#include "log.h"

//---------------------------------
//----FILE STREAM DEFINITIONS------
//---------------------------------

#define NO_FILE_ERROR (size_t)-1

typedef struct file_stream
{
    const char *f_path;
    FILE *fp;
} file_stream;

/*
 * Opens file stream from given file,
 * file_stream->file_path is only shallow copy,
 * file path is used only for debug logs,
 * if opening failes file_stream->fp is NULL
 */
file_stream *fs_init(const char *f_path);
/*
 * Opens file in file stream, closes previous file,
 * if closing failes file is not changed,
 * if opening failes file_stream->fp is NULL,
 */
int fs_open_file(file_stream *fs, const char *f_path);
/*
 * Closes file in file stream, sets f_path to NULL,
 * if close was successful
 */
int fs_close_file(file_stream *fs);
/*
 * Reads data from file stream into given buffer
 */
static inline size_t fs_read(file_stream *fs, char *buffer, size_t buffer_size)
{
    if (!fs->fp)
        return NO_FILE_ERROR;

    size_t read = fread(buffer, 1, buffer_size, fs->fp);
    if (read < buffer_size && ferror(fs->fp))
        log_errno(0, fs->f_path);

    return read;
}
/*
 * Frees memory used by file stream, closes opened file
 */
int fs_end(file_stream *fs);

//---------------------------------
//----LINE STREAM DEFINITIONS------
//---------------------------------

#define DEFAULT_LINE_BUFFER_SIZE 256
#define LINE_BUFFER_EXP_GROW_LIMIT 64 * 1024 * 1024   // 64 MB
#define LINE_BUFFER_LINEAR_GROW_SIZE 64 * 1024 * 1024 // 64 MB

typedef struct line_stream line_stream;
typedef struct ls_internal ls_internal;

struct ls_internal
{
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

/*
 * Initializes line stream from file stream
 */
line_stream *ls_init_from_fs(file_stream *fs, char *buffer, size_t buffer_size);
/*
 * Reads line from file into line_stream line, overwrites previous read,
 * sets line_length to read line length including \n,
 * returns 0 on successful read
 * returns -1 if no data to read from
 * retruns 1 if line is longer than available memory,
 * contains part of the line that was successfully read
 */
int ls_read(line_stream *ls);
/*
 * Changes file in line stream,
 * resets read buffer
 */
static inline void ls_change_file(line_stream *ls, FILE *fp)
{
    ls->lsi.fp = fp;
    ls->lsi.buffer_idx = 0;
    ls->lsi.buffer_read = 0;
}
/*
 * Frees memory used by line stream
 */
void ls_end(line_stream *ls);

//---------------------------------
//----DIR STREAM DEFINITIONS------
//---------------------------------

// typedef struct dir_stream
// {
//     int dir_count;
//     char **dir_paths;
//     char **current_dir;
//     DIR *current_dp;
// } dir_stream;

// dir_stream *ds_init(char **dir_paths, int dir_count);
// int ds_open_dir(dir_stream *ds);
// int ds_close_dir(dir_stream *ds);
// int ds_end(dir_stream *ds);

// // Check if there is next directory in stream
// static inline int ds_has_dir(dir_stream *ds)
// {
//     return ds->current_dir + 1 < ds->dir_paths + ds->dir_count;
// }

// // Moves current directory pointer to the next directory in stream
// static inline void ds_skip_dir(dir_stream *ds)
// {
//     if (!ds->current_dir)
//         return;

//     if (ds_has_dir(ds))
//         ds->current_dir++;
//     else
//         ds->current_dir = NULL;
// }

// // Reads content from directory
// static inline struct dirent *ds_read(dir_stream *ds)
// {
//     if (!ds->current_dp)
//     {
//         log_info("No directory to read from!");
//         return NULL;
//     }

//     errno = 0;
//     struct dirent *dp = readdir(ds->current_dp);

//     if (dp == NULL)
//     {
//         if (errno == 0)
//         {
//             log_info("No file remaining in directory");
//             return NULL;
//         }
//         else
//         {
//             log_errno(0, *(ds->current_dir));
//             return NULL;
//         }
//     }

//     return dp;
// }

#endif
