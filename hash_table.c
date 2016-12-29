
#include "hash_table.h"

#include <stdio.h>
#include <string.h>

/**
 * Represents an element put into a bucket of the hash table.
 */
struct hash_table_elem {
    /**
     * the hash table entry (containing key and value).
     */
    hash_table_entry entry;
    /**
     * a pointer to the next hash element in the bucket.
     */
    hash_table_elem *next;
};

/**
 * Returns the index of the bucket for a given key.
 * @param tbl a pointer to the hash table instance as returned by hash_table_create().
 * @param key the key.
 * @return the bucket index.
 */
static size_t
get_bucket_idx(hash_table *tbl, hash_table_key_t key)
{
    // TODO: assert that bucket size is a power of two!
    return tbl->hash_func(key) & (tbl->bucket_cnt - 1);
}

/**
 * Creates a hash table entry for a given key, value pair.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @param val the value.
 * @return a newly created hash table element allocated on the heap, or NULL if heap allocation failed.
 */
static hash_table_elem *
create_elem(hash_table *tbl, hash_table_key_t key, hash_table_val_t val)
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
    new_elem->next = NULL;

    return new_elem;
}

/**
 * Looks for a hash table element for a given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key to look for
 * @return the hash table element or NULL if no element for the given key was found.
 */
