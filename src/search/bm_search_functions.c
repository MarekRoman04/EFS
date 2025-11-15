#include "search.h"

static inline bm_data set_bmh_search(bm_search_data *sd);
static inline bm_data set_bmh_search_line(bm_search_data *sd);
static inline void ls_init(bm_search_data *sd);
static inline int read_line(const char *f_path, line_stream *ls, bm_data *bmd);
int bm_quiet_search(bm_search_data *sd);
int bm_list_search(bm_search_data *sd);
int bm_count_search(bm_search_data *sd);
int bm_line_number_search(bm_search_data *sd);
int bm_print_search(bm_search_data *sd);

/*
 *---TO-DO---
 *Move bm_data to bm_search_data struct
 */

// Initializes bmd for buffered search
static inline bm_data set_bmh_search(bm_search_data *sd)
{
    bm_data bmd = {
        .bad_char_table = sd->bad_char_table,
        .good_suffix_table = sd->good_suffix_table,
        .pattern = sd->pattern,
        .pattern_length = sd->pattern_length,
        .data = sd->buffer.data,
        .data_length = 0,
        .idx = 0,
        .ignore_case = FLAG_SET(sd->flags, FLAG_IGNORE_CASE),
    };

    return bmd;
}

// Initializes bmd for line search
static inline bm_data set_bmh_search_line(bm_search_data *sd)
{
    bm_data bmd = {
        .bad_char_table = sd->bad_char_table,
        .good_suffix_table = sd->good_suffix_table,
        .pattern = sd->pattern,
        .pattern_length = sd->pattern_length,
        .idx = 0,
        .ignore_case = FLAG_SET(sd->flags, FLAG_IGNORE_CASE),
    };

    return bmd;
}

// Initializes line stream
static inline void ls_init(bm_search_data *sd)
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
static inline int read_line(const char *f_path, line_stream *ls, bm_data *bmd)
{
    int read_val = ls_read(ls);
    if (read_val)
    {
        if (read_val == -1)
            return read_val;
        else
            log_info("Error reading line in %s", f_path);
    }

    bmd->data = ls->line;
    bmd->data_length = ls->line_length;
    bmd->idx = 0;

    return read_val;
}

int bm_quiet_search(bm_search_data *sd)
{
    // Proccess input line by line if searching for words
    if (FLAG_SET(sd->flags, FLAG_WORD))
    {
        ls_init(sd);
        bm_data bmd = set_bmh_search(sd);

        while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmd) != -1)
        {
            if ((!sd->bmh_search(&bmd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
                return 0;
        }
    }
    // Buffered search for pattern match
    else
    {
        bm_data bmd = set_bmh_search(sd);

        while ((bmd.data_length = fs_read(sd->fs_searched, sd->buffer.data, sd->buffer.size)))
        {
            if (!sd->bmh_search(&bmd))
                return 0;
        }
    }

    return 1;
}

int bm_list_search(bm_search_data *sd)
{
    int ret_val = 1;
    // Proccess input line by line if searching for words
    if (FLAG_SET(sd->flags, FLAG_WORD))
    {
        ls_init(sd);
        bm_data bmd = set_bmh_search(sd);

        while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmd) != -1)
        {
            if ((!sd->bmh_search(&bmd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
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
        bm_data bmd = set_bmh_search(sd);

        while ((bmd.data_length = fs_read(sd->fs_searched, sd->buffer.data, sd->buffer.size)))
        {
            if (!sd->bmh_search(&bmd))
            {
                ret_val = 0;
                fprintf(sd->out_p, "%s\n", sd->fs_searched->f_path);
                break;
            }
        }
    }

    return ret_val;
}

int bm_count_search(bm_search_data *sd)
{
    int ret_val = 1;
    size_t count = 0;
    ls_init(sd);
    bm_data bmd = set_bmh_search(sd);

    while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmd) != -1)
    {
        if ((!sd->bmh_search(&bmd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            count++;
        }
    }

    fprintf(sd->out_p, "%s:%ld\n", sd->fs_searched->f_path, count);

    return ret_val;
}

int bm_line_number_search(bm_search_data *sd)
{
    int ret_val = 1;
    size_t line = 0;
    ls_init(sd);
    bm_data bmd = set_bmh_search(sd);

    while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmd) != -1)
    {
        line++;

        if ((!sd->bmh_search(&bmd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            fprintf(sd->out_p, "%s:%ld:", sd->fs_searched->f_path, line);
            fwrite(sd->ls_searched->line, 1, sd->ls_searched->line_length, sd->out_p);
        }
    }

    return ret_val;
}

int bm_print_search(bm_search_data *sd)
{
    int ret_val = 1;
    ls_init(sd);
    bm_data bmd = set_bmh_search(sd);

    while (read_line(sd->fs_searched->f_path, sd->ls_searched, &bmd) != -1)
    {
        if ((!sd->bmh_search(&bmd)) ^ FLAG_SET(sd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            fprintf(sd->out_p, "%s:", sd->fs_searched->f_path);
            fwrite(sd->ls_searched->line, 1, sd->ls_searched->line_length, sd->out_p);
        }
    }

    return ret_val;
}
