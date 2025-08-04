#include "file_stream.h"

line_stream *fs_ls_init(const char *file_path, char *buffer, size_t buffer_size);
int fs_ls_read(line_stream *ls);
int fs_ls_file(line_stream *ls, const char *file_path);
int fs_ls_end(line_stream *ls);

/*
 * Initializes line stream from given file,
 * if buffer is null, allocates buffer with buffer_size,
 * if buffer_size is 0 reads directly from file
 */
line_stream *fs_ls_init(const char *file_path, char *buffer, size_t buffer_size)
{
    line_stream *ls = (line_stream *)malloc(sizeof(line_stream));
    if (!ls)
    {
        log_info("Error allocating memory");
        return NULL;
    }

    ls->lsi.fp = fopen(file_path, "r");
    if (!ls->lsi.fp)
    {
        log_errno(0, file_path);
        free(ls);
        return NULL;
    }

    ls->lsi.file_path = file_path;
    ls->lsi.line_buffer_size = 0;
    ls->line_length = 0;
    ls->lsi.line_buffer_size = DEFAULT_LINE_BUFFER_SIZE;
    ls->line = (char *)malloc(sizeof(char) * DEFAULT_LINE_BUFFER_SIZE);
    if (!ls->line)
    {
        log_info("Error allocating memory");
        fclose(ls->lsi.fp);
        free(ls);
        return NULL;
    }

    if (buffer_size)
    {
        if (!buffer)
        {
            buffer = (char *)malloc(sizeof(char) * buffer_size);
            if (!buffer)
            {
                log_info("Error allocating memory");
                fclose(ls->lsi.fp);
                free(ls->line);
                free(ls);
                return NULL;
            }
        }
    }

    ls->lsi.buffer = buffer;
    ls->lsi.buffer_idx = 0;
    ls->lsi.buffer_read = 0;
    ls->lsi.buffer_size = buffer_size;

    return ls;
}

/*
 * Reads line from file into line_stream line, overwrites previous read,
 * sets line_length to read line length including \n,
 * returns 0 on successful read
 * returns -1 if no data to read from
 * retruns 1 if line is longer than available memory,
 * contains part of the line that was successfully read
 */
int fs_ls_read(line_stream *ls)
{
    int line_found = 0;
    ls->line_length = 0;

    do
    {
        // Reads data from file
        if (ls->lsi.buffer_read <= ls->lsi.buffer_idx)
        {
            ls->lsi.buffer_read =  fs_read(ls->lsi.buffer, ls->lsi.buffer_size, ls->lsi.fp, ls->lsi.file_path);
            ls->lsi.buffer_idx = 0;
        }

        if (!ls->lsi.buffer_read)
        {
            if (ls->line_length)
                line_found = 1;
            else
                return -1;
        }

        // Reads line from buffer
        while (ls->lsi.buffer_idx < ls->lsi.buffer_read)
        {
            char c = ls->lsi.buffer[ls->lsi.buffer_idx];
            ls->line[ls->line_length] = c;
            ls->lsi.buffer_idx++;
            ls->line_length++;

            if (c == '\n')
            {
                line_found = 1;
                break;
            }

            if (ls->lsi.line_buffer_size <= ls->line_length)
            {
                char *new_line = realloc(ls->line, ls->lsi.line_buffer_size * 2);
                if (!new_line)
                    return 1;
                else
                {
                    ls->line = new_line;
                    ls->lsi.line_buffer_size *= 2;
                }
            }
        }

    } while (!line_found);

    return 0;
}

/*
 * Changes file used in line_stream,
 * if file opening fails previous file remains
 */
int fs_ls_file(line_stream *ls, const char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (!fp)
        return 1;

    fclose(ls->lsi.fp);
    ls->lsi.fp = fp;
    ls->lsi.file_path = file_path;

    return 0;
}

/*
 * Ends given line_stream, frees memory used by line_stream,
 * if buffer was given on init, this buffer is also freed
 */
int fs_ls_end(line_stream *ls)
{
    if (fclose(ls->lsi.fp))
    {
        log_info("Error ending line stream!");
        return 1;
    }

    free(ls->lsi.buffer);
    free(ls->line);
    free(ls);

    return 0;
}
