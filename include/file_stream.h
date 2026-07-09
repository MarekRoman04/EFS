#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <log.h>

//---------------------------------
//----FILE STREAM DEFINITIONS------
//---------------------------------

typedef struct file_stream file_stream;

/*
 * Opens file stream from given file,
 * f_path is stored only as shallow copy
 * if f_path is not NULL file stream opens given file with specified flags
 * if opening failes err is set, err is unused if f_path is NULL
 */
file_stream *fs_init(const char *f_path, int flags, int *err);
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
 * Reads n bytes of data from file stream into given buffer
 */
ssize_t fs_read(file_stream *fs, char *buffer, size_t n);
/*
 * Returns file path used to open file,
 * file path is only shallow copy
 */
const char *fs_get_path(file_stream *fs);
/*
 * Frees memory used by file stream, closes opened file
 */
int fs_end(file_stream *fs);

//---------------------------------
//----LINE STREAM DEFINITIONS------
//---------------------------------

typedef struct line_stream line_stream;

typedef struct line
{
    char *data;
    ssize_t length;
} line;

/*
 * Initializes line stream from file stream
 */
line_stream *ls_init_from_fs(file_stream *fs);
/*
 * Reads line from file into line_stream line, overwrites previous read,
 * sets line_length to read line length including \n,
 * if line is longer than available memory contains part
 * of the line that was successfully read
 * on error line length is set to -1 and line data is NULL,
 * on EOF line length is set to 0 and line data is NULL,
 */
line ls_read(line_stream *ls);
/*
 * Changes file stream to read lines from,
 */
void ls_change_fs(line_stream *ls, file_stream *fs);
/*
 * Frees memory used by line stream
 */
void ls_end(line_stream *ls);

//---------------------------------
//----DIR STREAM DEFINITIONS-------
//---------------------------------

typedef struct dir_stream dir_stream;
/*
 * Opens dir stream from given directory,
 * if dp_at is set, path is relative to the dp_at directory,
 * dir_stream->dir_name is only shallow copy,
 * if opening failes dir_stream->dp is NULL
 * and err is set
 */
dir_stream *ds_init(const char *dir_name, DIR *dp_at, int *err);
/*
 * Opens directory in dir stream, closes previous directory,
 * if dp_at is set, path is relative to the dp_at directory,
 * if closing failes directory is not changed,
 */
int ds_open_dir(dir_stream *ds, DIR *dp_at, const char *dir_name);
/*
 * Closes directory in dir stream, sets dir_name to NULL,
 * if close was successful
 */
int ds_close_dir(dir_stream *ds);
/*
 * Frees memory used by dir stream, closes opened directory
 */
int ds_end(dir_stream *ds);

//---------------------------------
//----RDIR STREAM DEFINITIONS------
//---------------------------------

#define DEFAULT_STACK_SIZE 8 // move to internal header
#define END_OF_DIR 0
#define FILE_NOT_DIR 1

typedef struct rdir_stream rdir_stream;
/*
 * Opens recursive dir stream from given directory,
 * if opening directory failes err is set
 */
rdir_stream *rds_init(const char *dir_name, int *err);
/*
 * Reads next entry from directory,
 * returns entry path relative to initial directory
 * on error NULL is returned and err is set,
 * on end of directory NULL is returned and err is 0
 */
const char *rds_current_dir(rdir_stream *rds);
const char *rds_read_entry_name(rdir_stream *rds, int *err);
/*
 * Closes all currently opened directories and opens stream from given directory,
 * if opening fails no directory remains open in stream
 */
int rds_change_dir(rdir_stream *rds, const char *dir_name);
/*
 * Closes all opened dir streams and frees memory used by recursive dir stream
 */
int rds_end(rdir_stream *rds);

#endif
