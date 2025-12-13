#include <algo.h>

static inline h_set *rk_get_pattern_set(const char **patterns, size_t *pattern_lengths, size_t count);
static inline h_set *rk_get_pattern_hashes(const char **patterns, size_t *patern_lengths,
                                           size_t count, h_set_iterator **out_iterator);
static inline rk_data_hash *rk_get_data_hashes(const char *data, size_t data_length, size_t *patern_lengths, size_t count, size_t *out_length);
static inline void rk_free_hashes(h_set_iterator *hsi);
static inline void rk_free_sizes(h_set_iterator *hsi);
static inline uint64_t rk_hash(const char *data, size_t data_length);
static inline uint64_t rk_hash_i(const char *data, size_t data_length);
static inline uint64_t rk_base_power(size_t exp);
static inline uint64_t rk_rolling_hash(uint64_t hash, unsigned char old, unsigned char new, uint64_t base_power);
static inline size_t _rk_search(rk_search *rks, const char *data, size_t data_length, size_t *out_length);
static inline size_t _rk_search_i(rk_search *rks, const char *data, size_t data_length, size_t *out_length);

rk_search *rk_search_init(rk_data *rkd);
int rk_count(rk_search *rks, const char *data, size_t data_length, int ignore_case);
int rk_find(rk_search *rks, const char *data, size_t data_length, int ignore_case);
int rk_find_w(rk_search *rks, const char *data, size_t data_length, int ignore_case);
void rk_search_end(rk_search *rks);

// Creates hash set from paterns
static inline h_set *rk_get_pattern_set(const char **patterns, size_t *pattern_lengths, size_t count)
{
    h_set *hs = h_set_init();
    if (!hs)
    {
        log_info("Error creating hash set!");
        return NULL;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (h_set_add(hs, patterns[i], pattern_lengths[i]) < 0)
        {
            log_info("Error adding element to hash set!");
            h_set_end(hs);
            return NULL;
        }
    }

    return hs;
}

// Creates hash set containing hashed patterns and iterator to it
static inline h_set *rk_get_pattern_hashes(const char **patterns, size_t *patern_lengths,
                                           size_t count, h_set_iterator **out_iterator)
{
    h_set *hs = h_set_init();
    if (!hs)
    {
        log_info("Error creating hash set!");
        return NULL;
    }

    *out_iterator = h_set_iterator_init(hs);
    if (!*out_iterator)
    {
        log_info("Error creating hash set iterator!");
        h_set_end(hs);
        return NULL;
    }

    uint64_t *hash = malloc(sizeof(uint64_t));
    if (!hash)
    {
        log_info("Error allocating memory!");
        h_set_iterator_end(*out_iterator);
        h_set_end(hs);
        return NULL;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (!hash)
        {
            hash = malloc(sizeof(uint64_t));
            if (!hash)
            {
                log_info("Error allocating memory!");
                rk_free_hashes(*out_iterator);
                h_set_iterator_end(*out_iterator);
                h_set_end(hs);
                return NULL;
            }
        }

        *hash = rk_hash(patterns[i], patern_lengths[i]);

        int ret_val = h_set_add(hs, (const char *)hash, sizeof(*hash));
        if (!ret_val)
            hash = NULL;
        else if (ret_val < 0)
        {
            log_info("Error adding element to hash set!");
            rk_free_hashes(*out_iterator);
            h_set_iterator_end(*out_iterator);
            h_set_end(hs);
            return NULL;
        }
    }

    if (hash)
        free(hash);

    h_set_iterator_reset(*out_iterator);
    return hs;
}

