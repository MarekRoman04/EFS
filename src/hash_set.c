#include <hash_set.h>

#define H_SET_INITIAL_CAPACITY 8
#define FNV_OFFSET_BASIS 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

struct h_set
{
    int capacity;
    int length;
    const char **data;
    size_t *data_lengths;
};

h_set *h_set_init(int capacity);
int h_set_insert(h_set *hs, const char *data, size_t data_length);
int h_set_has(h_set *hs, const char *data, size_t data_length);
char **h_set_move_end(h_set *hs, size_t **dest_lengths, size_t *count);
void h_set_end(h_set *hs);

void h_set_iterator_init(h_set_iterator *hsi, h_set *hs);
void h_set_iterator_reset(h_set_iterator *hsi);
const char *h_set_iterator_get(h_set_iterator *hsi, size_t *out_length);

static inline uint64_t fnv_1a_hash(const char *data, size_t data_length);
static inline int h_set_grow(h_set *h_set);

static inline uint64_t fnv_1a_hash(const char *data, size_t data_length)
{
    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < data_length; i++)
    {
        hash ^= (uint64_t)(unsigned char)data[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

static inline int h_set_grow(h_set *hs)
{
    int new_capacity = hs->capacity * 2;
    const char **new_data = calloc(new_capacity, sizeof(char *));
    size_t *new_lengths = calloc(new_capacity, sizeof(size_t));
    if (!new_data || !new_lengths)
    {
        free(new_data);
        free(new_lengths);
        return 1;
    }

    const char **old_data = hs->data;
    size_t *old_lengths = hs->data_lengths;
    hs->data = new_data;
    hs->data_lengths = new_lengths;
    hs->capacity = new_capacity;

    for (int i = 0; i < hs->capacity / 2; i++)
    {
        if (!old_data[i])
            continue;

        int idx = fnv_1a_hash(old_data[i], old_lengths[i]) % hs->capacity;
        while (hs->data[idx])
            idx = (idx + 1) % hs->capacity;

        hs->data[idx] = old_data[i];
        hs->data_lengths[i] = old_lengths[i];
    }

    free(old_data);
    free(old_lengths);

    return 0;
}

h_set *h_set_init(int capacity)
{
    h_set *hs = malloc(sizeof(h_set));
    if (!hs)
        return NULL;

    hs->capacity = capacity ? capacity : H_SET_INITIAL_CAPACITY;
    hs->length = 0;
    hs->data = calloc(hs->capacity, sizeof(char *));
    hs->data_lengths = calloc(hs->capacity, sizeof(size_t));
    if (!hs->data || !hs->data_lengths)
    {
        free(hs->data);
        free(hs->data_lengths);
        return NULL;
    }

    return hs;
}

int h_set_insert(h_set *hs, const char *data, size_t data_length)
{
    if (hs->length > (hs->capacity * 2 / 3) && h_set_grow(hs))
        return -1;

    int idx = fnv_1a_hash(data, data_length) % hs->capacity;
    while (hs->data[idx])
    {
        if (hs->data_lengths[idx] == data_length && !memcmp(hs->data[idx], data, data_length))
            return 1;

        idx = (idx + 1) % hs->capacity;
    }

    char *hs_data = malloc(data_length);
    if (!hs_data)
        return -1;

    memcpy(hs_data, data, data_length);

    hs->data[idx] = hs_data;
    hs->data_lengths[idx] = data_length;
    hs->length++;

    return 0;
}

int h_set_has(h_set *hs, const char *data, size_t data_length)
{
    int idx = fnv_1a_hash(data, data_length) % hs->capacity;
    int initial_idx = idx;

    do
    {
        if (!hs->data[idx])
            return -1;

        if (hs->data_lengths[idx] == data_length && !memcmp(hs->data[idx], data, data_length))
            return 0;

        idx = (idx + 1) % hs->capacity;

    } while (idx != initial_idx);

    return -1;
}

char **h_set_move_end(h_set *hs, size_t **dest_lengths, size_t *count)
{
    int write = 0;

    for (int read = 0; read < hs->capacity; read++)
    {
        if (hs->data[read])
        {
            if (write != read)
            {
                hs->data[write] = hs->data[read];
                hs->data_lengths[write] = hs->data_lengths[read];

                hs->data[read] = NULL;
                hs->data_lengths[read] = 0;
            }

            write++;
        }
    }

    *count = hs->length;
    *dest_lengths = hs->data_lengths;
    char **data = (char **)hs->data;
    free(hs);

    return data;
}

void h_set_end(h_set *hs)
{
    if (!hs)
        return;

    for (int i = 0; i < hs->capacity; i++)
        free((char *)hs->data[i]);

    free(hs->data);
    free(hs->data_lengths);
    free(hs);
}

void h_set_iterator_init(h_set_iterator *hsi, h_set *hs)
{
    hsi->hs = hs;
    hsi->idx = 0;
}

void h_set_iterator_reset(h_set_iterator *hsi) { hsi->idx = 0; }

const char *h_set_iterator_get(h_set_iterator *hsi, size_t *out_length)
{
    const char *data;
    while (hsi->idx < hsi->hs->capacity && !hsi->hs->data[hsi->idx])
        hsi->idx++;

    if (hsi->idx >= hsi->hs->capacity)
        return NULL;

    data = hsi->hs->data[hsi->idx];
    if (out_length)
        *out_length = hsi->hs->data_lengths[hsi->idx];

    hsi->idx++;

    return data;
}
