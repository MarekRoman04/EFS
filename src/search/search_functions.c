#include "search.h"

static inline bmh_search_data set_bmh_search(search_data *sd);
static inline bmh_search_data set_bmh_search_line(search_data *sd);
static inline void set_bmh_flags(bmh_search_data *bmh_sd, search_data *sd);
static inline void ls_init(search_data *sd);
static inline int read_line(const char *f_path, line_stream *ls, bmh_search_data *bmh_sd);
int quiet_search(search_data *sd);
int list_search(search_data *sd);
int count_search(search_data *sd);
int line_number_search(search_data *sd);
int print_search(search_data *sd);

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
static inline void ls_init(search_data *sd)
{
    if (sd->ls_searched)
    {
        ls_change_file(sd->ls_searched, sd->fs_searched->fp);
    }
    else
    {
        sd->ls_searched = ls_init_from_fs(sd->fs_searched, sd->buffer.data, sd->buffer.size);
        if (!sd->ls_searched)
            log_error("Error creating line stream!");
    }
}

// Reads line from line stream
static inline int read_line(const char *f_path, line_stream *ls, bmh_search_data *bmh_sd)
{
    int read_val = ls_read(ls);
    if (read_val)
    {
        if (read_val == -1)
            return read_val;
        else
            log_info("Error reading line in %s", f_path);
    }

    bmh_sd->data = ls->line;
    bmh_sd->data_length = ls->line_length;
    bmh_sd->idx = 0;

    return read_val;
}

int quiet_search(search_data *sd)
{
    // Proccess input line by line if searching for words
    if (FLAG_SET(sd->flags, FLAG_WORD))
    {
        ls_init(sd);
        bmh_search_data bmh_sd = set_bmh_search(sd);

        while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmh_sd) != -1)
        {
            if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
                return 0;
        }
    }
    // Buffered search for pattern match
    else
    {
        bmh_search_data bmh_sd = set_bmh_search(sd);
        unsigned char end_idx = 0;

        while ((bmh_sd.data_length = fs_read(sd->fs_searched, sd->buffer.data, sd->buffer.size)))
        {
            if (!sd->bmh_search(&bmh_sd))
                return 0;
        }
    }

    return 1;
}

int list_search(search_data *sd)
{
    int ret_val = 1;
    // Proccess input line by line if searching for words
    if (FLAG_SET(sd->flags, FLAG_WORD))
    {
        ls_init(sd);
        bmh_search_data bmh_sd = set_bmh_search(sd);

        while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmh_sd) != -1)
        {
            if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
            {
                ret_val = 0;
                fprintf(sd->out_p, "%s\n", sd->fs_searched->f_path);
                break;
            }
        }

        sd->buffer.data = NULL;
    }
    // Buffered search for pattern match
    else
    {
        bmh_search_data bmh_sd = set_bmh_search(sd);
        unsigned char end_idx = 0;

        while ((bmh_sd.data_length = fs_read(sd->fs_searched, sd->buffer.data, sd->buffer.size)))
        {
            if (!sd->bmh_search(&bmh_sd))
            {
                ret_val = 0;
                fprintf(sd->out_p, "%s\n", sd->fs_searched->f_path);
                break;
            }
        }
    }

    return ret_val;
}

int count_search(search_data *sd)
{
    int ret_val = 1;
    size_t count = 0;
    ls_init(sd);
    bmh_search_data bmh_sd = set_bmh_search(sd);

    while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmh_sd) != -1)
    {
        if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            count++;
        }
    }

    fprintf(sd->out_p, "%s:%ld\n", sd->fs_searched->f_path, count);

    return ret_val;
}

int line_number_search(search_data *sd)
{
    int ret_val = 1;
    size_t line = 0;
    ls_init(sd);
    bmh_search_data bmh_sd = set_bmh_search(sd);

    while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmh_sd) != -1)
    {
        line++;

        if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            fprintf(sd->out_p, "%s:%ld:", sd->fs_searched->f_path, line);
            fwrite(sd->ls_searched->line, 1, sd->ls_searched->line_length, sd->out_p);
        }
    }

    return ret_val;
}

int print_search(search_data *sd)
{
    int ret_val = 1;
    ls_init(sd);
    bmh_search_data bmh_sd = set_bmh_search(sd);

    while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmh_sd) != -1)
    {
        if ((!sd->bmh_search(&bmh_sd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            fprintf(sd->out_p, "%s:", sd->fs_searched->f_path);
            fwrite(sd->ls_searched->line, 1, sd->ls_searched->line_length, sd->out_p);
        }
    }

    return ret_val;
}
