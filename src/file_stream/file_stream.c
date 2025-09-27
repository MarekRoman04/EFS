#include "file_stream.h"

static inline int fs_open(file_stream *fs, const char *f_path);

file_stream *fs_init(const char *f_path);
int fs_open_file(file_stream *fs, const char *f_path);
int fs_close_file(file_stream *fs);
int fs_end(file_stream *fs);

static inline int fs_open(file_stream *fs, const char *f_path)
{
    errno = 0;
    fs->fp = fopen(f_path, "r");
    if (!fs->fp)
    {
        log_info("Error opening file: %s", f_path);
        if (errno)
        log_errno(0, f_path);
        
        return errno ? errno : 1;
    }

    fs->f_path = f_path;

    return 0;
}

file_stream *fs_init(const char *f_path)
{
    file_stream *fs = (file_stream *)malloc(sizeof(file_stream));
    if (!fs)
    {
        log_info("Error allocating memory!");
        return NULL;
    }
    fs->f_path = NULL;
    fs->fp = NULL;

    if (f_path)
        fs_open(fs, f_path);

    return fs;
}

int fs_open_file(file_stream *fs, const char *f_path)
{
    if (fs->fp)
    {
        if (fs_close_file(fs))
            return errno ? errno : 1;
    }

    return fs_open(fs, f_path);
}

int fs_close_file(file_stream *fs)
{
    if (!fs->fp)
        return 0;

    errno = 0;
    if (fclose(fs->fp))
    {
        log_info("Error closing file: %s", fs->f_path);
        if (errno)
            log_errno(0, fs->f_path);

        return errno ? errno : 1;
    }

    fs->f_path = NULL;
    fs->fp = NULL;
    return 0;
}

int fs_end(file_stream *fs)
{
    if (fs_close_file(fs))
        return 1;

    free(fs);
    return 0;
}