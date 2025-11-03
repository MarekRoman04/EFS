#include <algo.h>

rk_search *rk_search_init(rk_data *rkd);
void rk_search_end(rk_search *rks);

static inline h_set *rk_get_pattern_set(const char **patterns, size_t *pattern_lengths, size_t count);
static inline h_set *rk_get_pattern_hashes(const char **patterns, size_t *patern_lengths, size_t count, h_set_iterator **out_iterator);
static inline rk_data_hash *rk_get_data_hashes(const char *data, size_t *pattern_lengths, size_t count, size_t *out_length);
static inline void rk_free_hashes(h_set_iterator *hsi);
static inline uint64_t rk_hash(const char *data, size_t data_length);

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
static inline h_set *rk_get_pattern_hashes(const char **patterns, size_t *patern_lengths, size_t count, h_set_iterator **out_iterator)
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

    return hs;
}

// Creates array containing rolling hashes for each pattern length
static inline rk_data_hash *rk_get_data_hashes(const char *data, size_t *patern_lengths, size_t count, size_t *out_length)
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

    rk_data_hash *hash = malloc(sizeof(rk_data_hash));
    if (!hash)
    {
        log_info("Error allocating memory!");
        h_set_iterator_end(hsi);
        h_set_end(hs);
        return NULL;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (!hash)
        {
            hash = malloc(sizeof(rk_data_hash));
            if (!hash)
            {
                log_info("Error allocating memory!");
                rk_free_hashes(hsi);
                h_set_iterator_end(hsi);
                h_set_end(hs);
                return NULL;
            }
        }

        hash->data_hash = rk_hash(data, patern_lengths[i]);
        hash->data_length = patern_lengths[i];

        int ret_val = h_set_add(hs, (const char *)hash, sizeof(*hash));
        if (!ret_val)
            hash = NULL;
        else if (ret_val < 0)
        {
            log_info("Error adding element to hash set!");
            rk_free_hashes(hsi);
            h_set_iterator_end(hsi);
            h_set_end(hs);
            return NULL;
        }
    }

    if (hash)
        free(hash);

    rk_data_hash *rkd_hashes = malloc(sizeof(rk_data_hash) * hs->length);
    if (!rkd_hashes)
    {
        log_info("Error allocating memory!");
        rk_free_hashes(hsi);
        h_set_iterator_end(hsi);
        h_set_end(hs);
        return NULL;
    }

    size_t i = 0;
    size_t length;
    while ((hash = (rk_data_hash *)h_set_iterator_get(hsi, &length)))
    {
        rkd_hashes[i].data_hash = hash->data_hash;
        rkd_hashes[i].data_length = hash->data_length;
        i++;
    }

    *out_length = i;
    printf("Written: %lu\n", i);

    rk_free_hashes(hsi);
    h_set_iterator_end(hsi);
    h_set_end(hs);

    return rkd_hashes;
}

static inline void rk_free_hashes(h_set_iterator *hsi)
{
    size_t size;
    char *hash;
    h_set_iterator_reset(hsi);

    while ((hash = h_set_iterator_get(hsi, &size)))
    {
        free(hash);
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

    rks->data_hashes = rk_get_data_hashes(rkd->data, rkd->pattern_lengths, rkd->count, &rks->data_hashes_length);
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

void rk_search_end(rk_search *rks)
{
    h_set_end(rks->patterns);
    rk_free_hashes(rks->patterns_hashes_i);
    h_set_iterator_end(rks->patterns_hashes_i);
    h_set_end(rks->patterns_hashes);
    free(rks->data_hashes);
    free(rks);
}

// size_t rabin_karp(const char *pattern, size_t pattern_length, const char *data, size_t data_length)
// {
//     if (pattern_length > data_length)
//         return NOT_FOUND;

//     uint64_t pattern_hash = hash(pattern, pattern_length);
//     uint64_t data_hash = rk_hash(data, pattern_length);
//     uint64_t base_power = mod_power(RP_HASH_BASE, pattern_length - 1);

//     for (size_t i = 0; i <= data_length; i++)
//     {
//         if (data_hash == pattern_hash && !memcmp(pattern, data + i, pattern_length))
//             return i;

//         if (i < data_length)
//             data_hash = rk_rolling_hash(data_hash, data[i], data[i + 1], base_power);
//     }

//     return NOT_FOUND;
// }

uint64_t mod_power(int base, int exp)
{
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

uint64_t rk_rolling_hash(uint64_t hash, char old, char new, uint64_t base_power)
{
    uint64_t new_hash = (RP_HASH_BASE * (hash - (unsigned char)old * base_power % RP_HASH_PRIME) + (unsigned char)new) % RP_HASH_PRIME;
    return new_hash;
}
