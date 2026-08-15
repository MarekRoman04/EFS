#include <efs_search.h>

typedef struct pattern_set_internal
{
    const char *pattern;
    size_t length;
    pattern_set ps;
} pattern_set_internal;

typedef struct search_context
{
    cli_args *args;
    file_stream *fs;
    line_stream *ls;
    rdir_stream *rds;
    pattern_set_internal patterns;
    algo *a;
} search_context;

static int read_line(search_context *ctx, line *l);
static int init_fs(search_context *ctx);
static int init_ls(search_context *ctx);
static int load_file_patterns(search_context *ctx);
static void free_patterns(pattern_set_internal *ps);
static int load_pattern(search_context *ctx);
static int open_file(search_context *ctx);
static int get_file_rec(search_context *ctx);
static int get_file(search_context *ctx);

typedef int (*search_function)(search_context *);
static int search_q(search_context *ctx);
static int search_l(search_context *ctx);
static int search_c(search_context *ctx);
static int search_n(search_context *ctx);
static int search_default(search_context *ctx);
static search_function get_search_function(unsigned int flag);

static int read_line(search_context *ctx, line *l)
{
    *l = ls_read(ctx->ls);
    if (!l->data)
    {
        if (l->length != 0)
            log_error("Error reading line in %s\n", fs_get_path(ctx->fs));

        return 1;
    }

    return 0;
}

static int init_fs(search_context *ctx)
{
    int err;

    ctx->fs = fs_init(NULL, O_RDONLY, &err);
    if (!ctx->fs)
    {
        log_error("Error initializing file stream!\n", NULL);
        return 1;
    }

    return 0;
}

static int init_ls(search_context *ctx)
{
    ctx->ls = ls_init_from_fs(ctx->fs);
    if (!ctx->ls)
    {
        log_error("Error initializing line stream!\n", NULL);
        return 1;
    }

    return 0;
}

static int init_rds(search_context *ctx)
{
    ctx->rds = rds_init(NULL, NULL);
    if (!ctx->rds)
    {
        log_error("Error initializing directory stream!\n", NULL);
        return 1;
    }

    return 0;
}

static int init_algo(search_context *ctx)
{
    ctx->a = algo_init(&ctx->patterns.ps, ctx->args->flags);
    if (!ctx->a)
    {
        log_error("Error initializing search algorithm!\n", NULL);
        return 1;
    }

    return 0;
}

void pattern_to_lower(char *pattern, size_t pattern_length)
{
    for (size_t i = 0; i < pattern_length; i++)
        pattern[i] = tolower(pattern[i]);
}

static int load_file_patterns(search_context *ctx)
{
    if (fs_open_file(ctx->fs, ctx->args->pattern))
    {
        log_error("Error opening pattern file: %s!\n", ctx->args->pattern);
        return 1;
    }

    h_set *pattern_set = h_set_init(0);
    if (!pattern_set)
    {
        log_error("Error initialzing hash set!\n", NULL);
        return 1;
    }

    line l;
    while (!read_line(ctx, &l))
    {
        if (!l.length)
            continue;

        if (h_set_insert(pattern_set, l.data, l.length) == -1)
        {
            log_error("Error loading pattern!\n", NULL);
            h_set_end(pattern_set);

            return 1;
        }
    }

    if (!l.data && l.length)
    {
        h_set_end(pattern_set);
        return 1;
    }

    ctx->patterns.ps.patterns = (const char **)h_set_move_end(pattern_set, &ctx->patterns.ps.lengths, &ctx->patterns.ps.count);
    if (!ctx->patterns.ps.count)
    {
        log_error("Error no patterns found!\n", NULL);
        return 1;
    }

    if (FLAG_SET(ctx->args->flags, FLAG_IGNORE_CASE))
    {
        for (size_t i = 0; i < ctx->patterns.ps.count; i++)
            pattern_to_lower((char *)ctx->patterns.ps.patterns[i], ctx->patterns.ps.lengths[i]);
    }

    return 0;
}

static int load_pattern(search_context *ctx)
{
    ctx->patterns.pattern = ctx->args->pattern;
    ctx->patterns.length = strlen(ctx->args->pattern);

    ctx->patterns.ps.patterns = &ctx->patterns.pattern;
    ctx->patterns.ps.lengths = &ctx->patterns.length;
    ctx->patterns.ps.count = 1;

    if (FLAG_SET(ctx->args->flags, FLAG_IGNORE_CASE))
        pattern_to_lower((char *)ctx->patterns.pattern, ctx->patterns.length);

    return 0;
}

