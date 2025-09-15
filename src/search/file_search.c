#include "search.h"

static inline char_buffer buffer_alloc(cli_args *args);
static inline FILE *set_out_path(cli_args *args);
static inline search_data set_search_data(cli_args *args);

int start_file_search(cli_args *args);

// Allocates buffer from args or uses default size (4KB - 16KB)
static inline char_buffer buffer_alloc(cli_args *args)
{
    char_buffer buffer;

    if (args->buffer_size)
    {
        buffer.size = args->buffer_size;
        buffer.data = (char *)malloc(sizeof(char) * buffer.size);
        if (!buffer.data)
            log_error("Error allocating buffer->data!");
    }
    else
    {
        buffer.size = DEFAULT_BUFFER_SIZE;
        while (buffer.size > MIN_BUFFER_SIZE)
        {
            buffer.data = (char *)malloc(sizeof(char) * buffer.size);
            if (buffer.data)
                break;
            else
                buffer.size /= 2;
        }

        if (!buffer.data)
            log_error("Error allocating buffer->data!");
    }

    return buffer;
}

// Sets output path from args or uses default value (stdout)
static inline FILE *set_out_path(cli_args *args)
{
    FILE *out_p;
    if (args->out_path)
    {
        out_p = fopen(args->out_path, "a");
        if (!out_p)
        {
            log_errno(0, args->out_path);
            log_info("Default value will be used");
            out_p = DEFAULT_OUT_PATH;
        }
    }
    else
        out_p = DEFAULT_OUT_PATH;

    return out_p;
}

// Sets search data for buffered search
static inline search_data set_search_data(cli_args *args)
{
    search_data sd = {
        .buffer = buffer_alloc(args),
        .flags = args->flags,
        .out_p = set_out_path(args),
        .bmh_search = FLAG_SET(args->flags, FLAG_WORD) ? &bmh_find_w : bmh_find,
    };

    sd.fs_searched = fs_init(NULL);
    if (!sd.fs_searched)
        log_error("Error allocating file stream!");

    sd.pattern_length = strlen(args->pattern);
    sd.pattern = (char *)malloc(sizeof(char) * sd.pattern_length);
    if (!sd.pattern)
        log_error("Error allocating memory!");

    strcpy(sd.pattern, args->pattern);

    if (FLAG_SET(sd.flags, FLAG_IGNORE_CASE))
    {

        for (size_t i = 0; i < sd.pattern_length; i++)
            sd.pattern[i] = (char)tolower((unsigned char)sd.pattern[i]);
    }

    sd.table = bmh_pre_process(sd.pattern, (unsigned char)sd.pattern_length);
    if (!sd.table)
        log_error("Error creating bmh_table!");

    return sd;
}

static inline void free_search_data(search_data sd)
{
    if (sd.out_p != stdout && sd.out_p != stderr)
        fclose(sd.out_p);

    fs_end(sd.fs_searched);
    free(sd.table);
    free(sd.buffer.data);
    free(sd.pattern);
}

int start_file_search(cli_args *args)
{
    int ret_val = 1;
    search_data sd = set_search_data(args);

    // Quiet search handled separately to handle early end
    if (FLAG_SET(sd.flags, FLAG_QUIET))
    {
        for (char **current = args->files; current < args->files + args->file_count; current++)
        {
            if (fs_open_file(sd.fs_searched, *(current)))
                continue;

            if (!quiet_search(&sd))
            {
                ret_val = 0;
                break;
            }
        }
    }
    else
    {
        int (*search_function)(search_data *sd);
        if (FLAG_SET(sd.flags, FLAG_LIST))
            search_function = &list_search;
        else if (FLAG_SET(sd.flags, FLAG_COUNT))
            search_function = &count_search;
        else if (FLAG_SET(sd.flags, FLAG_LINE_NUMBER))
            search_function = &line_number_search;
        else
            search_function = &print_search;

        for (char **current = args->files; current < args->files + args->file_count; current++)
        {
            if (fs_open_file(sd.fs_searched, *(current)))
                continue;

            if (!search_function(&sd))
                ret_val = 0;
        }
    }

    free_search_data(sd);
    return ret_val;
}
