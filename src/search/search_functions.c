#include "arg_parser.h"
#include "file.h"
#include "file_stream.h"
#include "search.h"

static inline bmh_search_data set_bmh_search(search_data *sd);
int quiet_search(search_data *sd, file_list *fl);
int list_search(search_data *sd, file_list *fl);
int count_search(search_data *sd, file_list *fl);
int line_number_search(search_data *sd, file_list *fl);
int print_search(search_data *sd, file_list *fl);

static inline bmh_search_data set_bmh_search(search_data *sd)
{
    bmh_search_data bmh_sd = {
        .table = sd->table,
        .pattern = sd->pattern,
        .pattern_length = sd->pattern_length,
        .data = sd->buffer.data,
        .data_length = 0,
        .idx = 0,
    };

    return bmh_sd;
}

int quiet_search(search_data *sd, file_list *fl)
{
    file_stream *fs = fs_init(fl->file_paths, fl->file_count);
    if (!fs)
        log_error("Error creating file_stream!");

    bmh_search_data bmh_sd = set_bmh_search(sd);

    do
    {
        while (fs->current_file && fs_open_file(fs, "r"))
            fs_skip_file(fs);

        if (!fs->current_file)
            break;

        unsigned char end_idx = 0;
        while ((bmh_sd.data_length = fs_read_file(fs, sd->buffer.data, sd->buffer.size)))
        {
            if (!bmh_find(&bmh_sd))
            {
                fs_end(fs);
                return 0;
            }
        }
    } while (fs_has_file(fs));

    fs_end(fs);
    return 1;
}

int list_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    file_stream *fs = fs_init(fl->file_paths, fl->file_count);
    if (!fs)
        log_error("Error creating file_stream!");

    bmh_search_data bmh_sd = set_bmh_search(sd);

    do
    {
        while (fs->current_file && fs_open_file(fs, "r"))
            fs_skip_file(fs);

        if (!fs->current_file)
            break;

        unsigned char end_idx = 0;
        while ((bmh_sd.data_length = fs_read_file(fs, sd->buffer.data, sd->buffer.size)))
        {
            if (!bmh_find(&bmh_sd))
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

int count_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    file_stream *fs = fs_init(fl->file_paths, fl->file_count);
    if (!fs)
        log_error("Error creating file_stream!");

    bmh_search_data bmh_sd = set_bmh_search(sd);

    do
    {
        while (fs->current_file && fs_open_file(fs, "r"))
            fs_skip_file(fs);

        if (!fs->current_file)
            break;

        int found = 0;
        unsigned char end_idx = 0;
        while ((bmh_sd.data_length = fs_read_file(fs, sd->buffer.data, sd->buffer.size)))
            found += bmh_count(&bmh_sd);

        if (found)
            ret_val = 0;

        fprintf(sd->out_p, "%s: %d\n", *(fs->current_file), found);
    } while (fs_has_file(fs));

    fs_end(fs);
    return ret_val;
}

int line_number_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    size_t i = 0;
    line_stream *ls = NULL;
    while (!ls && i < fl->file_count)
    {
        ls = fs_ls_init(fl->file_paths[i], sd->buffer.data, sd->buffer.size);
        if (!ls)
            log_info("Ignoring: %s", fl->file_paths[i]);
        i++;
    }

    bmh_search_data bmh_sd = {
        .table = sd->table,
        .pattern = sd->pattern,
        .pattern_length = sd->pattern_length,
        .idx = 0,
    };

    if (!ls)
        log_error("Error creating line stream!");

    for (; i <= fl->file_count; i++)
    {
        size_t line = 0;
        int read_val;
        while ((read_val = fs_ls_read(ls)) != -1)
        {
            if (read_val)
            {
                log_info("Error reading line in %s", ls->lsi.file_path);
                break;
            }

            line++;
            bmh_sd.data = ls->line;
            bmh_sd.data_length = ls->line_length;
            bmh_sd.idx = 0;

            if (!bmh_find(&bmh_sd))
            {
                ret_val = 0;
                fprintf(sd->out_p, "%s:%ld:", ls->lsi.file_path, line);
                fwrite(ls->line, 1, ls->line_length, sd->out_p);
            }
        }

        if (i + 1 <= fl->file_count)
            fs_ls_file(ls, fl->file_paths[i]);
    }

    sd->buffer.data = NULL;
    fs_ls_end(ls);

    return ret_val;
}

int print_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    size_t i = 0;
    line_stream *ls = NULL;
    while (!ls && i < fl->file_count)
    {
        ls = fs_ls_init(fl->file_paths[i], sd->buffer.data, sd->buffer.size);
        if (!ls)
            log_info("Ignoring: %s", fl->file_paths[i]);
        i++;
    }

    bmh_search_data bmh_sd = {
        .table = sd->table,
        .pattern = sd->pattern,
        .pattern_length = sd->pattern_length,
        .idx = 0,
    };

    if (!ls)
        log_error("Error creating line stream!");

    for (; i <= fl->file_count; i++)
    {
        int read_val;
        while ((read_val = fs_ls_read(ls)) != -1)
        {
            if (read_val)
            {
                log_info("Error reading line in %s", ls->lsi.file_path);
                break;
            }

            bmh_sd.data = ls->line;
            bmh_sd.data_length = ls->line_length;
            bmh_sd.idx = 0;

            if (!bmh_find(&bmh_sd))
            {
                ret_val = 0;
                fprintf(sd->out_p, "%s:", ls->lsi.file_path);
                fwrite(ls->line, 1, ls->line_length, sd->out_p);
            }
        }

        if (i + 1 <= fl->file_count)
            fs_ls_file(ls, fl->file_paths[i]);
    }

    sd->buffer.data = NULL;
    fs_ls_end(ls);

    return ret_val;
}
