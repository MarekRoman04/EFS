#include <algo.h>

uint64_t fnv_1a_hash(const char *data, size_t data_length);
h_set *h_set_init();
int h_set_add(h_set *h_set, const char *data, size_t data_length);
void h_set_end(h_set *h_set);

static inline int h_set_add_entry(h_set_entry *entries, int idx, const char *data, size_t data_length);
static inline int h_set_grow(h_set *h_set);
static inline void h_set_free_entries(h_set_entry *entries, int count);

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
static inline int h_set_grow(h_set *h_set)
{
    int new_capacity = h_set->capacity * 2;
    h_set_entry *new_entries = calloc(new_capacity, sizeof(h_set_entry));
    if (!new_entries)
    {
        log_info("Error allocating memory!");
        return -1;
    }

    for (int i = 0; i < h_set->capacity; i++)
    {
        h_set_entry *entry = &h_set->entries[i];
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

    h_set_free_entries(h_set->entries, h_set->capacity);
    h_set->capacity = new_capacity;
    h_set->entries = new_entries;
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

uint64_t fnv_1a_hash(const char *data, size_t data_length)
{
    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < data_length; i++)
    {
        hash ^= (uint64_t)(unsigned char)data[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

h_set *h_set_init()
{
    h_set *h_set = malloc(sizeof(h_set));
    if (!h_set)
    {
        log_info("Error allocating memory!");
        return NULL;
    }

    h_set->capacity = TABLE_MIN_BUCKETS;
    h_set->length = 0;
    h_set->entries = calloc(h_set->capacity, sizeof(h_set_entry));
    if (!h_set->entries)
    {
        free(h_set);
        log_info("Error allocating memory!");
        return NULL;
    }

    return h_set;
}

int h_set_add(h_set *h_set, const char *data, size_t data_length)
{
    if (!h_set_has(h_set, data, data_length))
        return 1;

    if (h_set->length > (h_set->capacity * 2 / 3) && h_set_grow(h_set))
    {
        log_info("Error resizing table!");
        return -1;
    }

    uint64_t idx = fnv_1a_hash(data, data_length) % h_set->capacity;
    if (h_set_add_entry(h_set->entries, idx, data, data_length))
        return -1;

    h_set->length++;

    return 0;
}

int h_set_has(h_set *h_set, const char *data, size_t data_length)
{
    int idx = fnv_1a_hash(data, data_length) % h_set->capacity;
    h_set_entry *entry = &h_set->entries[idx];
    for (; entry; entry = entry->next)
    {
        if (!entry->data)
            break;

        if (entry->data_length == data_length && !memcmp(data, entry->data, data_length))
            return 0;
    }

    return -1;
}

void h_set_end(h_set *h_set)
{
    h_set_free_entries(h_set->entries, h_set->capacity);
    free(h_set);
}
