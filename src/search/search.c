#include "search.h"

static inline char_buffer buffer_alloc(cli_args *args);
static inline FILE *set_out_path(cli_args *args);
static inline bm_search_data set_bm_search_data(cli_args *args);
static inline int (*set_search_function(unsigned int flags))(bm_search_data *sd);
static inline int set_rdir_stream(rdir_stream **rds, const char *dir_name);
static inline int read_dir(rdir_stream *rds);
static inline void free_bm_search_data(bm_search_data *sd);

static inline int pattern_start_file_search(cli_args *args);
static inline int pattern_start_rec_search(cli_args *args);

int start_pattern_search(cli_args *args);
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
static inline bm_search_data set_bm_search_data(cli_args *args)
{
    bm_search_data sd = {
        .ls_searched = NULL,
        .buffer = buffer_alloc(args),
        .flags = args->flags,
        .out_p = set_out_path(args),
        .bmh_search = FLAG_SET(args->flags, FLAG_WORD) ? &bm_find_w : bm_find,
    };

    sd.fs_searched = fs_init(NULL);
    if (!sd.fs_searched)
        log_error("Error allocating file stream!");

    sd.pattern_length = strlen(args->pattern);
    sd.pattern = (char *)malloc(sizeof(char) * (sd.pattern_length + 1));
    if (!sd.pattern)
        log_error("Error allocating memory!");

    strcpy(sd.pattern, args->pattern);

    if (FLAG_SET(sd.flags, FLAG_IGNORE_CASE))
    {

        for (size_t i = 0; i < sd.pattern_length; i++)
            sd.pattern[i] = (char)tolower((unsigned char)sd.pattern[i]);
    }

    sd.bad_char_table = bm_bad_char_table(sd.pattern, (uint8_t)sd.pattern_length);
    if (!sd.bad_char_table)
        log_error("Error creating bm table!");

    // if (sd.pattern_length > 16)
    // {
    //     sd.good_suffix_table = bm_good_suffix_table(sd.pattern, (uint8_t)sd.pattern_length);
    //     if (!sd.good_suffix_table)
    //         log_error("Error creating bm table");
    // }
    // else
    sd.good_suffix_table = NULL;

    return sd;
}

// Returns search function based on flags
static inline int (*set_search_function(unsigned int flags))(bm_search_data *sd)
{
    if (FLAG_SET(flags, FLAG_LIST))
        return &bm_list_search;
    else if (FLAG_SET(flags, FLAG_COUNT))
        return &bm_count_search;
    else if (FLAG_SET(flags, FLAG_LINE_NUMBER))
        return &bm_line_number_search;
    else
        return &bm_print_search;
}

// Sets directory used in rdir stream
static inline int set_rdir_stream(rdir_stream **rds, const char *dir_name)
{
    if (!(*rds))
    {
        int err;
        *rds = rds_init(dir_name, &err);
        if (!rds)
            log_error("Error allocating rdir stream!");

        if (err)
        {
            log_errno(0, dir_name);
            return 1;
        }
    }
    else
    {
        if (rds_change_dir(*rds, dir_name))
        {
            log_errno(0, dir_name);
            return 1;
        }
    }

    return 0;
}

// Reads entry recursively from directory
static inline int read_dir(rdir_stream *rds)
{
    int read_val = rds_read(rds);
    if (read_val)
    {
        if (read_val != END_OF_DIRECTORY)
            log_info("Error reading in %s", rds->entry_path);
    }

    return read_val;
}

static inline void free_bm_search_data(bm_search_data *sd)
{
    if (sd->out_p != stdout && sd->out_p != stderr)
        fclose(sd->out_p);

    if (sd->ls_searched)
        ls_end(sd->ls_searched);

    fs_end(sd->fs_searched);
    free(sd->bad_char_table);
    free(sd->good_suffix_table);
    free(sd->buffer.data);
    free(sd->pattern);
}

static inline int pattern_start_file_search(cli_args *args)
{
    int ret_val = 1;
    bm_search_data sd = set_bm_search_data(args);

    // Quiet search handled separately to handle early end
    if (FLAG_SET(sd.flags, FLAG_QUIET))
    {
        for (char **current = args->files; current < args->files + args->file_count; current++)
        {
            if (fs_open_file(sd.fs_searched, *(current)))
                continue;

            if (!bm_quiet_search(&sd))
            {
                ret_val = 0;
                break;
            }
        }
    }
    else
    {
        int (*search_function)(bm_search_data *sd) = set_search_function(sd.flags);

        for (char **current = args->files; current < args->files + args->file_count; current++)
        {
            if (fs_open_file(sd.fs_searched, *(current)))
                continue;

            if (!search_function(&sd))
                ret_val = 0;
        }
    }

    free_bm_search_data(&sd);
    return ret_val;
}

static inline int pattern_start_rec_search(cli_args *args)
{
    int ret_val = 1;
    struct stat file_stat;
    rdir_stream *rds = NULL;
    bm_search_data sd = set_bm_search_data(args);

    // Quiet search handled separately to handle early end
    if (FLAG_SET(sd.flags, FLAG_QUIET))
    {
        for (char **current = args->files; current < args->files + args->file_count; current++)
        {
            if (stat(*(current), &file_stat))
            {
                log_errno(0, *(current));
                continue;
            }

            if (S_ISDIR(file_stat.st_mode))
            {
                if (set_rdir_stream(&rds, *(current)))
                    continue;

                while (!read_dir(rds))
                {
                    if (fs_open_file(sd.fs_searched, rds->entry_path))
                        continue;

                    if (!bm_quiet_search(&sd))
                    {
                        ret_val = 0;
                        break;
                    }
                }
            }
            else
            {
                if (fs_open_file(sd.fs_searched, *(current)))
                    continue;

                if (!bm_quiet_search(&sd))
                {
                    ret_val = 0;
                    break;
                }
            }
        }
    }
    else
    {
        int (*search_function)(bm_search_data *sd) = set_search_function(sd.flags);

        for (char **current = args->files; current < args->files + args->file_count; current++)
        {
            if (stat(*(current), &file_stat))
            {
                log_errno(0, *(current));
                continue;
            }

            if (S_ISDIR(file_stat.st_mode))
            {
                if (set_rdir_stream(&rds, *(current)))
                    continue;

                while (!read_dir(rds))
                {
                    if (fs_open_file(sd.fs_searched, rds->entry_path))
                        continue;

                    if (!search_function(&sd))
                        ret_val = 0;
                }
            }
            else
            {
                if (fs_open_file(sd.fs_searched, *(current)))
                    continue;

                if (!search_function(&sd))
                    ret_val = 0;
            }
        }
    }

    if (rds)
        rds_end(rds);

    free_bm_search_data(&sd);
    return ret_val;
}

int start_pattern_search(cli_args *args)
{
    if (FLAG_SET(args->flags, FLAG_RECURSIVE))
        return pattern_start_rec_search(args);
    else
        return pattern_start_file_search(args);
}

int start_file_search(cli_args *args)
{
    return -1;
}