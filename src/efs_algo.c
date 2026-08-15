#include <efs_search.h>

static inline int is_word_char(char c) { return isalnum((unsigned char)c) || c == '_'; }

/* Algo Declarations*/

typedef int (*search_function)(void *data, const line *l);
static int bm_search(void *data, const line *l);
static int bm_search_w(void *data, const line *l);
static int rk_search(void *data, const line *l);
static int rk_search_w(void *data, const line *l);

typedef struct algo
{
    int invert;
    void *data;
    search_function function;
} algo;

algo *algo_init(const pattern_set *patterns, unsigned int flags);
int algo_search(const algo *a, const line *l) { return (!!a->function(a->data, l) ^ a->invert); }
void algo_end(algo *a);

/* BM Declarations*/

typedef size_t (*bm_search_function)(void *data, const line *l);
static size_t bmh_search(void *data, const line *l);
static size_t bmh_search_i(void *data, const line *l);

typedef size_t bm_table[256];

typedef struct bm_algo
{
    const char *pattern;
    size_t pattern_length;
    bm_table bad_char;
    bm_search_function function;
} bm_algo;

static bm_algo *bm_init(const char *pattern, size_t pattern_length, int ignore_case);
static void bm_end(bm_algo *data) { free(data); }

/* RK Declarations*/

#define RK_BASE 256ULL
#define RK_MOD 1000000007ULL

typedef size_t (*rk_search_function)(void *data, const line *l, size_t *length);
static size_t rk_search_loc(void *data, const line *l, size_t *length);
static size_t rk_search_loc_i(void *data, const line *l, size_t *length);

typedef struct rk_data_hash
{
    uint64_t data_hash;
    uint64_t mod_power;
    size_t data_length;
} rk_data_hash;

typedef struct rk_algo
{
    h_set *pattern_hashes;
    rk_data_hash *data_hashes;
    size_t data_hashes_length;
    rk_search_function function;
} rk_algo;

static uint64_t rk_hash(const char *data, size_t data_length);
static uint64_t rk_hash_i(const char *data, size_t data_length);
static uint64_t rk_base_power(size_t pattern_length);
static uint64_t rk_rolling_hash(uint64_t hash, unsigned char old, unsigned char new, uint64_t base_power);
static h_set *rk_get_pattern_hash(const pattern_set *ps);
static rk_data_hash *rk_init_data_hash(const pattern_set *ps, size_t *length);
static rk_algo *rk_init(const pattern_set *ps, int ignore_case);
static size_t rk_search_loc(void *data, const line *l, size_t *length);
static size_t rk_search_loc_i(void *data, const line *l, size_t *length);
static void rk_end(void *data);

/* BM Definitions */

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

static size_t bmh_search_i(void *data, const line *l)
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

static bm_algo *bm_init(const char *pattern, size_t pattern_length, int ignore_case)
{
    bm_algo *data = malloc(sizeof(bm_algo));
    if (!data)
        return NULL;

    data->pattern = pattern;
    data->pattern_length = pattern_length;
    data->function = ignore_case ? bmh_search_i : bmh_search;

    for (int i = 0; i < 256; i++)
        data->bad_char[i] = pattern_length;

    for (size_t i = 0; i < pattern_length - 1; i++)
        data->bad_char[(unsigned char)pattern[i]] = pattern_length - 1 - i;

    return data;
}

/* RK Definitions */

static inline uint64_t rk_hash(const char *data, size_t data_length)
{
    uint64_t hash = 0;

    for (size_t i = 0; i < data_length; ++i)
    {
        hash = (hash * RK_BASE + (unsigned char)data[i]) % RK_MOD;
    }

    return hash;
}

static inline uint64_t rk_hash_i(const char *data, size_t data_length)
{
    uint64_t hash = 0;

    for (size_t i = 0; i < data_length; ++i)
    {
        hash = (hash * RK_BASE + (unsigned char)tolower(data[i])) % RK_MOD;
    }

    return hash;
}

static uint64_t rk_base_power(size_t pattern_length)
{
    uint64_t power = 1;

    for (size_t i = 1; i < pattern_length; ++i)
    {
        power = (power * RK_BASE) % RK_MOD;
    }

    return power;
}

static uint64_t rk_rolling_hash(uint64_t hash, unsigned char old, unsigned char new, uint64_t base_power)
{
    hash = (hash + RK_MOD - (old * base_power) % RK_MOD) % RK_MOD;
    hash = (hash * RK_BASE + new) % RK_MOD;
    return hash;
}

static h_set *rk_get_pattern_hash(const pattern_set *ps)
{
    h_set *s = h_set_init(ps->count);
    if (!s)
        return NULL;

    for (size_t i = 0; i < ps->count; i++)
    {
        uint64_t hash = rk_hash(ps->patterns[i], ps->lengths[i]);
        if (h_set_insert(s, (char *)&hash, sizeof(uint64_t)) < 0)
        {
            h_set_end(s);
            return NULL;
        }
    }

    return s;
}

static rk_data_hash *rk_init_data_hash(const pattern_set *ps, size_t *length)
{
    *length = 0;
    h_set *s = h_set_init(ps->count);
    if (!s)
        return NULL;

    for (size_t i = 0; i < ps->count; i++)
    {
        int rv = h_set_insert(s, (char *)&ps->lengths[i], sizeof(ps->lengths[0]));
        if (rv == 0)
            (*length)++;
        else if (rv < 0)
        {
            h_set_end(s);
            return NULL;
        }
    }

    rk_data_hash *h = malloc(sizeof(rk_data_hash) * (*length));

    size_t *pattern_length;
    h_set_iterator si = {s, 0};
    for (size_t i = 0; (pattern_length = (size_t *)h_set_iterator_get(&si, NULL)); i++)
    {
        h[i].mod_power = rk_base_power(*pattern_length);
        h[i].data_length = *pattern_length;
    }

    h_set_end(s);
    return h;
}

