#include "search.h"

static inline char_buffer buffer_alloc(cli_args *args);
static inline FILE *set_out_path(cli_args *args);
static inline int set_rdir_stream(rdir_stream **rds, const char *dir_name);
static inline int read_dir(rdir_stream *rds);

static inline bm_search_data bm_set_search_data(cli_args *args);
static inline int (*bm_set_search_function(unsigned int flags))(bm_search_data *sd);
static inline void free_bm_search_data(bm_search_data *sd);

static inline rk_search_data rk_set_search_data(cli_args *args);
static inline rk_patterns rk_search_data_get_patterns(line_stream *ls, size_t max_length, int flags);
static inline int (*rk_set_search_function(unsigned int flags))(rk_search_data *);
static inline void rk_free_search_data(rk_search_data *rks);

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

// Sets search data for buffered search
static inline bm_search_data bm_set_search_data(cli_args *args)
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
    if (sd.pattern_length > 255)
        log_error("Pattern: %s, too long, use file option instead!", args->pattern);

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

// Returns bm search function based on flags
static inline int (*bm_set_search_function(unsigned int flags))(bm_search_data *sd)
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

static inline rk_search_data rk_set_search_data(cli_args *args)
{
    rk_search_data rks = {
        .buffer = buffer_alloc(args),
        .rk_search_function = FLAG_SET(args->flags, FLAG_WORD) ? &rk_find_w : &rk_find,
        .out_p = set_out_path(args),
        .flags = args->flags,
    };

    rks.fs_searched = fs_init(args->pattern);
    if (!rks.fs_searched)
        log_error("Error allocating file stream!");

    if (!rks.fs_searched->fp)
        log_error("Error opening file!");

    rks.ls_searched = ls_init_from_fs(rks.fs_searched, rks.buffer.data, rks.buffer.size);
    if (!rks.ls_searched)
        log_error("Error creating line stream!");

    rks.patterns = rk_search_data_get_patterns(rks.ls_searched, rks.buffer.size, args->flags);

    rk_data rkd = {
        .count = rks.patterns.pattern_count,
        .data = NULL,
        .data_length = 0,
        .patterns = (const char **)rks.patterns.patterns,
        .pattern_lengths = rks.patterns.pattern_lengths,
    };

    rks.rks = rk_search_init(&rkd);
    if (!rks.rks)
        log_error("Error initializing rk search!");

    return rks;
}

static inline rk_patterns rk_search_data_get_patterns(line_stream *ls, size_t max_length, int flags)
{
    rk_patterns rkp = {
        .pattern_count = 0,
        .patterns = malloc(sizeof(char *) * RK_PATTERNS_MIN_SIZE),
        .pattern_lengths = malloc(sizeof(size_t *) * RK_PATTERNS_MIN_SIZE),
        .max_count = RK_PATTERNS_MIN_SIZE,
    };

    if (!rkp.patterns || !rkp.pattern_lengths)
        log_error("Error allocating pattern array!");

    int read_val;
    while (!(read_val = ls_read(ls)))
    {
        if (ls->line_length > max_length)
        {
            log_info("Warning: Pattern length exceeds block size, certain search modes may ignore this pattern!");
            continue;
        }

        if (rkp.pattern_count >= rkp.max_count)
        {
            rkp.max_count *= 2;
            char **new_patterns = realloc(rkp.patterns, sizeof(char *) * rkp.max_count);
            size_t *new_pattern_lengths = realloc(rkp.pattern_lengths, sizeof(size_t *) * rkp.max_count);

            if (!new_patterns || !new_pattern_lengths)
                log_error("Error allocating pattern array!");

            rkp.patterns = new_patterns;
            rkp.pattern_lengths = new_pattern_lengths;
        }

        rkp.patterns[rkp.pattern_count] = malloc(ls->line_length - 1);
        if (!rkp.patterns[rkp.pattern_count])
            log_error("Error allocating pattern!");

        memcpy(rkp.patterns[rkp.pattern_count], ls->line, ls->line_length - 1);
        rkp.pattern_lengths[rkp.pattern_count] = ls->line_length - 1;
        rkp.pattern_count++;
    }

    if (read_val == 1)
        log_error("Error reading patterns!");

    if (FLAG_SET(flags, FLAG_IGNORE_CASE))
    {
        for (size_t i = 0; i < rkp.pattern_count; i++)
        {
            for (size_t j = 0; j < rkp.pattern_lengths[i]; j++)
            {
                rkp.patterns[i][j] = (char)tolower((unsigned char)rkp.patterns[i][j]);
            }
        }
    }

    return rkp;
}

// Returns rk search function based on flags
static inline int (*rk_set_search_function(unsigned int flags))(rk_search_data *)
{
    if (FLAG_SET(flags, FLAG_QUIET))
        return &rk_quiet_search;
    else if (FLAG_SET(flags, FLAG_LIST))
        return &rk_list_search;
    else if (FLAG_SET(flags, FLAG_COUNT))
        return &rk_count_search;
    else if (FLAG_SET(flags, FLAG_LINE_NUMBER))
        return &rk_line_number_search;
    else
        return &rk_print_search;
}

static inline void rk_free_search_data(rk_search_data *rks)
{
    if (rks->out_p != stdout && rks->out_p != stderr)
        fclose(rks->out_p);

    for (size_t i = 0; i < rks->patterns.pattern_count; i++)
    {
        free(rks->patterns.patterns[i]);
    }

    rk_search_end(rks->rks);
    ls_end(rks->ls_searched);
    fs_end(rks->fs_searched);
    free(rks->buffer.data);
    free(rks->patterns.patterns);
    free(rks->patterns.pattern_lengths);
}

static inline int pattern_start_file_search(cli_args *args)
{
    int ret_val = 1;
    bm_search_data sd = bm_set_search_data(args);

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
        int (*search_function)(bm_search_data *sd) = bm_set_search_function(sd.flags);

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
    bm_search_data sd = bm_set_search_data(args);

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
        int (*search_function)(bm_search_data *sd) = bm_set_search_function(sd.flags);

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

static inline int file_start_file_search(cli_args *args)
{
    int ret_val = 1;
    rk_search_data rks = rk_set_search_data(args);

    int (*search_function)(rk_search_data *) = rk_set_search_function(rks.flags);

    for (char **current = args->files; current < args->files + args->file_count; current++)
    {
        if (fs_open_file(rks.fs_searched, *(current)))
            continue;

        if (!search_function(&rks))
            ret_val = 0;
    }

    rk_free_search_data(&rks);
    return ret_val;
}

static inline int file_start_rec_search(cli_args *args)
{
    int ret_val = 1;
    struct stat file_stat;
    rdir_stream *rds = NULL;
    rk_search_data rks = rk_set_search_data(args);

    int (*search_function)(rk_search_data *) = rk_set_search_function(rks.flags);

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
                if (fs_open_file(rks.fs_searched, rds->entry_path))
                    continue;

                if (!search_function(&rks))
                    ret_val = 0;
            }
        }
        else
        {
            if (fs_open_file(rks.fs_searched, *(current)))
                continue;

            if (!search_function(&rks))
                ret_val = 0;
        }
    }

    rk_free_search_data(&rks);
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
    if (FLAG_SET(args->flags, FLAG_RECURSIVE))
        return file_start_rec_search(args);
    else
        return file_start_file_search(args);
}