// Creates array containing rolling hashes for each pattern length
static inline rk_data_hash *rk_get_data_hashes(const char *data, size_t data_length, size_t *patern_lengths, size_t count, size_t *out_length)
{
    h_set *hs = h_set_init();
    if (!hs)
    {
        log_info("Error creating hash set!");
        return NULL;
    }

    h_set_iterator *hsi = h_set_iterator_init(hs);
    if (!hsi)
    {
        log_info("Error creating hash set iterator!");
        h_set_end(hs);
        return NULL;
    }

    size_t *size = malloc(sizeof(size_t));
    if (!size)
    {
        log_info("Error allocating memory!");
        h_set_iterator_end(hsi);
        h_set_end(hs);
        return NULL;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (!size)
        {
            size = malloc(sizeof(size_t));
            if (!size)
            {
                log_info("Error allocating memory!");
                rk_free_sizes(hsi);
                h_set_iterator_end(hsi);
                h_set_end(hs);
                return NULL;
            }
        }

        *size = patern_lengths[i];

        int ret_val = h_set_add(hs, (const char *)size, sizeof(*size));
        if (!ret_val)
            size = NULL;
        else if (ret_val < 0)
        {
            log_info("Error adding element to hash set!");
            rk_free_sizes(hsi);
            h_set_iterator_end(hsi);
            h_set_end(hs);
            return NULL;
        }
    }

    if (size)
        free(size);

    rk_data_hash *rkd_hashes = malloc(sizeof(rk_data_hash) * hs->length);
    if (!rkd_hashes)
    {
        log_info("Error allocating memory!");
        rk_free_sizes(hsi);
        h_set_iterator_end(hsi);
        h_set_end(hs);
        return NULL;
    }

    size_t i = 0;
    size_t length;
    h_set_iterator_reset(hsi);

    while ((size = (size_t *)h_set_iterator_get(hsi, &length)))
    {
        if (*size < data_length)
            rkd_hashes[i].data_hash = rk_hash(data, *size);

        rkd_hashes[i].mod_power = rk_base_power(*size);
        rkd_hashes[i].data_length = *size;
        i++;
    }

    *out_length = i;

    rk_free_sizes(hsi);
    h_set_iterator_end(hsi);
    h_set_end(hs);

    return rkd_hashes;
}

static inline void rk_free_hashes(h_set_iterator *hsi)
{
    size_t size;
    char *hash;
    h_set_iterator_reset(hsi);

    while ((hash = (char *)h_set_iterator_get(hsi, &size)))
    {
        free(hash);
    }
}

static inline void rk_free_sizes(h_set_iterator *hsi)
{
    size_t size;
    char *data;
    h_set_iterator_reset(hsi);

    while ((data = (char *)h_set_iterator_get(hsi, &size)))
    {
        free(data);
    }
}

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
        unsigned char c = tolower((int) data[i]);
        hash = (hash * RK_BASE + c) % RK_MOD;
    }

    return hash;
}

static inline uint64_t rk_base_power(size_t pattern_length)
{
    uint64_t power = 1;

    for (size_t i = 1; i < pattern_length; ++i)
    {
        power = (power * RK_BASE) % RK_MOD;
    }

    return power;
}

static inline uint64_t rk_rolling_hash(uint64_t hash, unsigned char old, unsigned char new, uint64_t base_power)
{
    hash = (hash + RK_MOD - (old * base_power) % RK_MOD) % RK_MOD;
    hash = (hash * RK_BASE + new) % RK_MOD;
    return hash;
}

/*
 * Returns location of pattern from given set in given data, if not found returns NOT_FOUND
 * sets out_length to length of matched pattern
 */
static inline size_t _rk_search(rk_search *rks, const char *data, size_t data_length, size_t *out_length)
{
    for (size_t i = 1; i < data_length + 1; i++)
    {
        for (size_t j = 0; j < rks->data_hashes_length; j++)
        {
            rk_data_hash *rdh = &rks->data_hashes[j];
            if (i < rdh->data_length)
                continue;

            if (i == rdh->data_length)
                rdh->data_hash = rk_hash(data, rdh->data_length);

            else
            {
                unsigned char old = data[i - rdh->data_length - 1];
                unsigned char new = data[i - 1];
                rdh->data_hash = rk_rolling_hash(rdh->data_hash, old, new, rdh->mod_power);
            }

            if (!h_set_has(rks->patterns_hashes, (char *)&rdh->data_hash, sizeof(rdh->data_hash)))
            {
                *out_length = rdh->data_length;
                return i - rdh->data_length;
            }
        }
    }

    return NOT_FOUND;
}

/*
 * Returns location of pattern ignoring case from given set in given data,
 * if not found returns NOT_FOUND, sets out_length to length of matched pattern
 */
