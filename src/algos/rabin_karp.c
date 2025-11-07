#include <algo.h>

static inline h_set *rk_get_pattern_set(const char **patterns, size_t *pattern_lengths, size_t count);
static inline h_set *rk_get_pattern_hashes(const char **patterns, size_t *patern_lengths, size_t count,
                                           size_t data_length, h_set_iterator **out_iterator);
static inline rk_data_hash *rk_get_data_hashes(const char *data, size_t data_length, size_t *patern_lengths, size_t count, size_t *out_length);
static inline void rk_free_hashes(h_set_iterator *hsi);
static inline void rk_free_sizes(h_set_iterator *hsi);
static inline uint64_t rk_hash(const char *data, size_t data_length);
static inline uint64_t rk_mod_power(size_t exp);
static inline uint64_t rk_rolling_hash(uint64_t hash, char old, char new, uint64_t base_power);
static inline size_t _rk_search(rk_search *rks, const char *data, size_t data_length, size_t *out_length);

rk_search *rk_search_init(rk_data *rkd);
int rk_count(rk_search *rks, const char *data, size_t data_length);
int rk_find(rk_search *rks, const char *data, size_t data_length);
int rk_find_w(rk_search *rks, const char *data, size_t data_length);
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
static inline h_set *rk_get_pattern_hashes(const char **patterns, size_t *patern_lengths, size_t count,
                                           size_t data_length, h_set_iterator **out_iterator)
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
        if (patern_lengths[i] > data_length)
            continue;

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
        if (patern_lengths[i] > data_length)
            continue;

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
    while ((size = (size_t *)h_set_iterator_get(hsi, &length)))
    {

        rkd_hashes[i].data_hash = rk_hash(data, *size);
        rkd_hashes[i].mod_power = rk_mod_power(*size - 1);
        rkd_hashes[i].data_length = *size;
        i++;
    }

    *out_length = i;
    printf("Written: %lu\n", i);

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
    uint64_t h = 0;
    for (size_t i = 0; i < data_length; i++)
    {
        h = (h * RP_HASH_BASE + (unsigned char)data[i]) % RP_HASH_PRIME;
    }

    return h;
}

static inline uint64_t rk_mod_power(size_t exp)
{
    uint64_t base = RP_HASH_BASE;
    uint64_t result = 1;

    while (exp > 0)
    {
        if (exp % 2)
            result = (result * base) % RP_HASH_PRIME;

        base = (base * base) % RP_HASH_PRIME;
        exp /= 2;
    }

    return result;
}

static inline uint64_t rk_rolling_hash(uint64_t hash, char old, char new, uint64_t base_power)
{
    return (RP_HASH_BASE * (hash - (unsigned char)old * base_power % RP_HASH_PRIME) + (unsigned char)new) % RP_HASH_PRIME;
}

/*
 * Returns location of pattern from given set in given data, if not found returns NOT_FOUND
 * sets out_length to length of matched pattern
 */
static inline size_t _rk_search(rk_search *rks, const char *data, size_t data_length, size_t *out_length)
{
    for (size_t i = 0; i < data_length; i++)
    {
        for (size_t j = 0; j < rks->data_hashes_length; j++)
        {
            rk_data_hash *rdh = &rks->data_hashes[i];
            if (i < rdh->data_length)
                continue;

            rdh->data_hash = rk_rolling_hash(rdh->data_hash, data[i - rdh->data_length], data[i], rdh->mod_power);

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

    rks->patterns_hashes = rk_get_pattern_hashes(rkd->patterns, rkd->pattern_lengths, rkd->count, rkd->data_length, &rks->patterns_hashes_i);
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

int rk_count(rk_search *rks, const char *data, size_t data_length)
{
    size_t out_length;
    int found = 0;
    size_t data_loc = 0;

    while (data_loc < data_length)
    {
        size_t loc;
        if ((loc = _rk_search(rks, data, data_length, &out_length)) == NOT_FOUND)
            return found;

        found++;
        data += loc + 1;
    }

    return found;
}

int rk_find(rk_search *rks, const char *data, size_t data_length)
{
    size_t out_length;
    return _rk_search(rks, data, data_length, &out_length) == NOT_FOUND ? 1 : 0;
}

int rk_find_w(rk_search *rks, const char *data, size_t data_length)
{
    size_t rk_result;
    size_t out_length;
    size_t data_loc = 0;

    while (data_loc < data_length)
    {
        rk_result = _rk_search(rks, data, data_length, &out_length);
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
