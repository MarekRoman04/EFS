#include "search.h"

static inline bmh_search_data set_bmh_search(search_data *sd);
static inline bmh_search_data set_bmh_search_line(search_data *sd);
static inline void set_bmh_flags(bmh_search_data *bmh_sd, search_data *sd);
static inline line_stream *ls_init(file_list *fl, search_data *sd, size_t *i);
static inline int read_line(line_stream *ls, bmh_search_data *bmh_sd);
int quiet_search(search_data *sd, file_list *fl);
int list_search(search_data *sd, file_list *fl);
int count_search(search_data *sd, file_list *fl);
int line_number_search(search_data *sd, file_list *fl);
int print_search(search_data *sd, file_list *fl);

// Initializes bmh_sd for buffered search
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

    set_bmh_flags(&bmh_sd, sd);

    return bmh_sd;
}

// Initializes bmh_sd for line search
static inline bmh_search_data set_bmh_search_line(search_data *sd)
{
    bmh_search_data bmh_sd = {
        .table = sd->table,
        .pattern = sd->pattern,
        .pattern_length = sd->pattern_length,
        .idx = 0,
    };

    set_bmh_flags(&bmh_sd, sd);

    return bmh_sd;
}

// Sets bmh_search flags
static inline void set_bmh_flags(bmh_search_data *bmh_sd, search_data *sd)
{
    if (FLAG_SET(sd->flags, FLAG_IGNORE_CASE))
        bmh_sd->flags |= BMH_IGNORE_CASE;
    if (FLAG_SET(sd->flags, FLAG_WORD))
        bmh_sd->flags |= BHM_WORD;
}

// Initializes line stream
static inline line_stream *ls_init(file_list *fl, search_data *sd, size_t *i)
{
    line_stream *ls = NULL;
    while (!ls && *i < fl->file_count)
    {
        ls = fs_ls_init(fl->file_paths[*i], sd->buffer.data, sd->buffer.size);
        if (!ls)
            log_info("Ignoring: %s", fl->file_paths[*i]);
        (*i)++;
    }

    if (!ls)
        log_error("Error creating line stream!");

    return ls;
}

// Reads line from line stream
static inline int read_line(line_stream *ls, bmh_search_data *bmh_sd)
{
    int read_val = fs_ls_read(ls);
    if (read_val)
    {
        if (read_val == -1)
            return read_val;
        else
            log_info("Error reading line in %s", ls->lsi.file_path);
    }

    bmh_sd->data = ls->line;
    bmh_sd->data_length = ls->line_length;
    bmh_sd->idx = 0;

    return read_val;
}

int quiet_search(search_data *sd, file_list *fl)
{
    // Proccess input line by line if searching for words
    if (FLAG_SET(sd->flags, FLAG_WORD))
    {
        size_t i = 0;
        line_stream *ls = ls_init(fl, sd, &i);
        bmh_search_data bmh_sd = set_bmh_search(sd);

        for (; i <= fl->file_count; i++)
        {
            while (read_line(ls, &bmh_sd) != -1)
            {
                if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
                {
                    sd->buffer.data = NULL;
                    fs_ls_end(ls);
                    return 0;
                }
            }

            if (i + 1 <= fl->file_count)
                fs_ls_file(ls, fl->file_paths[i]);
        }

        sd->buffer.data = NULL;
        fs_ls_end(ls);
    }
    // Buffered search for pattern match
    else
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
                if (!sd->bmh_search(&bmh_sd))
                {
                    fs_end(fs);
                    return 0;
                }
            }
        } while (fs_has_file(fs));

        fs_end(fs);
    }

    return 1;
}

int list_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;

    if (FLAG_SET(sd->flags, FLAG_WORD))
    {
        size_t i = 0;
        line_stream *ls = ls_init(fl, sd, &i);
        bmh_search_data bmh_sd = set_bmh_search(sd);

        for (; i <= fl->file_count; i++)
        {
            while (read_line(ls, &bmh_sd) != -1)
            {
                if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
                {
                    ret_val = 0;
                    fprintf(sd->out_p, "%s\n", ls->lsi.file_path);
                    break;
                }
            }

            if (i + 1 <= fl->file_count)
                fs_ls_file(ls, fl->file_paths[i]);
        }

        sd->buffer.data = NULL;
        fs_ls_end(ls);
    }
    else
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
                if (!sd->bmh_search(&bmh_sd))
                {
                    ret_val = 0;
                    fprintf(sd->out_p, "%s\n", *(fs->current_file));
                    break;
                }
            }
        } while (fs_has_file(fs));

        fs_end(fs);
    }
    return ret_val;
}

int count_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    size_t i = 0;
    line_stream *ls = ls_init(fl, sd, &i);
    bmh_search_data bmh_sd = set_bmh_search(sd);

    for (; i <= fl->file_count; i++)
    {
        size_t count = 0;
        while (read_line(ls, &bmh_sd) != -1)
        {
            if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
            {
                ret_val = 0;
                count++;
            }
        }

        fprintf(sd->out_p, "%s:%ld\n", ls->lsi.file_path, count);

        if (i + 1 <= fl->file_count)
            fs_ls_file(ls, fl->file_paths[i]);
    }

    sd->buffer.data = NULL;
    fs_ls_end(ls);

    return ret_val;
}

int line_number_search(search_data *sd, file_list *fl)
{
    int ret_val = 1;
    size_t i = 0;
    line_stream *ls = ls_init(fl, sd, &i);
    bmh_search_data bmh_sd = set_bmh_search(sd);

    for (; i <= fl->file_count; i++)
    {
        size_t line = 0;
        int read_val;
        while (read_line(ls, &bmh_sd) != -1)
        {
            line++;

            if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
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
    line_stream *ls = ls_init(fl, sd, &i);
    bmh_search_data bmh_sd = set_bmh_search(sd);

    for (; i <= fl->file_count; i++)
    {
        int read_val;
        while (read_line(ls, &bmh_sd) != -1)
        {
            if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
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
