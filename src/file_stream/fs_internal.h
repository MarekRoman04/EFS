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

//---------------------------------
//----DIR STREAM DEFINITIONS-------
//---------------------------------

#define DEFAULT_PATH_SIZE 256
#define END_OF_DIRECTORY -1


struct dir_stream
{
    const char *dir_name;
    int dir_name_length;
    DIR *dp;
    struct dirent *entry;
};

//---------------------------------
//----RDIR STREAM DEFINITIONS------
//---------------------------------

struct rdir_stream
{
    char *entry_path_buffer;
    int entry_path_buffer_size;
    int entry_path_length;
    dir_stream **ds_stack;
    dir_stream **ds_stack_top;
    int ds_stack_size;
    char *entry_path;
    struct dirent *entry;
    struct stat entry_stat;
};

#define DEFAULT_STACK_SIZE 8

#endif