static inline size_t _rk_search_i(rk_search *rks, const char *data, size_t data_length, size_t *out_length)
{
    for (size_t i = 1; i < data_length + 1; i++)
    {
        for (size_t j = 0; j < rks->data_hashes_length; j++)
        {
            rk_data_hash *rdh = &rks->data_hashes[j];
            if (i < rdh->data_length)
                continue;

            if (i == rdh->data_length)
                rdh->data_hash = rk_hash_i(data, rdh->data_length);

            else
            {
                unsigned char old = tolower((int)data[i - rdh->data_length - 1]);
                unsigned char new = tolower((int)data[i - 1]);
                rdh->data_hash = rk_rolling_hash(rdh->data_hash, old, new, rdh->mod_power);
            }

            if (!h_set_has(rks->patterns_hashes, (char *)&rdh->data_hash, sizeof(rdh->data_hash)))
            {
                *out_length = rdh->data_length;
                return i - rdh->data_length;
            }
        }
    }

    return NOT_FOUND;
}

rk_search *rk_search_init(rk_data *rkd)
{
    rk_search *rks = malloc(sizeof(rk_search));
    if (!rks)
    {
        log_info("Error allocating memory!");
        return NULL;
    }

    rks->patterns = rk_get_pattern_set(rkd->patterns, rkd->pattern_lengths, rkd->count);
    if (!rks->patterns)
    {
        log_info("Error creating pattern set!");
        free(rks);
        return NULL;
    }

    rks->patterns_hashes = rk_get_pattern_hashes(rkd->patterns, rkd->pattern_lengths, rkd->count, &rks->patterns_hashes_i);
    if (!rks->patterns_hashes)
    {
        log_info("Error creating pattern hash set!");
        h_set_end(rks->patterns);
        free(rks);
        return NULL;
    }

    rks->data_hashes = rk_get_data_hashes(rkd->data, rkd->data_length, rkd->pattern_lengths, rkd->count, &rks->data_hashes_length);
    if (!rks->data_hashes)
    {
        log_info("Error creating data hash set!");
        rk_free_hashes(rks->patterns_hashes_i);
        h_set_iterator_end(rks->patterns_hashes_i);
        h_set_end(rks->patterns_hashes);
        h_set_end(rks->patterns);
        free(rks);
        return NULL;
    }

    return rks;
}

int rk_count(rk_search *rks, const char *data, size_t data_length, int ignore_case)
{
    size_t (*search)(rk_search *, const char *, size_t, size_t *) = ignore_case ? &_rk_search_i : &_rk_search;
    size_t out_length;
    int found = 0;
    size_t data_loc = 0;

    while (data_loc < data_length)
    {
        size_t loc;
        if ((loc = search(rks, data, data_length, &out_length)) == NOT_FOUND)
            return found;

        found++;
        data += loc + 1;
    }

    return found;
}

int rk_find(rk_search *rks, const char *data, size_t data_length, int ignore_case)
{
    size_t (*search)(rk_search *, const char *, size_t, size_t *) = ignore_case ? &_rk_search_i : &_rk_search;
    size_t out_length;
    return search(rks, data, data_length, &out_length) == NOT_FOUND ? 1 : 0;
}

int rk_find_w(rk_search *rks, const char *data, size_t data_length, int ignore_case)
{
    size_t (*search)(rk_search *, const char *, size_t, size_t *) = ignore_case ? &_rk_search_i : &_rk_search;
    size_t rk_result;
    size_t out_length;
    size_t data_loc = 0;

    while (data_loc < data_length)
    {
        rk_result = search(rks, data, data_length, &out_length);
        if (rk_result == NOT_FOUND)
            return 1;

        size_t loc = data_loc + rk_result;
        if ((!loc || !is_word_char(data[loc - 1])) &&
            (loc + out_length >= data_length || !is_word_char(data[loc + out_length])))
            return 0;

        data_loc += rk_result + 1;
    }

    return 1;
}

void rk_search_end(rk_search *rks)
{
    h_set_end(rks->patterns);
    rk_free_hashes(rks->patterns_hashes_i);
    h_set_iterator_end(rks->patterns_hashes_i);
    h_set_end(rks->patterns_hashes);
    free(rks->data_hashes);
    free(rks);
}
