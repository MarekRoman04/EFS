#include "fs_internal.h"

file_stream *fs_init(const char *f_path, int flags);
int fs_open_file(file_stream *fs, const char *f_path);
int fs_close_file(file_stream *fs);
ssize_t fs_read(file_stream *fs, char *buffer, size_t n);
const char *fs_get_path(file_stream *fs);
int fs_end(file_stream *fs);

file_stream *fs_init(const char *f_path, int flags)
{
    file_stream *fs = (file_stream *)calloc(1, sizeof(file_stream));
    if (!fs)
        return NULL;

    fs->flags = flags;
    if (f_path)
        fs_open_file(fs, f_path);

    return fs;
}

int fs_open_file(file_stream *fs, const char *f_path)
{
    errno = 0;

    if (fs->fd)
    {
        if (fs_close_file(fs))
            return errno ? errno : 1;
    }

    if ((fs->fd = open(f_path, fs->flags)) == -1)
    {
        fs->f_path = NULL;
        return errno ? errno : 1;
    }

    fs->f_path = f_path;
    return 0;
}

int fs_close_file(file_stream *fs)
{
    errno = 0;
    if (close(fs->fd))
        return errno ? errno : 1;

    fs->f_path = NULL;
    fs->fd = -1;

    return 0;
}

ssize_t fs_read(file_stream *fs, char *buffer, size_t n)
{
    if (fs->fd < 0)
        return -1;

    ssize_t total_read = 0;

    do
    {
        if (fs->read_bytes >= fs->buffer_idx)
        {
            if ((fs->read_bytes = read(fs->fd, fs->buffer, FS_BUFFER_SIZE)) < 0)
                return -1;
        }

        ssize_t buffered_size = fs->read_bytes - fs->buffer_idx;
        ssize_t to_copy = (ssize_t)n < buffered_size ? (ssize_t)n : buffered_size;

        memcpy(buffer + total_read, fs->buffer + fs->buffer_idx, to_copy);
        total_read += to_copy;

    } while (fs->read_bytes && total_read < (ssize_t)n);

    return total_read;
}

const char *fs_get_path(file_stream *fs) { return fs->f_path; }

int fs_end(file_stream *fs)
{
    if (fs_close_file(fs))
        return 1;

    free(fs);
    return 0;
}
