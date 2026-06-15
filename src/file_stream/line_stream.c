#include "fs_internal.h"

line_stream *ls_init_from_fs(file_stream *fs);
line ls_read(line_stream *ls);
void ls_end(line_stream *ls);

line_stream *ls_init_from_fs(file_stream *fs)
{
    line_stream *ls = calloc(1, sizeof(line_stream));
    if (!ls)
        return NULL;

    ls->fs = fs;
    ls->line_buffer_size = DEFAULT_LINE_BUFFER_SIZE;
    ls->line = malloc(sizeof(char) * DEFAULT_LINE_BUFFER_SIZE);
    if (!ls->line)
    {
        free(ls);
        return NULL;
    }

    return ls;
}

line ls_read(line_stream *ls)
{
    ls->line_length = 0;

    while (1)
    {
        if (ls->fs->buffer_idx >= ls->fs->read_bytes)
        {
            ls->fs->buffer_idx = 0;
            ls->fs->read_bytes = read(ls->fs->fd, ls->fs->buffer, FS_BUFFER_SIZE);

            if (ls->fs->read_bytes < 0)
                return (line){.data = NULL, .length = -1};

            if (ls->fs->read_bytes == 0)
            {
                if (ls->line_length)
                    return (line){.data = ls->line, .length = ls->line_length};
                else
                    return (line){.data = NULL, .length = 0};
            }
        }

        while (ls->fs->buffer_idx < ls->fs->read_bytes)
        {
            if ((ssize_t)ls->line_buffer_size <= ls->line_length)
            {
                char *new_line = realloc(ls->line, ls->line_buffer_size * 2);
                if (!new_line)
                    return (line){.data = ls->line, .length = ls->line_length};
                else
                {
                    ls->line = new_line;
                    ls->line_buffer_size *= 2;
                }
            }

            char c = ls->fs->buffer[ls->fs->buffer_idx];
            ls->line[ls->line_length] = c;
            ls->fs->buffer_idx++;
            ls->line_length++;

            if (c == '\n')
                return (line){.data = ls->line, .length = ls->line_length};
        }
    };
}

void ls_change_fs(line_stream *ls, file_stream *fs) { ls->fs = fs; }

void ls_end(line_stream *ls)
{
    if (!ls)
        return;

    free(ls->line);
    free(ls);
}
