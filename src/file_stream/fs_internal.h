#ifndef FS_INTERNAL_H
#define FS_INTERNAL_H

#include <file_stream.h>

//---------------------------------
//----FILE STREAM DEFINITIONS------
//---------------------------------

#define FS_BUFFER_SIZE (1u << 16)

struct file_stream
{
    const char *f_path;
    int fd;
    int flags;
    char buffer[FS_BUFFER_SIZE];
    ssize_t read_bytes;
    ssize_t buffer_idx;
};

//---------------------------------
//----LINE STREAM DEFINITIONS------
//---------------------------------

#define DEFAULT_LINE_BUFFER_SIZE 256
#define LINE_BUFFER_EXP_GROW_LIMIT 64 * 1024 * 1024   // 64 MB
#define LINE_BUFFER_LINEAR_GROW_SIZE 64 * 1024 * 1024 // 64 MB

struct line_stream
{
    file_stream *fs;
    char *line;
    ssize_t line_length;
    size_t line_buffer_size;
};

#endif