#include "file_stream.h"

static inline int ds_open(dir_stream *ds, const char *dir_name);
static inline int ds_openat(dir_stream *ds, DIR *dp, const char *dir_name);

dir_stream *ds_init(const char *dir_name, DIR *dp_at, int *err);
int ds_open_dir(dir_stream *ds, DIR *dp_at, const char *dir_name);
int ds_close_dir(dir_stream *ds);
int ds_read(dir_stream *ds);
int rds_change_dir(rdir_stream *rds, const char *dir_name);
int ds_end(dir_stream *ds);

static inline int ds_open(dir_stream *ds, const char *dir_name)
{
    errno = 0;
    ds->dp = opendir(dir_name);
    if (!ds->dp)
    {
        log_info("Error openning directory: %s", dir_name);
        if (errno)
            log_errno(0, dir_name);

        return errno ? errno : 1;
    }

    ds->dir_name = dir_name;
    ds->dir_name_length = strlen(dir_name);
    if (ds->dir_name[ds->dir_name_length - 1] == '/' ||
        ds->dir_name[ds->dir_name_length - 1] == '\\')
        ds->dir_name_length--;

    return 0;
}

static inline int ds_openat(dir_stream *ds, DIR *dp, const char *dir_name)
{
    errno = 0;
    int dp_fd = dirfd(dp);
    int ds_fd = openat(dp_fd, dir_name, O_RDONLY);
    if (ds_fd == -1)
    {
        log_errno(0, dir_name);
        return 1;
    }

    ds->dp = fdopendir(ds_fd);
    if (!ds->dp)
    {
        log_info("Error openning directory: %s", dir_name);
        if (errno)
            log_errno(0, dir_name);

        return errno ? errno : 1;
    }

    ds->dir_name = dir_name;
    ds->dir_name_length = strlen(dir_name);
    if (ds->dir_name[ds->dir_name_length - 1] == '/' ||
        ds->dir_name[ds->dir_name_length - 1] == '\\')
        ds->dir_name_length--;

    return 0;
}

dir_stream *ds_init(const char *dir_name, DIR *dp_at, int *err)
{
    dir_stream *ds = (dir_stream *)malloc(sizeof(dir_stream));
    if (!ds)
    {
        log_info("Error allocating memory!");
        return NULL;
    }
    ds->dir_name = NULL;
    ds->dp = NULL;

    if (dir_name)
        *err = ds_open_dir(ds, dp_at, dir_name);

    return ds;
}

int ds_open_dir(dir_stream *ds, DIR *dp_at, const char *dir_name)
{
    if (ds->dp)
    {
        if (ds_close_dir(ds))
            return errno ? errno : 1;
    }

    return dp_at ? ds_openat(ds, dp_at, dir_name) : ds_open(ds, dir_name);
}

int ds_close_dir(dir_stream *ds)
{
    if (!ds->dp)
        return 0;

    errno = 0;
    if (closedir(ds->dp))
    {
        log_info("Error closing directory: %s", ds->dir_name);
        if (errno)
            log_errno(0, ds->dir_name);

        return errno ? errno : 1;
    }

    ds->dir_name = NULL;
    ds->dp = NULL;
    return 0;
}

int ds_read(dir_stream *ds)
{
    errno = 0;
    ds->entry = readdir(ds->dp);
    if (!ds->entry)
    {
        if (errno)
            log_errno(0, ds->dir_name);

        return errno ? errno : END_OF_DIRECTORY;
    }

    return 0;
}

int ds_end(dir_stream *ds)
{
    if (!ds)
        return 0;

    if (ds_close_dir(ds))
        return 1;

    free(ds);
    return 0;
}

//---------------------------------
//----RDIR STREAM FUNCTIONS--------
//---------------------------------

static inline int rds_open(rdir_stream *rds, const char *dir_name);
static inline int rds_close(rdir_stream *rds);

rdir_stream *rds_init(const char *dir_name, int *err);
int rds_read(rdir_stream *rds);
int rds_end(rdir_stream *rds);

static inline int rds_open(rdir_stream *rds, const char *dir_name)
{
    // Expands stack if needed
    if (rds->rdsi.ds_stack_top >= rds->rdsi.ds_stack + rds->rdsi.ds_stack_size)
    {
        int new_size = rds->rdsi.ds_stack_size * 2;
        dir_stream **new_stack = realloc(rds->rdsi.ds_stack, sizeof(dir_stream *) * new_size);
        if (!new_stack)
            return NO_MEM_FOR_STACK;

        for (int i = rds->rdsi.ds_stack_size; i < new_size; i++)
        {
            new_stack[i] = NULL;
        }

        rds->rdsi.ds_stack = new_stack;
        rds->rdsi.ds_stack_top = rds->rdsi.ds_stack + rds->rdsi.ds_stack_size;
        rds->rdsi.ds_stack_size = new_size;
    }

    // Puts directory in dir_stream
    DIR *prev_dp = NULL;
    if (rds->rdsi.ds_stack_top - 1 >= rds->rdsi.ds_stack)
        prev_dp = (*(rds->rdsi.ds_stack_top - 1))->dp;
    dir_stream *ds;
    int err;
    if (*(rds->rdsi.ds_stack_top))
    {
        ds = *rds->rdsi.ds_stack_top;
        err = ds_open_dir(ds, prev_dp, dir_name);
    }
    else
    {
        ds = ds_init(dir_name, prev_dp, &err);
        *rds->rdsi.ds_stack_top = ds;
    }

    if (err)
        return err;

    rds->rdsi.ds_stack_top++;
    return 0;
}

