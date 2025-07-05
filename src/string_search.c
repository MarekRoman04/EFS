#include "search.h"

size_t buffer_size = 8192;

static inline void string_search_count(const string *pattern, const search_buffer *buffer);
static inline void string_search_list(const string *pattern, const search_buffer *buffer);
static inline void string_search_quiet(const string *pattern, const search_buffer *buffer);
void start_search(const string *pattern, const search_buffer *buffer);

void process_data(cli_args *args)
{
    search_buffer buffer = {.buffer = malloc(sizeof(char) * buffer_size),
                            .buffer_size = buffer_size,
                            .f_path = NULL,
                            .fp = NULL,
                            .flags = args->flags};
    if (buffer.buffer == NULL)
        log_error("Error allocating buffer!\n");

    for (int i = 0; i < args->file_count; i++)
    {
        buffer.f_path = args->files[i];
        FILE *fp = fopen(buffer.f_path, "r");
        if (fp == NULL)
        {
            log_errno(0, buffer.f_path);
            continue;
        }

        string pattern = {.data = args->pattern, .length = strlen(args->pattern)};
        buffer.fp = fp;
        start_search(&pattern, &buffer);
        fclose(buffer.fp);
    }

    free(buffer.buffer);
}

void start_search(const string *pattern, const search_buffer *buffer)
{
    if (FLAG_SET(buffer->flags, FLAG_QUIET))
    {
        string_search_quiet(pattern, buffer);
    }
    else if (FLAG_SET(buffer->flags, FLAG_LIST))
    {
        string_search_list(pattern, buffer);
    }
    else if (FLAG_SET(buffer->flags, FLAG_COUNT))
    {
        string_search_count(pattern, buffer);
    }
}

static inline void string_search_count(const string *pattern, const search_buffer *buffer)
{
    int result = 0;
    string str_file = {.data = buffer->buffer, .length = 0};
    while ((str_file.length = read_file_chunk(buffer, pattern->length)))
    {
        result += bmh_count(pattern, &str_file);
    }

    printf("%s: %d\n", buffer->f_path, result);
}

static inline void string_search_list(const string *pattern, const search_buffer *buffer)
{
    string str_file = {.data = buffer->buffer, .length = 0};
    while ((str_file.length = read_file_chunk(buffer, pattern->length)))
    {
        if (bmh_count(pattern, &str_file) != BMH_NOT_FOUND)
        {
            printf("%s\n", buffer->f_path);
            return;
        }
    }
}

static inline void string_search_quiet(const string *pattern, const search_buffer *buffer)
{
    string str_file = {.data = buffer->buffer, .length = 0};
    while ((str_file.length = read_file_chunk(buffer, pattern->length)))
    {
        if (bmh_count(pattern, &str_file) != BMH_NOT_FOUND)
            exit(EXIT_SUCCESS);
    }
}