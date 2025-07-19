#include "arg_parser.h"
#include "file.h"
#include "file_stream.h"
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
        .pattern = args->pattern,
        .pattern_length = strlen(args->pattern),
        .buffer = buffer_alloc(args),
        .flags = args->flags,
        .out_p = set_out_path(args)};
    sd.table = bmh_pre_process(sd.pattern, (unsigned char)sd.pattern_length);
    if (!sd.table)
        log_error("Error creating bmh_table!");

    return sd;
}

static inline void free_search_data(search_data sd)
{
    if (sd.out_p != stdout && sd.out_p != stderr)
        fclose(sd.out_p);

    free(sd.table);
    free(sd.buffer.data);
}

int start_file_search(cli_args *args)
{
    int ret_val;
    search_data sd = set_search_data(args);
    file_list fl = list_files(args);

    switch (args->flags)
    {
    case FLAG_QUIET:
        ret_val = quiet_search(&sd, &fl);
        break;
    case FLAG_LIST:
        ret_val = list_search(&sd, &fl);
        break;
    case FLAG_COUNT:
        ret_val = count_search(&sd, &fl);
        break;
    case FLAG_LINE_NUMBER:
        ret_val = line_number_search(&sd, &fl);
        break;
    default:
        ret_val = print_search(&sd, &fl);
        break;
    }

    free_search_data(sd);
    return ret_val;
}