static inline int rds_close(rdir_stream *rds)
{
    if (rds->rdsi.ds_stack_top == rds->rdsi.ds_stack)
        return 0;

    dir_stream *ds = *(rds->rdsi.ds_stack_top - 1);

    int err = ds_close_dir(ds);
    if (err)
        return err;

    rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length - (ds->dir_name_length + 1)] = '\0';
    rds->rdsi.entry_path_length -= ds->dir_name_length + 1;
    rds->rdsi.ds_stack_top--;

    return 0;
}

rdir_stream *rds_init(const char *dir_name, int *err)
{
    rdir_stream *rds = (rdir_stream *)malloc(sizeof(rdir_stream));
    rds->rdsi.entry_path_buffer_size = DEFAULT_PATH_SIZE;
    rds->rdsi.entry_path_buffer = (char *)malloc(sizeof(char) * DEFAULT_PATH_SIZE);
    rds->rdsi.entry_path_length = 0;
    if (!rds->rdsi.entry_path_buffer)
    {
        log_info("Error allocating memory");
        free(rds);
        return NULL;
    }

    rds->rdsi.ds_stack_size = DEFAULT_STACK_SIZE;
    rds->rdsi.ds_stack = (dir_stream **)malloc(sizeof(dir_stream *) * DEFAULT_STACK_SIZE);
    if (!rds->rdsi.ds_stack)
    {
        log_info("Error allocating memory");
        free(rds->rdsi.entry_path_buffer);
        free(rds);
        return NULL;
    }

    rds->rdsi.ds_stack_top = rds->rdsi.ds_stack;
    for (int i = 0; i < DEFAULT_STACK_SIZE; i++)
    {
        rds->rdsi.ds_stack[i] = NULL;
    }

    *err = rds_open(rds, dir_name);
    if (*err)
        return rds;

    int dir_name_length = strlen(dir_name);
    strcpy(rds->rdsi.entry_path_buffer, dir_name);
    rds->rdsi.entry_path_length = dir_name_length;
    if (dir_name[dir_name_length - 1] != '/' && dir_name[dir_name_length - 1] != '\\')
    {
        rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length] = '/';
        rds->rdsi.entry_path_length++;
        rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length] = '\0';
    }

    return rds;
}

int rds_read(rdir_stream *rds)
{
    if (rds->rdsi.ds_stack_top == rds->rdsi.ds_stack)
        return END_OF_DIRECTORY;

    dir_stream *ds = *(rds->rdsi.ds_stack_top - 1);
    int err;

    do
    {
        while ((err = ds_read(ds)))
        {
            if (err != END_OF_DIRECTORY)
                return err;

            int err = rds_close(rds);
            if (err)
                return err;

            if (rds->rdsi.ds_stack_top == rds->rdsi.ds_stack)
                return END_OF_DIRECTORY;

            ds = *(rds->rdsi.ds_stack_top - 1);
        }
    } while (!strcmp(ds->entry->d_name, ".") || !strcmp(ds->entry->d_name, ".."));

    int entry_name_length = strlen(ds->entry->d_name);
    if (rds->rdsi.entry_path_buffer_size <= rds->rdsi.entry_path_length + entry_name_length + 1)
    {
        int new_size = rds->rdsi.entry_path_buffer_size * 2;
        char *new_buffer = realloc(rds->rdsi.entry_path_buffer, new_size);
        if (!new_buffer)
            return NO_MEM_FOR_PATH;

        rds->rdsi.entry_path_buffer = new_buffer;
        rds->rdsi.entry_path_buffer_size = new_size;
    }

    strcpy(rds->rdsi.entry_path_buffer + rds->rdsi.entry_path_length, ds->entry->d_name);
    stat(rds->rdsi.entry_path_buffer, &rds->entry_stat);
    if (S_ISDIR(rds->entry_stat.st_mode))
    {
        int dir_name_length = strlen(ds->entry->d_name);
        rds->rdsi.entry_path_length += dir_name_length;
        rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length] = '/';
        rds->rdsi.entry_path_length++;
        rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length] = '\0';

        err = rds_open(rds, ds->entry->d_name);
        if (err)
            return 1;

        return rds_read(rds);
    }

    rds->entry = ds->entry;
    rds->entry_path = rds->rdsi.entry_path_buffer;

    return 0;
}

int rds_change_dir(rdir_stream *rds, const char *dir_name)
{
    while (rds->rdsi.ds_stack_top > rds->rdsi.ds_stack)
    {
        if (rds_close(rds))
            return 1;
    }

    rds->entry_path = NULL;
    rds->entry = NULL;

    if (rds_open(rds, dir_name))
        return 1;

    int dir_name_length = strlen(dir_name);
    strcpy(rds->rdsi.entry_path_buffer, dir_name);
    rds->rdsi.entry_path_length = dir_name_length;
    if (dir_name[dir_name_length - 1] != '/' && dir_name[dir_name_length - 1] != '\\')
    {
        rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length] = '/';
        rds->rdsi.entry_path_length++;
        rds->rdsi.entry_path_buffer[rds->rdsi.entry_path_length] = '\0';
    }

    return 0;
}

int rds_end(rdir_stream *rds)
{
    for (int i = 0; i < rds->rdsi.ds_stack_size; i++)
    {
        if (ds_end(rds->rdsi.ds_stack[i]))
            return 1;
    }

    free(rds->rdsi.ds_stack);
    free(rds->rdsi.entry_path_buffer);
    free(rds);

    return 0;
}