static hash_table_elem *
find_elem(hash_table *tbl, hash_table_key_t key)
{
    hash_table_elem *e;
    size_t idx;

    idx = get_bucket_idx(tbl, key);

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
static hash_table_elem *
next_elem(hash_table *tbl, hash_table_elem *current, size_t bucket_idx)
{
    size_t idx;

    // case 1: bucket contains another element
    if (current != NULL && current->next != NULL) {
        return current->next;
    }
    // case 2: look for next non-empty bucket
    for (idx = bucket_idx + 1; idx < tbl->bucket_cnt; ++idx) {
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
static hash_table_elem *
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
static void
free_elems(hash_table *tbl)
{
    int i;
    hash_table_elem *p, *e;

    // free hash table elements first
    for (i = 0; i < tbl->bucket_cnt; ++i) {
        p = tbl->buckets[i];
        while (p != NULL) {
            e = p->next;
            free(p);
            p = e;
        }
        tbl->buckets[i] = NULL;
    }
}

/**
 * Creates a hash table.
 * @param bucket_cnt the (initial) number of buckets the hash table shall use.
 * @param hash_func the hash function to use when adding / retrieving elements.
 * @param cmp_func the compare function used to compare two keys.
 * @return a pointer to a hash instance created on the heap; use hash_table_destroy() for cleanup!
 */
hash_table *
hash_table_create(size_t bucket_cnt, hash_function *hash_func, compare_function *cmp_func)
{
    hash_table *tbl;

    // TODO: assert that bucket count is a power of two

    // allocate hash table first
    tbl = malloc(sizeof(hash_table));
    if (tbl == NULL) {
        return NULL;
    }

    // allocate buckets
    tbl->buckets = (hash_table_elem **)calloc(bucket_cnt, sizeof(hash_table_elem *));
    if (tbl->buckets == NULL) {
        free(tbl);
        return NULL;
    }

    // set values for hash table struct members
    tbl->bucket_cnt = bucket_cnt;
    tbl->elem_cnt = 0;
    tbl->hash_func = hash_func;
    tbl->cmp_func = cmp_func;

    return tbl;
}

/**
 * Puts a key, value pair into the hash table.
 * @param tbl a pointer to the hash table instance as returned by hash_table_create().
 * @param key the key.
 * @param val the value.
 * @return an integer interpreted as true when the pair was successfully inserted, false otherwise.
 */
int
hash_table_put(hash_table *tbl, hash_table_key_t key, hash_table_val_t val)
{
    hash_table_elem *elem, *prev_elem, *new_elem;
    size_t idx;
    int cmp_val;

    idx = get_bucket_idx(tbl, key);

    // handle empty bucket case
    if (tbl->buckets[idx] == NULL) {
        new_elem = create_elem(tbl, key, val);
        if (new_elem == NULL) {
            return 0;
        }
        tbl->buckets[idx] = new_elem;
        tbl->elem_cnt++;
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
    new_elem = create_elem(tbl, key, val);
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
    tbl->elem_cnt++;
    return 1;
}

/**
 * Retrieves a value for a given key from the hash table instance.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return the value, or NULL.
 */
void *
hash_table_get(hash_table *tbl, hash_table_key_t key)
{
    hash_table_elem *e = find_elem(tbl, key);
    return (e != NULL) ? e->entry.val : NULL;
}

/**
 * Checks if the hash table contains a pair for a given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return true if hash table contains a pair for the given key, false otherwise.
 */
int
hash_table_contains(hash_table *tbl, hash_table_key_t key)
{
    return find_elem(tbl, key) != NULL;
}

/**
 * Removes an entry from the hash table with given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return the value of the hash table entry or NULL if no entry with the given key was present.
 */
hash_table_val_t
hash_table_remove(hash_table *tbl, hash_table_key_t key)
{
    hash_table_elem *e, *p;
    size_t idx;
    hash_table_val_t val;

    idx = get_bucket_idx(tbl, key);

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
            tbl->elem_cnt--;
            free(e);

            return val;
        }
    }
    return NULL;
}

/**
 * Returns the number of elements currently present in the hash table.
 * @param tbl a pointer to the hash table instance.
 * @return the number of hash table entries
 */
size_t
hash_table_size(hash_table *tbl)
{
    return tbl->elem_cnt;
}

/**
 * Initializes an iterator instance for iterating over the entries of a hash table.
 * WARNING: do not modify the hash table upon iteration!
 * @param tbl a pointer to the hash table instance.
 * @param iter a pointer to a allocated hash table iterator instance.
 */
void
hash_table_iterator_init(hash_table *tbl, hash_table_iter *iter)
{
    iter->tbl = tbl;
    iter->current = NULL;
    iter->bucket_idx = 0;
    iter->next = first_elem(tbl);
}

/**
 * Checks if the iterator can deliver another item.
 * @param iter a pointer to the hash table iterator instance.
 * @return true if the iterator can deliver another item, false otherwise.
 */
int
hash_table_iterator_has_next(hash_table_iter *iter)
{
    return (iter->next != NULL);
}

/**
 * Lets the iterator point to the next hash table element.
 * @param iter a pointer to the hash table iterator instance.
 */
void
hash_table_iterator_next(hash_table_iter *iter)
{
    if (iter->next == NULL) {
        return;
    }

    iter->current = iter->next;
    iter->bucket_idx = get_bucket_idx(iter->tbl, iter->current->entry.key); // TODO: do not recalculate hash value here, extend hash elem struct by bucket nr
    iter->next = next_elem(iter->tbl, iter->current, iter->bucket_idx);
}

/**
 * Returns the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @param out the hash table entry instance to populate.
 */
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

/**
 * Returns the key of the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the key value.
 */
hash_table_key_t
hash_table_iterator_get_key(hash_table_iter *iter)
{
    if (iter->current == NULL) {
        return NULL;
    }
    return iter->current->entry.key;
}

/**
 * Returns the value of the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the key value.
 */
hash_table_val_t
hash_table_iterator_get_value(hash_table_iter *iter)
{
    if (iter->current == NULL) {
        return NULL;
    }
    return iter->current->entry.val;
}

/**
 * Removes all entries currently present in the hash table.
 * @param tbl a pointer to the hash table instance.
 */
void
hash_table_clear(hash_table *tbl)
{
    free_elems(tbl);
    tbl->elem_cnt = 0;
}

/**
 * Destroys a hash table created by hash_table_create().
 * @param tbl a pointer to the hash table instance.
 */
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

/**
 * FNV hash function implementation for arbitrary bytes.
 * @param key the value to hash
 * @return the calculated hash value.
 */
unsigned int
hash_bytes(void *val, size_t size)
{
    unsigned int hash;
    unsigned char *_val;

    // cast value to expected type.
    _val = (unsigned char *)val;

    hash = FNV_32_BASIS;
    while (size-- > 0)
        hash = (hash * FNV_32_PRIME) ^ *_val++;

    return hash;
}

/**
 * FNV hash function implementation for strings.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_string(void *val)
{
    unsigned int hash;
    char *_val;

    // cast value to expected type.
    _val = (char *)val;

    hash = FNV_32_BASIS;
    while (*_val != '\0')
        hash = (hash * FNV_32_PRIME) ^ *_val++;

    return hash;
}

/**
 * FNV hash function implementation for integers.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_int(void *val)
{
    return hash_bytes(val, sizeof(int));
}

/**
 * FNV hash function implementation for longs.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_long(void *val)
{
    return hash_bytes(val, sizeof(long));
}

/**
 * A compare function implementation for strings.
 * @param val1 the first value for comparison.
 * @param val2 the second value for comparison.
 * @return returns a value < 0 if val1 is less than val2, 0 if val1 is equal to val2 or a value > 0 otherwise.
 */
int
hash_string_cmp(void *val1, void *val2)
{
    char *_val1, *_val2;

    // cast values to expected type
    _val1 = (char *)val1;
    _val2 = (char *)val2;

    return strcmp(_val1, _val2);
}