static rk_algo *rk_init(const pattern_set *ps, int ignore_case)
{
    rk_algo *data = malloc(sizeof(rk_algo));
    if (!data)
        return NULL;

    data->function = ignore_case ? rk_search_loc_i : rk_search_loc;

    if (!(data->pattern_hashes = rk_get_pattern_hash(ps)))
        goto _err;

    if (!(data->data_hashes = rk_init_data_hash(ps, &data->data_hashes_length)))
        goto _err;

    return data;

_err:
    h_set_end(data->pattern_hashes);
    free(data->data_hashes);
    free(data);
    return NULL;
}

static size_t rk_search_loc(void *data, const line *l, size_t *length)
{
    rk_algo *rk_data = data;

    for (size_t i = 1; i < (size_t)l->length + 1; i++)
    {
        for (size_t j = 0; j < rk_data->data_hashes_length; j++)
        {
            rk_data_hash *h = &rk_data->data_hashes[j];
            if (i < h->data_length)
                continue;
            else if (i == h->data_length)
                h->data_hash = rk_hash(l->data, i);
            else
            {
                unsigned char old = l->data[i - h->data_length - 1];
                unsigned char new = l->data[i - 1];
                h->data_hash = rk_rolling_hash(h->data_hash, old, new, h->mod_power);
            }

            if (!h_set_has(rk_data->pattern_hashes, (char *)&h->data_hash, sizeof(h->data_hash)))
            {
                if (length)
                    *length = h->data_length;

                return i - h->data_length;
            }
        }
    }

    return NOT_FOUND;
}

static size_t rk_search_loc_i(void *data, const line *l, size_t *length)
{
    rk_algo *rk_data = data;

    for (size_t i = 1; i < (size_t)l->length + 1; i++)
    {
        for (size_t j = 0; j < rk_data->data_hashes_length; j++)
        {
            rk_data_hash *h = rk_data->data_hashes + j;
            if (i < h->data_length)
                continue;
            else if (i == h->data_length)
                h->data_hash = rk_hash_i(l->data, i);
            else
            {
                unsigned char old = tolower(l->data[i - h->data_length - 1]);
                unsigned char new = tolower(l->data[i - 1]);
                h->data_hash = rk_rolling_hash(h->data_hash, old, new, h->mod_power);
            }

            if (!h_set_has(rk_data->pattern_hashes, (char *)&h->data_hash, sizeof(h->data_hash)))
            {
                if (length)
                    *length = h->data_length;

                return i - h->data_length;
            }
        }
    }

    return NOT_FOUND;
}

static void rk_end(void *data)
{
    rk_algo *rk_data = data;

    h_set_end(rk_data->pattern_hashes);
    free(rk_data->data_hashes);

    free(data);
}

/* Algo Definitions */

static int bm_search(void *data, const line *l)
{
    bm_algo *bm_data = data;
    return bm_data->function(data, l) == NOT_FOUND ? 1 : 0;
}

static int bm_search_w(void *data, const line *l)
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

static int rk_search(void *data, const line *l)
{
    rk_algo *rk_data = data;
    return rk_data->function(data, l, NULL) == NOT_FOUND ? 1 : 0;
}

/* TO-DO FIX */

static int rk_search_w(void *data, const line *l)
{
    rk_algo *rk_data = data;
    size_t loc = 0;
    size_t length;
    size_t rk_loc;

    while (loc < (size_t)l->length)
    {
        rk_loc = rk_data->function(data, l, &length);
        if (rk_loc == NOT_FOUND)
            return 1;

        size_t i = loc + rk_loc;

        if ((!i || !is_word_char(l->data[i - 1])) &&
            (i + length >= (size_t)l->length || !is_word_char(l->data[i + length])))
            return 0;

        loc = i + length;
    }

    return 1;
}

static int algo_init_bm(algo *a, const pattern_set *ps, unsigned int flags)
{
    a->function = FLAG_SET(flags, FLAG_WORD) ? bm_search_w : bm_search;
    a->data = bm_init(ps->patterns[0], ps->lengths[0], FLAG_SET(flags, FLAG_IGNORE_CASE));
    return a->data ? 0 : 1;
}

static int algo_init_rk(algo *a, const pattern_set *ps, unsigned int flags)
{
    a->function = FLAG_SET(flags, FLAG_WORD) ? rk_search_w : rk_search;
    a->data = rk_init(ps, FLAG_SET(flags, FLAG_IGNORE_CASE));
    return a->data ? 0 : 1;
}

algo *algo_init(const pattern_set *ps, unsigned int flags)
{
    algo *a = malloc(sizeof(algo));
    if (!a)
        return NULL;

    a->invert = FLAG_SET(flags, FLAG_INVERT);

    if ((ps->count > 1 ? algo_init_rk(a, ps, flags) : algo_init_bm(a, ps, flags)))
    {
        free(a);
        return NULL;
    }

    return a;
}

void algo_end(algo *a)
{
    if (!a)
        return;

    if (a->function == rk_search || a->function == rk_search_w)
        rk_end(a->data);
    else
        bm_end(a->data);

    free(a);
}
