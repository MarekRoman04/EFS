#include "arg_parser.h"
#include "file.h"
#include "file_stream.h"
#include "search.h"

int start_file_search(cli_args *args);
inline int handle_quiet_search(search_data *sd, file_list *fl);
inline int handle_list_search(search_data *sd, file_list *fl);
inline int handle_count_search(search_data *sd, file_list *fl);
// static inline int line_number_search(cli_args *args);
// static inline int print_search(cli_args *args);

int start_file_search(cli_args *args)
{
    int ret_val;

    // Allocates buffer from args or uses default size (4096 - 16384)
    char *buffer;
    size_t buffer_size;
    if (args->buffer_size)
    {
        buffer_size = args->buffer_size;
        buffer = (char *)malloc(sizeof(char) * buffer_size);
        if (!buffer)
            log_error("Error allocating buffer!");
    }
    else
    {
        buffer_size = DEFAULT_BUFFER_SIZE;
        while (buffer_size > MIN_BUFFER_SIZE)
        {
            buffer = (char *)malloc(sizeof(char) * buffer_size);
            if (buffer)
                break;
            else
                buffer_size /= 2;
        }

        if (!buffer)
            log_error("Error allocating buffer!");
    }

    // Sets output path from args or uses default value (stdout)
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

    file_list fl = list_files(args);
    search_data sd = {
        .pattern = args->pattern,
        .pattern_length = strlen(args->pattern),
        .buffer = buffer,
        .buffer_size = buffer_size,
        .flags = args->flags,
        .out_p = out_p};
    sd.table = bmh_pre_process(sd.pattern, (unsigned char)sd.pattern_length);
    if (!sd.table)
        log_error("Error creating bmh_table!");

    switch (args->flags)
    {
    case FLAG_QUIET:
        ret_val = handle_quiet_search(&sd, &fl);
        break;
    case FLAG_LIST:
        ret_val = handle_list_search(&sd, &fl);
        break;
    case FLAG_COUNT:
        ret_val = handle_count_search(&sd, &fl);
        break;
    case FLAG_LINE_NUMBER:
        /* code */
        break;
    default:
        /* code */
        break;
    }

    free(sd.table);
    free(buffer);
    return ret_val;
}

inline int handle_quiet_search(search_data *sd, file_list *fl)
{
    file_stream *fs = fs_init(fl->file_paths, fl->file_count);
    size_t read;

    do
    {
        while (fs->current_file && fs_open_file(fs, "r"))
            fs_skip_file(fs);

        if (!fs->current_file)
            break;

        unsigned char end_idx = 0;
        while ((read = fs_read_file(fs, sd->buffer, sd->buffer_size)))
        {
            if (!bmh_find(sd->table, sd->pattern, sd->pattern_length, sd->buffer, read, end_idx, &end_idx))
            {
                fs_end(fs);
                return 0;
            }
        }
    } while (fs_has_file(fs));

    fs_end(fs);
    return 1;
}

inline int handle_list_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    file_stream *fs = fs_init(fl->file_paths, fl->file_count);
    size_t read;

    do
    {
        while (fs->current_file && fs_open_file(fs, "r"))
            fs_skip_file(fs);

        if (!fs->current_file)
            break;

        unsigned char end_idx = 0;
        while ((read = fs_read_file(fs, sd->buffer, sd->buffer_size)))
        {
            if (!bmh_find(sd->table, sd->pattern, sd->pattern_length, sd->buffer, read, end_idx, &end_idx))
            {
                ret_val = 0;
                fprintf(sd->out_p, "%s\n", *(fs->current_file));
                break;
            }
        }
    } while (fs_has_file(fs));

    fs_end(fs);
    return ret_val;
}

inline int handle_count_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    file_stream *fs = fs_init(fl->file_paths, fl->file_count);
    size_t read;

    do
    {
        while (fs->current_file && fs_open_file(fs, "r"))
            fs_skip_file(fs);

        if (!fs->current_file)
            break;

        int found = 0;
        unsigned char end_idx = 0;
        while ((read = fs_read_file(fs, sd->buffer, sd->buffer_size)))
        {
            ret_val = 0;
            found += bmh_count(sd->table, sd->pattern, sd->pattern_length, sd->buffer, read, end_idx, &end_idx);
        }

        fprintf(sd->out_p, "%s: %d\n", *(fs->current_file), found);
    } while (fs_has_file(fs));

    fs_end(fs);
    return ret_val;
}
