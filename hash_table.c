
#include "hash_table.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * Represents an element put into a bucket of the hash table.
 */
struct hash_table_elem {
    /**
     * the hash table entry (containing key and value).
     */
    hash_table_entry entry;
    /**
     * The bucket the element resides in.
     */
    size_t bucket_idx;
    /**
     * a pointer to the next hash element in the bucket.
     */
    hash_table_elem *next;
};

/**
 * Checks if a number is a power of two.
 * @param n the number to check.
 * @return true if n is a power of two, false otherwise.
 */
static inline int
is_pow2(size_t n)
{
    return n != 0 && (n & (n - 1)) == 0;
}

/**
 * Rounds a number to the next power of two.
 * @param n the number to round.
 * @return the next power of two greather than n.
 */
static inline size_t
ceil_pow2(size_t n)
{
    while (!is_pow2(n)) {
        n = (n & (n - 1));
    }
    return n;
}

/**
 * Returns the index of the bucket for a given key.
 * @param tbl a pointer to the hash table instance as returned by hash_table_create().
 * @param key the key.
 * @return the bucket index.
 */
static inline size_t
bucket_idx(hash_table *tbl, const hash_table_key_t key)
{
    assert(is_pow2(tbl->num_buckets));
    return tbl->hash_func(key) & (tbl->num_buckets - 1);
}

/**
 * Creates a hash table entry for a given key, value pair.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @param val the value.
 * @param bucket_idx the bucket index for the given key.
 * @return a newly created hash table element allocated on the heap, or NULL if heap allocation failed.
 */
static inline hash_table_elem *
create_elem(hash_table *tbl, const hash_table_key_t key, const hash_table_val_t val, size_t bucket_idx)
{
    hash_table_elem *new_elem;

    // allocate heap memory
    new_elem = (hash_table_elem *)malloc(sizeof(hash_table_elem));
    if (new_elem == NULL) {
        return NULL;
    }

    // set struct members
    new_elem->entry.key = key;
    new_elem->entry.val = val;
    new_elem->bucket_idx = bucket_idx;
    new_elem->next = NULL;

    return new_elem;
}

/**
 * Looks for a hash table element for a given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key to look for
 * @return the hash table element or NULL if no element for the given key was found.
 */
static inline hash_table_elem *
find_elem(hash_table *tbl, const hash_table_key_t key)
{
    hash_table_elem *e;
    size_t idx;

    idx = bucket_idx(tbl, key);

    for (e = tbl->buckets[idx]; e != NULL; e = e->next) {
        if (tbl->cmp_func(e->entry.key, key) == 0) {
            return e;
        }
    }
    return NULL;
}

/**
 * Returns the next hash table element for a current element and bucket index.
 * @param tbl a pointer to the hash table instance.
 * @param bucket_idx the index of the current bucket.
 * @param current the current hash table element.
 * @return the next hash table element or NULL if there is no other element left.
 */
static inline hash_table_elem *
next_elem(hash_table *tbl, hash_table_elem *current, size_t bucket_idx)
{
    size_t idx;

    // case 1: bucket contains another element
    if (current != NULL && current->next != NULL) {
        return current->next;
    }
    // case 2: look for next non-empty bucket
    for (idx = bucket_idx + 1; idx < tbl->num_buckets; ++idx) {
        if (tbl->buckets[idx] != NULL) {
            return tbl->buckets[idx];
        }
    }
    return NULL;
}

/**
 * Returns the first hash table element.
 * @param tbl a pointer to the hash table instance.
 * @return the first hash table element or NULL if no element is present.
 */
static inline hash_table_elem *
first_elem(hash_table *tbl)
{
    if (hash_table_size(tbl) == 0) {
        return NULL;
    }
    return next_elem(tbl, NULL, 0);
}

/**
 * Frees memory allocated for the hash table elements.
 * @param tbl the hash table.
 */
static inline void
free_elems(hash_table *tbl)
{
    int i;
    hash_table_elem *p, *e;

    // free hash table elements first
    for (i = 0; i < tbl->num_buckets; ++i) {
        p = tbl->buckets[i];
        while (p != NULL) {
            e = p->next;
            free(p);
            p = e;
        }
        tbl->buckets[i] = NULL;
    }
}

hash_table *
hash_table_create(size_t num_buckets, hash_function *hash_func, compare_function *cmp_func)
{
    hash_table *tbl;

    // round num. of buckets to next power of two (for efficiency reasons)
    num_buckets = ceil_pow2(num_buckets);

    // allocate hash table first
    tbl = malloc(sizeof(hash_table));
    if (tbl == NULL) {
        return NULL;
    }

    // allocate buckets
    tbl->buckets = (hash_table_elem **)calloc(num_buckets, sizeof(hash_table_elem *));
    if (tbl->buckets == NULL) {
        free(tbl);
        return NULL;
    }

    // set values for hash table struct members
    tbl->num_buckets = num_buckets;
    tbl->num_elems = 0;
    tbl->hash_func = hash_func;
    tbl->cmp_func = cmp_func;

    return tbl;
}

