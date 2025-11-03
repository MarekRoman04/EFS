#include <algo.h>

h_set *h_set_init();
int h_set_add(h_set *hs, const char *data, size_t data_length);
void h_set_end(h_set *hs);
h_set_iterator *h_set_iterator_init(h_set *hs);
const char *h_set_iterator_get(h_set_iterator *hsi, size_t *out_length);
void h_set_iterator_end(h_set_iterator *hsi);

static inline uint64_t fnv_1a_hash(const char *data, size_t data_length);
static inline int h_set_add_entry(h_set_entry *entries, int idx, const char *data, size_t data_length);
static inline int h_set_grow(h_set *h_set);
static inline void h_set_free_entries(h_set_entry *entries, int count);

// FNV-1a hash function
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

// Adds entry to h_set->entries on idx, on collision creates new entry node
static inline int h_set_add_entry(h_set_entry *entries, int idx, const char *data, size_t data_length)
{
    h_set_entry *data_entry = &entries[idx];

    if (data_entry->data)
    {
        h_set_entry *new_entry = malloc(sizeof(h_set_entry));
        if (!new_entry)
        {
            log_info("Error allocating memory!");
            return -1;
        }

        while (data_entry->next)
            data_entry = data_entry->next;

        data_entry->next = new_entry;
        data_entry = new_entry;
    }

    data_entry->data = data;
    data_entry->data_length = data_length;
    data_entry->next = NULL;

    return 0;
}

// Exponentialy grows table, copies entries to newly created entry list
static inline int h_set_grow(h_set *hs)
{
    int new_capacity = hs->capacity * 2;
    h_set_entry *new_entries = calloc(new_capacity, sizeof(h_set_entry));
    if (!new_entries)
    {
        log_info("Error allocating memory!");
        return -1;
    }

    for (int i = 0; i < hs->capacity; i++)
    {
        h_set_entry *entry = &hs->entries[i];
        if (!entry->data)
            continue;

        for (; entry; entry = entry->next)
        {
            int idx = fnv_1a_hash(entry->data, entry->data_length) % new_capacity;
            if (h_set_add_entry(new_entries, idx, entry->data, entry->data_length))
            {
                log_info("Error copying entries!");
                h_set_free_entries(new_entries, new_capacity);
                return -1;
            }
        }
    }

    h_set_free_entries(hs->entries, hs->capacity);
    hs->capacity = new_capacity;
    hs->entries = new_entries;
    return 0;
}

static inline void h_set_free_entries(h_set_entry *entries, int count)
{
    for (int i = 0; i < count; i++)
    {
        h_set_entry *entry = entries[i].next;
        while (entry)
        {
            h_set_entry *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(entries);
}

h_set *h_set_init()
{
    h_set *hs = malloc(sizeof(h_set));
    if (!hs)
    {
        log_info("Error allocating memory!");
        return NULL;
    }

    hs->capacity = TABLE_MIN_BUCKETS;
    hs->length = 0;
    hs->entries = calloc(hs->capacity, sizeof(h_set_entry));
    if (!hs->entries)
    {
        free(hs);
        log_info("Error allocating memory!");
        return NULL;
    }

    return hs;
}

int h_set_add(h_set *hs, const char *data, size_t data_length)
{
    if (!h_set_has(hs, data, data_length))
        return 1;

    if (hs->length > (hs->capacity * 2 / 3) && h_set_grow(hs))
    {
        log_info("Error resizing table!");
        return -1;
    }

    uint64_t idx = fnv_1a_hash(data, data_length) % hs->capacity;
    if (h_set_add_entry(hs->entries, idx, data, data_length))
        return -1;

    hs->length++;

    return 0;
}

int h_set_has(h_set *hs, const char *data, size_t data_length)
{
    int idx = fnv_1a_hash(data, data_length) % hs->capacity;
    h_set_entry *entry = &hs->entries[idx];
    for (; entry; entry = entry->next)
    {
        if (!entry->data)
            break;

        if (entry->data_length == data_length && !memcmp(data, entry->data, data_length))
            return 0;
    }

    return -1;
}

void h_set_end(h_set *hs)
{
    h_set_free_entries(hs->entries, hs->capacity);
    free(hs);
}

h_set_iterator *h_set_iterator_init(h_set *hs)
{
    h_set_iterator *hsi = malloc(sizeof(h_set_iterator));
    if (!hsi)
    {
        log_info("Error allocating memory!");
        return NULL;
    }

    hsi->hs = hs;
    h_set_iterator_reset(hsi);

    return hsi;
}

const char *h_set_iterator_get(h_set_iterator *hsi, size_t *out_length)
{
    const char *ret_val = NULL;

    while (hsi->idx < hsi->hs->capacity)
    {
        while (hsi->entry)
        {
            if (!hsi->entry->data)
                break;

            ret_val = hsi->entry->data;
            *out_length = hsi->entry->data_length;
            hsi->entry = hsi->entry->next;
            return ret_val;
        }

        hsi->idx++;
        if (hsi->idx < hsi->hs->capacity)
            hsi->entry = &hsi->hs->entries[hsi->idx];
    }

    return ret_val;
}

void h_set_iterator_end(h_set_iterator *hsi)
{
    free(hsi);
}