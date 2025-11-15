#include <search.h>

int rk_count_search(rk_search_data *rsd);
int rk_line_number_search(rk_search_data *rsd);
int rk_print_search(rk_search_data *rsd);

int rk_count_search(rk_search_data *rsd)
{
    int ret_val = 1;
    size_t count = 0;
    ls_change_file(rsd->ls_searched, rsd->fs_searched->fp);

    int read_val;
    while (!(read_val = ls_read(rsd->ls_searched)))
    {
        rk_rehash_data(rsd->rks, rsd->ls_searched->line, rsd->ls_searched->line_length);

        if (!rsd->rk_search_function(rsd->rks, rsd->ls_searched->line, rsd->ls_searched->line_length) ^ FLAG_SET(rsd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            count++;
        }
    }

    if (read_val == 1)
        log_info("Error reading line in %s", rsd->fs_searched->f_path);

    fprintf(rsd->out_p, "%s:%ld\n", rsd->fs_searched->f_path, count);

    return ret_val;
}

int rk_line_number_search(rk_search_data *rsd)
{
    int ret_val = 1;
    size_t line = 0;
    ls_change_file(rsd->ls_searched, rsd->fs_searched->fp);

    int read_val;
    while (!(read_val = ls_read(rsd->ls_searched)))
    {
        line++;
        rk_rehash_data(rsd->rks, rsd->ls_searched->line, rsd->ls_searched->line_length);

        if (!rsd->rk_search_function(rsd->rks, rsd->ls_searched->line, rsd->ls_searched->line_length) ^ FLAG_SET(rsd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            fprintf(rsd->out_p, "%s:%ld:", rsd->fs_searched->f_path, line);
            fwrite(rsd->ls_searched->line, 1, rsd->ls_searched->line_length, rsd->out_p);
        }
    }

    if (read_val == 1)
        log_info("Error reading line in %s", rsd->fs_searched->f_path);

    return ret_val;
}

int rk_print_search(rk_search_data *rsd)
{
    int ret_val = 1;
    ls_change_file(rsd->ls_searched, rsd->fs_searched->fp);

    int read_val;
    while (!(read_val = ls_read(rsd->ls_searched)))
    {
        rk_rehash_data(rsd->rks, rsd->ls_searched->line, rsd->ls_searched->line_length);

        if (!rsd->rk_search_function(rsd->rks, rsd->ls_searched->line, rsd->ls_searched->line_length) ^ FLAG_SET(rsd->flags, FLAG_INVERT))
        {
            ret_val = 0;
            fprintf(rsd->out_p, "%s:", rsd->fs_searched->f_path);
            fwrite(rsd->ls_searched->line, 1, rsd->ls_searched->line_length, rsd->out_p);
        }
    }

    if (read_val == 1)
        log_info("Error reading line in %s", rsd->fs_searched->f_path);

    return ret_val;
}