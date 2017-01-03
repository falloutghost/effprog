
#include "hash_table.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

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
bucket_idx(HashTable *tbl, const hash_table_key_t key)
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
static inline HashTableElem *
create_elem(HashTable *tbl, const hash_table_key_t key, const hash_table_val_t val, size_t bucket_idx)
{
    HashTableElem *new_elem;

    // allocate heap memory
    new_elem = (HashTableElem *)malloc(sizeof(HashTableElem));
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
static inline HashTableElem *
find_elem(HashTable *tbl, const hash_table_key_t key)
{
    HashTableElem *e;
    size_t idx;
    int cmp_val;

    idx = bucket_idx(tbl, key);

    for (e = tbl->buckets[idx]; e != NULL; e = e->next) {
        cmp_val = tbl->cmp_func(e->entry.key, key);
        if (cmp_val == 0) {
            return e;
        } else if (cmp_val > 0) {
            break;
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
static inline HashTableElem *
next_elem(HashTable *tbl, HashTableElem *current, size_t bucket_idx)
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
static inline HashTableElem *
first_elem(HashTable *tbl)
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
free_elems(HashTable *tbl)
{
    int i;
    HashTableElem *p, *e;

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

HashTable *
hash_table_create(size_t num_buckets, hash_function *hash_func, compare_function *cmp_func)
{
    HashTable *tbl;

    // round num. of buckets to next power of two (for efficiency reasons)
    num_buckets = ceil_pow2(num_buckets);

    // allocate hash table first
    tbl = malloc(sizeof(HashTable));
    if (tbl == NULL) {
        return NULL;
    }

    // allocate buckets
    tbl->buckets = (HashTableElem **)calloc(num_buckets, sizeof(HashTableElem *));
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
hash_table_put(HashTable *tbl, const hash_table_key_t key, const hash_table_val_t val)
{
    HashTableElem *elem, *prev_elem, *new_elem;
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
hash_table_get(HashTable *tbl, const hash_table_key_t key)
{
    HashTableElem *e = find_elem(tbl, key);
    return (e != NULL) ? e->entry.val : HASH_TABLE_VAL_NONE;
}

int
hash_table_contains(HashTable *tbl, const hash_table_key_t key)
{
    return find_elem(tbl, key) != NULL;
}

hash_table_val_t
hash_table_remove(HashTable *tbl, const hash_table_key_t key)
{
    HashTableElem *e, *p;
    size_t idx;
    hash_table_val_t val;
    int cmp_val;

    idx = bucket_idx(tbl, key);

    // find element
    for (e = tbl->buckets[idx], p = NULL; e != NULL; p =e, e = e->next) {
        cmp_val = tbl->cmp_func(e->entry.key, key);
        if (cmp_val == 0) {
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
        } else if (cmp_val > 0) {
            break;
        }
    }
    return HASH_TABLE_VAL_NONE;
}

void
hash_table_map(HashTable *tbl, map_function map_func)
{
    HashTableIter iter;
    HashTableEntry entry;

    hash_table_iter_init(tbl, &iter);
    while (hash_table_iter_has_next(&iter)) {
        hash_table_iter_next(&iter);
        hash_table_iter_get(&iter, &entry);
        map_func(&entry);
    }
}

size_t
hash_table_size(HashTable *tbl)
{
    return tbl->num_elems;
}

void
hash_table_iter_init(HashTable *tbl, HashTableIter *iter)
{
    iter->tbl = tbl;
    iter->current = NULL;
    iter->bucket_idx = 0;
    iter->next = first_elem(tbl);
}

int
hash_table_iter_has_next(HashTableIter *iter)
{
    return (iter->next != NULL);
}

void
hash_table_iter_next(HashTableIter *iter)
{
    if (iter->next == NULL) {
        return;
    }

    iter->current = iter->next;
    iter->bucket_idx = iter->next->bucket_idx;
    iter->next = next_elem(iter->tbl, iter->current, iter->bucket_idx);
}

void
hash_table_iter_get(HashTableIter *iter, HashTableEntry *out)
{
    if (iter->current == NULL) {
        out->key = NULL;
        out->val = NULL;
    }
    out->key = iter->current->entry.key;
    out->val = iter->current->entry.val;
}

hash_table_key_t
hash_table_iter_get_key(HashTableIter *iter)
{
    if (iter->current == NULL) {
        return NULL;
    }
    return iter->current->entry.key;
}

hash_table_val_t
hash_table_iter_get_value(HashTableIter *iter)
{
    if (iter->current == NULL) {
        return NULL;
    }
    return iter->current->entry.val;
}

void
hash_table_clear(HashTable *tbl)
{
    free_elems(tbl);
    tbl->num_elems = 0;
}

void
hash_table_destroy(HashTable *tbl)
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