static void free_patterns(pattern_set_internal *ps)
{
    if (ps->pattern || !ps->ps.patterns)
        return;

    for (size_t i = 0; i < ps->ps.count; i++)
        free((char *)ps->ps.patterns[i]);

    free(ps->ps.patterns);
    free(ps->ps.lengths);
}

static int open_file(search_context *ctx)
{
    int rv = fs_open_file(ctx->fs, *ctx->args->files);
    if (rv)
        log_error("Error %d opening file: %s\n", rv, *ctx->args->files);

    ctx->args->files++;
    ctx->args->file_count--;

    return rv;
}

static int get_file_rec(search_context *ctx)
{
    while (1)
    {
        const char *path;
        int err;

        while ((path = rds_read_entry_name(ctx->rds, &err)))
        {
            if (!fs_open_file(ctx->fs, path))
                return 0;

            log_error("Error opening file: %s\n", path);
        }

        if (err)
            log_error("Error reading in: %s\n", rds_current_dir(ctx->rds));

        if (ctx->args->file_count > 0)
        {
            int err = rds_change_dir(ctx->rds, *ctx->args->files);
            if (err == FILE_NOT_DIR)
            {
                if (!open_file(ctx))
                    return 0;

                continue;
            }
            if (err)
                log_error("Error opening directory: %s\n", *ctx->args->files);

            ctx->args->files++;
            ctx->args->file_count--;
            continue;
        }

        return 1;
    }
}

static int get_file(search_context *ctx)
{
    if (ctx->rds)
        return get_file_rec(ctx);

    while (ctx->args->file_count > 0)
    {
        if (!open_file(ctx))
            return 0;
    }

    return 1;
}

static int search_q(search_context *ctx)
{
    line l;
    while (!read_line(ctx, &l))
    {
        if (!algo_search(ctx->a, &l))
        {
            fs_end(ctx->fs);
            ls_end(ctx->ls);
            rds_end(ctx->rds);
            algo_end(ctx->a);

            exit(EXIT_SUCCESS);
        }
    }

    return 1;
}

static int search_l(search_context *ctx)
{
    line l;
    while (!read_line(ctx, &l))
    {
        if (!algo_search(ctx->a, &l))
        {
            printf("%s\n", fs_get_path(ctx->fs));
            return 0;
        }
    }

    return 1;
}

static int search_c(search_context *ctx)
{
    int rv = 1;
    line l;
    size_t count = 0;

    while (!read_line(ctx, &l))
    {
        if (!algo_search(ctx->a, &l))
        {
            rv = 0;
            count++;
        }
    }

    printf("%s:%ld\n", fs_get_path(ctx->fs), count);

    return rv;
}

static int search_n(search_context *ctx)
{
    int rv = 1;
    line l;
    size_t line_num = 0;

    while (!read_line(ctx, &l))
    {
        line_num++;

        if (!algo_search(ctx->a, &l))
        {
            rv = 0;
            printf("%s:%ld:%s\n", fs_get_path(ctx->fs), line_num, l.data);
        }
    }

    return rv;
}

static int search_default(search_context *ctx)
{
    int rv = 1;
    line l;

    while (!read_line(ctx, &l))
    {
        if (!algo_search(ctx->a, &l))
        {
            rv = 0;
            printf("%s:%s\n", fs_get_path(ctx->fs), l.data);
        }
    }

    return rv;
}

static search_function get_search_function(unsigned int flags)
{
    if (FLAG_SET(flags, FLAG_QUIET))
        return search_q;
    if (FLAG_SET(flags, FLAG_LIST))
        return search_l;
    if (FLAG_SET(flags, FLAG_COUNT))
        return search_c;
    if (FLAG_SET(flags, FLAG_LINE_NUMBER))
        return search_n;

    return search_default;
}

int search(cli_args *args)
{
    int rv = 1;
    search_context ctx = {.args = args};
    search_function s_fn = get_search_function(args->flags);

    if (init_fs(&ctx) || init_ls(&ctx))
        goto _err;

    if (FLAG_SET(args->flags, FLAG_RECURSIVE) && init_rds(&ctx))
        goto _err;

    if (FLAG_SET(args->flags, FLAG_FILE) ? load_file_patterns(&ctx) : load_pattern(&ctx))
        goto _err;

    if (init_algo(&ctx))
        goto _err;

    while (!get_file(&ctx))
        rv = s_fn(&ctx);

_err:
    fs_end(ctx.fs);
    ls_end(ctx.ls);
    rds_end(ctx.rds);
    algo_end(ctx.a);
    free_patterns(&ctx.patterns);

    return rv;
}
