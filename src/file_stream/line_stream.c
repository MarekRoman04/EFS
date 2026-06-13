#include "fs_internal.h"

line_stream *ls_init_from_fs(file_stream *fs);
line ls_read(line_stream *ls);
void ls_change_file(line_stream *ls, int fd);
void ls_end(line_stream *ls);

line_stream *ls_init_from_fs(file_stream *fs)
{
    line_stream *ls = (line_stream *)malloc(sizeof(line_stream));
    if (!ls)
    {
        log_info("Error allocating memory", NULL);
        return NULL;
    }

    ls->fs = fs;
    ls->line_buffer_size = DEFAULT_LINE_BUFFER_SIZE;
    ls->line = (char *)malloc(sizeof(char) * DEFAULT_LINE_BUFFER_SIZE);
    if (!ls->line)
    {
        free(ls);
        return NULL;
    }

    return ls;
}

line ls_read(line_stream *ls)
{
    int line_found = 0;

    do
    {
        // Reads data from file
        if (ls->fs->buffer_idx >= ls->fs->read_bytes)
        {
            if ((ls->fs->read_bytes = fs_read(ls->fs, ls->fs->buffer, FS_BUFFER_SIZE)) <= 0)
                return (line){.data = NULL, .length = ls->fs->read_bytes - 1};
        }

        // Reads line from buffer
        while (ls->fs->buffer_idx < ls->fs->read_bytes)
        {
            char c = ls->fs->buffer[ls->fs->buffer_idx];
            ls->line[ls->line_length] = c;
            ls->fs->buffer_idx++;
            ls->line_length++;

            if (c == '\n')
            {
                line_found = 1;
                break;
            }

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
        }

    } while (!line_found && ls->fs->read_bytes);

    return (line){.data = ls->line, .length = ls->line_length};
}

void ls_change_fs(line_stream *ls, file_stream *fs) { ls->fs = fs; }

void ls_end(line_stream *ls)
{
    free(ls->line);
    free(ls);
}