int
hash_table_put(hash_table *tbl, const hash_table_key_t key, const hash_table_val_t val)
{
    hash_table_elem *elem, *prev_elem, *new_elem;
    size_t idx;
    int cmp_val;

    idx = bucket_idx(tbl, key);

    // handle empty bucket case
    if (tbl->buckets[idx] == NULL) {
        new_elem = create_elem(tbl, key, val, idx);
        if (new_elem == NULL) {
            return 0;
        }
        tbl->buckets[idx] = new_elem;
        tbl->num_elems++;
        return 1;
    }

    // target bucket is not empty => find correct position in bucket
    for (elem = tbl->buckets[idx], prev_elem = NULL; elem != NULL; prev_elem = elem, elem = elem->next) {
        cmp_val = tbl->cmp_func(elem->entry.key, key);
        if (cmp_val >= 0)
            break;
    }

    // found equal element => replace value
    if (cmp_val == 0) {
        // replace value of element
        elem->entry.val = val;
        return 1;
    }

    // found greater element => insert new element into bucket before the found element
    new_elem = create_elem(tbl, key, val, idx);
    if (new_elem == NULL) {
        return 0;
    }
    if (prev_elem == NULL) {
        // insert as head
        new_elem->next = tbl->buckets[idx];
        tbl->buckets[idx] = new_elem;
    } else {
        new_elem->next = prev_elem->next;
        prev_elem->next = new_elem;
    }
    tbl->num_elems++;
    return 1;
}

hash_table_val_t
hash_table_get(hash_table *tbl, const hash_table_key_t key)
{
    hash_table_elem *e = find_elem(tbl, key);
    return (e != NULL) ? e->entry.val : HASH_TABLE_VAL_NONE;
}

int
hash_table_contains(hash_table *tbl, const hash_table_key_t key)
{
    return find_elem(tbl, key) != NULL;
}

hash_table_val_t
hash_table_remove(hash_table *tbl, const hash_table_key_t key)
{
    hash_table_elem *e, *p;
    size_t idx;
    hash_table_val_t val;

    idx = bucket_idx(tbl, key);

    // find element
    for (e = tbl->buckets[idx], p = NULL; e != NULL; p =e, e = e->next) {
        if (tbl->cmp_func(e->entry.key, key) == 0) {
            val = e->entry.val;

            // remove element from hash table
            if (p == NULL) {
                // case 1: element is head
                tbl->buckets[idx] = e->next;
            }
            else {
                // case 2: element is in list
                p->next = e->next;
            }
            tbl->num_elems--;
            free(e);

            return val;
        }
    }
    return HASH_TABLE_VAL_NONE;
}

void
hash_table_map(hash_table *tbl, map_function map_func)
{
    hash_table_iter iter;
    hash_table_entry entry;

    hash_table_iterator_init(tbl, &iter);
    while (hash_table_iterator_has_next(&iter)) {
        hash_table_iterator_next(&iter);
        hash_table_iterator_get(&iter, &entry);
        map_func(&entry);
    }
}

size_t
hash_table_size(hash_table *tbl)
{
    return tbl->num_elems;
}

void
hash_table_iterator_init(hash_table *tbl, hash_table_iter *iter)
{
    iter->tbl = tbl;
    iter->current = NULL;
    iter->bucket_idx = 0;
    iter->next = first_elem(tbl);
}

int
hash_table_iterator_has_next(hash_table_iter *iter)
{
    return (iter->next != NULL);
}

void
hash_table_iterator_next(hash_table_iter *iter)
{
    if (iter->next == NULL) {
        return;
    }

    iter->current = iter->next;
    iter->bucket_idx = iter->next->bucket_idx;
    iter->next = next_elem(iter->tbl, iter->current, iter->bucket_idx);
}

void
hash_table_iterator_get(hash_table_iter *iter, hash_table_entry *out)
{
    if (iter->current == NULL) {
        out->key = NULL;
        out->val = NULL;
    }
    out->key = iter->current->entry.key;
    out->val = iter->current->entry.val;
}

hash_table_key_t
hash_table_iterator_get_key(hash_table_iter *iter)
{
    if (iter->current == NULL) {
        return NULL;
    }
    return iter->current->entry.key;
}

hash_table_val_t
hash_table_iterator_get_value(hash_table_iter *iter)
{
    if (iter->current == NULL) {
        return NULL;
    }
    return iter->current->entry.val;
}

void
hash_table_clear(hash_table *tbl)
{
    free_elems(tbl);
    tbl->num_elems = 0;
}

void
hash_table_destroy(hash_table *tbl)
{
    // free hash table elements first
    free_elems(tbl);

    // free bucket array
    free(tbl->buckets);
    tbl->buckets = NULL;

    // free hash table
    free(tbl);
    tbl = NULL;
}

unsigned int
hash_bytes(const void *data, size_t size)
{
    unsigned int hash;
    unsigned char *_data = (unsigned char *)data;

    hash = FNV_32_BASIS;
    while (size-- > 0)
        hash = (hash * FNV_32_PRIME) ^ *_data++;

    return hash;
}

unsigned int
hash_string(const void *data)
{
    unsigned int hash;
    char *_data = (char *)data;

    hash = FNV_32_BASIS;
    while (*_data != '\0')
        hash = (hash * FNV_32_PRIME) ^ *_data++;

    return hash;
}

unsigned int
hash_int(const void *val)
{
    return hash_bytes(val, sizeof(int));
}

unsigned int
hash_long(const void *val)
{
    return hash_bytes(val, sizeof(long));
}

int
hash_string_cmp(const void *a, const void *b)
{
    char *a_str = (char *)a;
    char *b_str = (char *)b;
    return strcmp(a_str, b_str);
}
