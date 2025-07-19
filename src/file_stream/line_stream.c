#include "file_stream.h"

static inline line_buffer *line_buffer_alloc(size_t buffer_size);
static inline void line_buffer_free(line_buffer *buffer);
static inline ls_internal *ls_internal_init(const char *file_path, char *buffer, size_t buffer_size);
line_stream *fs_ls_init(const char *file_path, char *buffer, size_t buffer_size);
int fs_ls_read_line(line_stream *ls);
int fs_ls_change_file(line_stream *ls, const char *file_path);
int fs_ls_end(line_stream *ls);

// Allocates line buffer node
static inline line_buffer *line_buffer_alloc(size_t buffer_size)
{
    line_buffer *lb = (line_buffer *)malloc(sizeof(line_buffer));
    if (!lb)
    {
        log_info("Error allocating memory line_buffer");
        return NULL;
    }

    char *buffer = (char *)malloc(sizeof(char) * buffer_size);
    if (!buffer)
    {
        log_info("Error allocating memory line_buffer buffer");
        free(lb);
        return NULL;
    }

    lb->buffer = buffer;
    lb->buffer_idx = 0;
    lb->buffer_size = buffer_size;
    lb->next_buffer = NULL;
    return lb;
}

// Frees all line buffers in linked list
static inline void line_buffer_free(line_buffer *buffer)
{
    if (!buffer)
        return;

    line_buffer *lb = buffer;
    line_buffer *lb_next = buffer->next_buffer;

    while (lb_next)
    {
        free(lb->buffer);
        free(lb);
        lb = lb_next;
        lb_next = lb->next_buffer;
    }

    free(lb->buffer);
    free(lb);
}

// Allocated line stream internal struct
static inline ls_internal *ls_internal_init(const char *file_path, char *buffer, size_t buffer_size)
{
    ls_internal *lsi = (ls_internal *)malloc(sizeof(ls_internal));
    if (!lsi)
    {
        log_info("Error allocating memory line_stream_internal");
        return NULL;
    }

    lsi->fp = fopen(file_path, "r");
    if (!lsi->fp)
    {
        log_errno(0, file_path);
        line_buffer_free(lsi->empty_lb_head);
        free(lsi);
        return NULL;
    }

    lsi->buffer = buffer;
    lsi->buffer_idx = 0;
    lsi->buffer_size = buffer_size;
    lsi->read = 0;
    lsi->empty_lb_head = NULL;
    lsi->empty_lb_tail = NULL;
}

// Initializes line stream struct
line_stream *fs_ls_init(const char *file_path, char *buffer, size_t buffer_size)
{
    line_stream *ls = (line_stream *)malloc(sizeof(line_stream));
    if (!ls)
    {
        log_info("Error allocating memory line_stream");
        return NULL;
    }

    ls->file_path = file_path;

    ls->line = (line_data *)malloc(sizeof(line_data));
    if (!ls->line)
    {
        log_info("Error allocating memory line_stream line_data");
        free(ls);
        return NULL;
    }

    line_buffer *lb = line_buffer_alloc(DEFAULT_LINE_BUFFER_SIZE);
    if (!lb)
    {
        log_info("Error allocating memory line_stream line_data lb");
        free(ls->line);
        free(ls);
        return NULL;
    }

    ls->line->lb_head = lb;
    ls->line->lb = lb;

    ls->lsi = ls_internal_init(file_path, buffer, buffer_size);
    if (!ls->lsi)
    {
        log_info("Error allocating memory line_stream line_stream_internal");
        line_buffer_free((line_buffer *)ls->line->lb_head);
        free(ls->line);
        free(ls);
        return NULL;
    }

    return ls;
}

/*
 * Reads data from file, overwrites previous line data buffers,
 * lb point to firt buffer in list,
 * unused buffers are returned to line stream.
 */
int fs_ls_read_line(line_stream *ls)
{
    fs_ls_line_reset(ls);
    size_t line_length = 0;
    int line_found = 0;

    do
    {
        // Reads data from file
        if (ls->lsi->read <= ls->lsi->buffer_idx)
        {
            ls->lsi->buffer_idx = 0;
            ls->lsi->read = fs_read(ls->lsi->buffer, ls->lsi->buffer_size, ls->lsi->fp, ls->file_path);
        }

        if (!ls->lsi->read)
        {
            if (line_length)
            {
                line_found = 1;
                break;
            }
            else
            {
                log_info("No data to read from!");
                return 1;
            }
        }

        // Reads line from buffer
        while (ls->lsi->buffer_idx < ls->lsi->read)
        {
            char c = ls->lsi->buffer[ls->lsi->buffer_idx];
            ls->line->lb->buffer[ls->line->lb->buffer_idx] = c;
            ls->lsi->buffer_idx++;
            ls->line->lb->buffer_idx++;
            line_length++;

            if (c == '\n')
            {
                line_found = 1;
                break;
            }

            if (ls->line->lb->buffer_idx > ls->line->lb->buffer_size)
            {
                int j = 1;
            }

            // Moves to next line buffer, allocates new buffer if all buffers are used
            if (ls->line->lb->buffer_size <= ls->line->lb->buffer_idx)
            {
                if (ls->lsi->empty_lb_head)
                {
                    ls->line->lb->next_buffer = ls->lsi->empty_lb_head;
                    ls->lsi->empty_lb_head = ls->lsi->empty_lb_head->next_buffer;
                    if (ls->lsi->empty_lb_head == NULL)
                        ls->lsi->empty_lb_tail = NULL;

                    ls->line->lb = ls->line->lb->next_buffer;
                    ls->line->lb->next_buffer = NULL;
                }
                else
                {
                    ls->line->lb->next_buffer = line_buffer_alloc(DEFAULT_LINE_BUFFER_SIZE);
                    if (!ls->line->lb->next_buffer)
                    {
                        log_info("Error allocating memory line_stream line line_buffer next_buffer");
                        return 1;
                    }

                    ls->line->lb = ls->line->lb->next_buffer;
                }
            }
        }

    } while (!line_found);

    ls->line->lb = (line_buffer *)ls->line->lb_head;
    ls->line->line_length = line_length;
    return 0;
}

// Changes file in line stream and resets line buffer
int fs_ls_change_file(line_stream *ls, const char *file_path)
{
    if (fclose(ls->lsi->fp))
    {
        log_info("Error cloasing file: %s", ls->file_path);
        return 1;
    }

    ls->file_path = file_path;
    ls->lsi->fp = fopen(file_path, "r");
    if (!ls->lsi->fp)
    {
        log_errno(0, file_path);
        return 1;
    }

    ls->lsi->buffer_idx = 0;
    fs_ls_line_reset(ls);
}

// Ends line stream and frees all memory used
int fs_ls_end(line_stream *ls)
{
    if (fclose(ls->lsi->fp))
    {
        log_info("Error closing file: %s", ls->file_path);
        return 1;
    }

    line_buffer_free((line_buffer *)ls->line->lb_head);
    free(ls->line);
    line_buffer_free(ls->lsi->empty_lb_head);
    free(ls->lsi);
    free(ls);

    return 0;
}
