#include <efs_search.h>

typedef int (*search_function)(void *data, const line *l);
int bm_find(void *data, const line *l);
int bm_find_w(void *data, const line *l);

typedef struct algo
{
    int invert;
    void *data;
    search_function function;
} algo;

algo *algo_init(const pattern_set *patterns, unsigned int flags);
int algo_search(const algo *a, const line *l);
void algo_end(algo *a);

typedef size_t (*bm_search_function)(void *data, const line *l);
static size_t bmh_search(void *data, const line *l);
static size_t bmh_search_c(void *data, const line *l);

typedef size_t bm_table[256];

typedef struct bm_algo
{
    const char *pattern;
    size_t pattern_length;
    bm_table bad_char;
    bm_table good_suffix;
    bm_search_function function;
    unsigned int flags;
} bm_algo;

bm_algo *bm_init(const char *pattern, size_t pattern_length, int ignore_case);
void bm_end(bm_algo *data) { free(data); }

static size_t bmh_search(void *data, const line *l)
{
    bm_algo *bm_data = (bm_algo *)data;

    if ((size_t)l->length < bm_data->pattern_length)
        return NOT_FOUND;

    size_t loc = 0;

    while (loc <= (size_t)l->length - bm_data->pattern_length)
    {
        size_t i = 0;

        for (; i < bm_data->pattern_length; i++)
        {
            if (l->data[loc + i] != bm_data->pattern[i])
                break;
        }

        if (i == bm_data->pattern_length)
            return loc;

        loc += bm_data->bad_char[(unsigned char)l->data[loc + bm_data->pattern_length - 1]];
    }

    return NOT_FOUND;
}

static size_t bmh_search_c(void *data, const line *l)
{
    bm_algo *bm_data = data;

    if ((size_t)l->length < bm_data->pattern_length)
        return NOT_FOUND;

    size_t loc = 0;

    while (loc <= (size_t)l->length - bm_data->pattern_length)
    {
        size_t i = 0;

        for (; i < bm_data->pattern_length; i++)
        {
            if (tolower(l->data[loc + i]) != bm_data->pattern[i])
                break;
        }

        if (i == bm_data->pattern_length)
            return loc;

        loc += bm_data->bad_char[tolower(l->data[loc + bm_data->pattern_length - 1])];
    }

    return NOT_FOUND;
}

bm_algo *bm_init(const char *pattern, size_t pattern_length, int ignore_case)
{
    bm_algo *data = malloc(sizeof(bm_algo));
    if (!data)
        return NULL;

    data->pattern = pattern;
    data->pattern_length = pattern_length;
    data->function = ignore_case ? bmh_search_c : bmh_search;

    for (int i = 0; i < 256; i++)
        data->bad_char[i] = pattern_length;

    for (size_t i = 0; i < pattern_length - 1; i++)
        data->bad_char[(unsigned char)pattern[i]] = pattern_length - 1 - i;

    return data;
}

int bm_find(void *data, const line *l)
{
    bm_algo *bm_data = data;
    return bm_data->function(data, l) == NOT_FOUND ? 1 : 0;
}

int bm_find_w(void *data, const line *l)
{
    bm_algo *bm_data = data;
    size_t loc = 0;
    size_t bm_loc;

    while (loc < (size_t)l->length - bm_data->pattern_length)
    {
        bm_loc = bm_data->function(data, &(const line){l->data + loc, l->length - loc});
        if (bm_loc == NOT_FOUND)
            return 1;

        size_t i = loc + bm_loc;
        if ((!i || !is_word_char(l->data[i - 1])) &&
            (i + bm_data->pattern_length >= (size_t)l->length || !is_word_char(l->data[i + bm_data->pattern_length])))
            return 0;

        loc = i + bm_data->pattern_length;
    }

    return 1;
}

algo *algo_init(const pattern_set *ps, unsigned int flags)
{
    if (ps->count != 1)
        return NULL;

    algo *a = malloc(sizeof(algo));
    if (!a)
        return NULL;

    a->invert = FLAG_SET(flags, FLAG_INVERT);
    a->function = FLAG_SET(flags, FLAG_WORD) ? bm_find_w : bm_find;
    a->data = bm_init(ps->patterns[0], ps->lengths[0], FLAG_SET(flags, FLAG_IGNORE_CASE));
    if (!a->data)
    {
        free(a);
        return NULL;
    }

    return a;
}

int algo_search(const algo *a, const line *l)
{
    return (!!a->function(a->data, l) ^ a->invert);
}

void algo_end(algo *a)
{
    bm_end(a->data);
    free(a);
}
