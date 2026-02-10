#include <file_stream.h>

line_stream *ls_init_from_fs(file_stream *fs, char *buffer, size_t buffer_size);
int ls_read(line_stream *ls);
void ls_end(line_stream *ls);

line_stream *ls_init_from_fs(file_stream *fs, char *buffer, size_t buffer_size)
{
    line_stream *ls = (line_stream *)malloc(sizeof(line_stream));
    if (!ls)
    {
        log_info("Error allocating memory", NULL);
        return NULL;
    }

    ls->lsi.fp = fs->fp;
    ls->lsi.buffer = buffer;
    ls->lsi.buffer_read = 0;
    ls->lsi.buffer_idx = 0;
    ls->lsi.buffer_size = buffer_size;
    ls->lsi.line_buffer_size = DEFAULT_LINE_BUFFER_SIZE;
    ls->line = (char *)malloc(sizeof(char) * DEFAULT_LINE_BUFFER_SIZE);
    if (!ls->line)
    {
        log_info("Error allocating memory", NULL);
        fclose(ls->lsi.fp);
        free(ls);
        return NULL;
    }

    return ls;
}

int ls_read(line_stream *ls)
{
    int line_found = 0;
    ls->line_length = 0;

    do
    {
        // Reads data from file
        if (ls->lsi.buffer_read <= ls->lsi.buffer_idx)
        {
            ls->lsi.buffer_read = fread(ls->lsi.buffer, 1, ls->lsi.buffer_size, ls->lsi.fp);
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

void ls_end(line_stream *ls)
{
    free(ls->line);
    free(ls);
}
