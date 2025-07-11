#include "file_stream.h"
#include "log.h"
#include <stdio.h>
#include <unistd.h>

file_stream *fs_init(char **f_paths, int f_count);
int fs_open_file(file_stream *fs, const char *mode);
size_t fs_read_file(file_stream *fs, char *buffer, size_t buffer_size);
void fs_skip_file(file_stream *fs);
int fs_close_file(file_stream *fs);
int fs_end(file_stream *fs);

// Initializes file stream struct for file stream functions
file_stream *fs_init(char **f_paths, int f_count)
{
    file_stream *fs = (file_stream *)malloc(sizeof(file_stream));
    if (!fs)
    {
        log_info("Error allocating memory!");
        return NULL;
    }

    fs->file_paths = f_paths;
    fs->file_count = f_count;
    fs->current_file = f_paths;
    fs->current_fp = NULL;

    return fs;
}

// Opens file in file stream, moves to next file if file is already opened
int fs_open_file(file_stream *fs, const char *mode)
{
    if (fs->current_fp && fs_close_file(fs))
    {
        log_info("Can not move to next file!");
        return 1;
    }

    if (!fs->current_file)
    {
        log_info("No file remaining in stream!");
        return 1;
    }

    fs->current_fp = fopen(*(fs->current_file), mode);
    if (!fs->current_fp)
    {
        log_errno(0, *(fs->current_file));
        return 1;
    }

    return 0;
}

// Reads data from file to buffer from file stream, returns number of bytes read
size_t fs_read_file(file_stream *fs, char *buffer, size_t buffer_size)
{
    if (!fs->current_fp)
    {
        log_info("No file opened to read from!");
        return 0;        
    }

    int fd = fileno(fs->current_fp);
    size_t read = fread(buffer, 1, buffer_size, fs->current_fp);
    if (read < buffer_size && ferror(fs->current_fp))
        log_errno(0, *(fs->current_file));

    return read;
}

// Closes currently opened file in file stream and moves current file pointer to next file
int fs_close_file(file_stream *fs)
{
    if (!fs->current_fp)
    {
        log_info("No file to close!");
        return 1;
    }

    if (fclose(fs->current_fp))
    {
        log_info("Error closing file: %s", *(fs->current_file));
        return 1;
    }

    fs->current_fp = NULL;
    fs_skip_file(fs);

    return 0;
}

// Ends given filestream, frees memory used by file stream struct
int fs_end(file_stream *fs)
{
    if (fs->current_fp && fs_close_file(fs))
    {
        log_info("Error ending file stream!");
        return 1;
    }

    free(fs);
    return 0;
}
