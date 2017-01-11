
#include "cell_table.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/**
 * Fowler-Noll-Vo 32-bit constants
 * @see https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 */
#define FNV_32_PRIME 16777619u
#define FNV_32_BASIS 2166136261u

/**
 * Calculates a Fowler-Noll-Vo (FNV) 32-bit hash value of arbitrary data.
 * @param data the data to hash
 * @param size the size of the data to hash
 * @return the calculated FNV hash value.
 */
static inline unsigned int
hash_bytes(const void *data, size_t size)
{
    unsigned int hash;
    unsigned char *_data = (unsigned char *)data;

    hash = FNV_32_BASIS;
    while (size-- > 0)
        hash = (hash * FNV_32_PRIME) ^ *_data++;

    return hash;
}

/**
 * Calculates a Fowler-Noll-Vo (FNV) 32-bit hash value of 2D points.
 * @param p the point
 * @return the calculated FNV hash value.
 */
static inline unsigned int
hash_point2d(const Point2D *p)
{
    return hash_bytes(p, sizeof(Point2D));
}

/**
 * Compares two 2D points.
 * @param p1 the first point for comparison
 * @param p2 the second point for comparison
 * @return 0 when p1 == p2, a value > 0 when p1 > p2 or a value < 0 when p1 < p2
 */
static inline int
point2d_cmp(const Point2D *p1, const Point2D *p2)
{
    int dx = p1->x - p2->x;
    return (dx == 0) ? (p1->y - p2->y) : dx;
}

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
 * Returns the bucket index for a given hash value.
 * @param tbl a pointer to the hash table instance as returned by cell_table_create().
 * @param hash_val the hash value
 * @return the bucket index.
 */
static inline size_t
bucket_idx_hash(CellTable *tbl, unsigned int hash_val)
{
    assert(is_pow2(tbl->num_buckets));
    return hash_val & (tbl->num_buckets - 1);
}

/**
 * Returns the index of the bucket for a given key.
 * @param tbl a pointer to the hash table instance as returned by cell_table_create().
 * @param key the key.
 * @return the bucket index.
 */
static inline size_t
bucket_idx_key(CellTable *tbl, const Point2D *key)
{
    return bucket_idx_hash(tbl, hash_point2d(key));
}

/**
 * Writes key and value to a cell table entry.
 * @param e the cell table entry to write
 * @param key the key
 * @param value the value
 */
static inline void
write_entry(CellTableEntry *e, const Point2D *key, const Cell *value)
{
    e->key.x = key->x;
    e->key.y = key->y;
    e->value = (Cell *)value;
}

/**
 * Resolves a conflict and returns the next bucket to try.
 * @param key the key of the entry we try to insert.
 * @param idx the bucket index that caused a conflict.
 * @param probe_cnt the number of prev. tries to resolve the conflict.
 * @param num_buckets the overall number of buckets.
 * @return a new bucket index to try.
 */
static inline size_t
resolve_conflict(const Point2D *key, size_t idx, size_t probe_cnt, size_t num_buckets) {
    return (idx + 1) & (num_buckets - 1); // linear probing
}

/**
 * Look up a cell table element by key.
 * @param tbl the cell table.
 * @param key the key.
 * @param start_idx TODO
 * @return the found cell table element or NULL if no element with given key is stored in the cell table.
 */
static inline CellTableElem *
find_elem(CellTable *tbl, const Point2D *key, size_t start_idx)
{
    size_t probe_cnt = 0;
    size_t idx = start_idx;
    CellTableElem *elem = &tbl->buckets[idx];

    while (elem->is_occpuied && probe_cnt < tbl->num_buckets) {
        if (point2d_cmp(&elem->entry.key, key) == 0) {
            return elem;
        }

        // try next bucket according to conflict resolution strategy
        idx = resolve_conflict(key, idx, probe_cnt, tbl->num_buckets);
        elem = &tbl->buckets[idx];
        ++probe_cnt;
    }

    return NULL;
}

/**
 * Looks for the next occupied element in the cell table.
 * @param tbl the cell table.
 * @param prev_idx the index of the prev. element.
 * @param out_elem an output parameter for the found element.
 * @param out_idx an output parameter for the found element's bucket index.
 */
static void
next_elem_iter(CellTable *tbl, size_t prev_idx, CellTableElem **out_elem, size_t *out_idx)
{
    size_t idx;
    CellTableElem *elem;

    for (idx = prev_idx + 1; idx < tbl->num_buckets; ++idx) {
        elem = &tbl->buckets[idx];
        if (elem->is_occpuied) {
            *out_elem = elem;
            *out_idx = idx;
            return;
        }
    }

    *out_elem = NULL;
    *out_idx = -1;
}

/**
 * Looks for the first occupied element in the cell table.
 * @param tbl the cell table.
 * @param out_elem an output parameter for the found element.
 * @param out_idx an output parameter for the found element's bucket index.
 */
static inline void
first_elem_iter(CellTable *tbl, CellTableElem **out_elem, size_t *out_idx)
{
    if (tbl->num_elems == 0) {
        *out_elem = NULL;
        *out_idx = -1;
    } else {
        next_elem_iter(tbl, -1, out_elem, out_idx);
    }
}

/**
 * Calculates the current load factor.
 * @return the current load factor
 */
static inline float
current_load(CellTable *tbl)
{
    return (float)tbl->num_elems / tbl->num_buckets;
}

/**
 * Rehashes the hash table with a new bucket array twice the size.
 * @param tbl the hash table to rehash
 * @return true if the operation succeeded, false otherwise
 */
static int
rehash(CellTable *tbl)
{
    CellTableElem *new_buckets;
    size_t new_num_buckets, idx, new_idx, probe_cnt;
    CellTableElem *elem, *new_elem;

    // allocate new bucket array
    new_num_buckets = tbl->num_buckets * 2;
    new_buckets = calloc(new_num_buckets, sizeof(CellTableElem));
    if (new_buckets == NULL) {
        return 0;
    }

    // perform rehashing
    for (idx = 0; idx < tbl->num_buckets; ++idx) {
        elem = &tbl->buckets[idx];

        // skip empty buckets
        if (!elem->is_occpuied) {
            continue;
        }

        // get index of new bucket
        new_idx = elem->hash_val & (new_num_buckets - 1);
        new_elem = &new_buckets[new_idx];

        // find next free new bucket
        probe_cnt = 0;
        while (new_elem->is_occpuied) {
            new_idx = resolve_conflict(&elem->entry.key, new_idx, probe_cnt, new_num_buckets);
            new_elem = &new_buckets[new_idx];
            ++probe_cnt;
        }

        // write new bucket
        write_entry(&new_elem->entry, &elem->entry.key, elem->entry.value);
        new_elem->hash_val = elem->hash_val;
        new_elem->is_occpuied = 1;
    }

    free(tbl->buckets);
    tbl->num_buckets = new_num_buckets;
    tbl->buckets = new_buckets;

    return 1;
}

CellTable *
cell_table_create(size_t num_buckets, float load_factor)
{
    if (load_factor <= 0 || load_factor >= 1) {
        // TODO: set errno
        return NULL;
    }

    // allocate cell table first
    CellTable *tbl = malloc(sizeof(CellTable));
    if (tbl == NULL) {
        return NULL;
    }

    // round no. of buckets to next power of two
    num_buckets = ceil_pow2(num_buckets);

    // allocate buckets
    tbl->buckets = calloc(num_buckets, sizeof(CellTableElem));
    if (tbl->buckets == NULL) {
        free(tbl);
        return NULL;
    }

    tbl->num_buckets = num_buckets;
    tbl->load_factor = load_factor;
    tbl->num_elems = 0;

    return tbl;
}

int
cell_table_put(CellTable *tbl, const Point2D *key, const Cell *value)
{
    unsigned int hash_val;
    size_t idx, probe_cnt;
    CellTableElem *elem;

    if (current_load(tbl) > tbl->load_factor && !rehash(tbl)) {
        return 0;
    }

    hash_val = hash_point2d(key);
    idx = bucket_idx_hash(tbl, hash_val);
    elem = &tbl->buckets[idx];

    // find next free element
    probe_cnt = 0;
    while (elem->is_occpuied) {
        // check if we have to update an already existing value
        if (point2d_cmp(&elem->entry.key, key) == 0) {
            elem->entry.value = (Cell *)value;
            return 1;
        }

        // try next bucket
        idx = resolve_conflict(key, idx, probe_cnt, tbl->num_buckets);
        elem = &tbl->buckets[idx];
        ++probe_cnt;
    }

    // update bucket
    write_entry(&elem->entry, key, value);
    elem->hash_val = hash_val;
    elem->is_occpuied = 1;

    tbl->num_elems++;

    return 1;
}

int
cell_table_contains(CellTable *tbl, const Point2D *key)
{
    size_t idx = bucket_idx_key(tbl, key);
    CellTableElem *elem = find_elem(tbl, key, idx);
    return elem != NULL;
}

Cell *
cell_table_get(CellTable *tbl, const Point2D *key)
{
    size_t idx = bucket_idx_key(tbl, key);
    CellTableElem *elem = find_elem(tbl, key, idx);
    return (elem != NULL) ? elem->entry.value : NULL;
}

Cell *
cell_table_remove(CellTable *tbl, const Point2D *key)
{
    unsigned int hash_val;
    size_t idx_orig, idx, probe_cnt;
    CellTableElem *elem_found, *elem_last_conflict, *e;
    Cell *found_val;

    hash_val = hash_point2d(key);
    idx = idx_orig = bucket_idx_hash(tbl, hash_val);

    // search for element to remove
    e =  &tbl->buckets[idx];
    elem_found = NULL;
    probe_cnt = 0;
    while (e->is_occpuied && probe_cnt < tbl->num_buckets) {
        if (point2d_cmp(&e->entry.key, key) == 0) {
            elem_found = e;
            break;
        }

        // try next bucket according to conflict resolution strategy
        idx = resolve_conflict(key, idx, probe_cnt, tbl->num_buckets);
        e = &tbl->buckets[idx];
        ++probe_cnt;
    }

    if (elem_found == NULL) {
        return NULL;
    }

    found_val = elem_found->entry.value;

    // skip found element
    idx = resolve_conflict(key, idx, probe_cnt, tbl->num_buckets);
    e = &tbl->buckets[idx];
    ++probe_cnt;

    // check conflict chain and search for last conflicting element
    elem_last_conflict = NULL;
    while (e->is_occpuied && probe_cnt < tbl->num_buckets) {
        // check if next bucket is a conflict with the found bucket
        if (idx_orig == bucket_idx_hash(tbl, e->hash_val)) {
            elem_last_conflict = e;
        }

        // advance to next bucket
        idx = resolve_conflict(key, idx, probe_cnt, tbl->num_buckets);
        e = &tbl->buckets[idx];
        ++probe_cnt;
    }

    // case 1: found element did not cause a conflict or is the last element in the conflict chain
    if (elem_last_conflict == NULL) {
        elem_found->is_occpuied = 0;
    }
    // case 2: found element is part but not the last element of a conflict chain
    else {
        // swap element to remove with last element of conflict chain
        write_entry(&elem_found->entry, &elem_last_conflict->entry.key, elem_last_conflict->entry.value);
        elem_found->hash_val = elem_last_conflict->hash_val;
        elem_last_conflict->is_occpuied = 0;
    }

    tbl->num_elems--;

    return found_val;
}

void
cell_table_clear(CellTable *tbl)
{
    size_t idx;
    CellTableElem *elem;

    for (idx = 0; idx < tbl->num_buckets; ++idx) {
        elem = &tbl->buckets[idx];
        // mark bucket heads as free
        elem->is_occpuied = 0;
    }

    // TODO: check if memset(0x0) for complete bucket field is faster

    tbl->num_elems = 0;
}

size_t
cell_table_size(CellTable *tbl)
{
    return tbl->num_elems;
}

void
cell_table_destroy(CellTable *tbl)
{
    free(tbl->buckets);
    free(tbl);
}

void
cell_table_iter_init(CellTable *tbl, CellTableIter *iter)
{
    iter->tbl = tbl;
    iter->current = NULL;
    iter->current_idx = -1;
    first_elem_iter(tbl, &iter->next, &iter->next_idx);
}

int
cell_table_iter_has_next(CellTableIter *iter)
{
    return iter->next != NULL;
}

void
cell_table_iter_next(CellTableIter *iter)
{
    iter->current = iter->next;
    iter->current_idx = iter->next_idx;
    next_elem_iter(iter->tbl, iter->current_idx, &iter->next, &iter->next_idx);
}

CellTableEntry *
cell_table_iter_get(CellTableIter *iter)
{
    return &iter->current->entry;
}

Point2D *
cell_table_iter_get_key(CellTableIter *iter)
{
    return &iter->current->entry.key;
}

Cell *
cell_table_iter_get_val(CellTableIter *iter)
{
    return iter->current->entry.value;
}

void
cell_table_map(CellTable *tbl, map_function map_func)
{
    size_t idx;
    CellTableElem *elem;

    for (idx = 0; idx < tbl->num_buckets; ++idx) {
        elem = &tbl->buckets[idx];
        if (elem->is_occpuied) {
            map_func(&elem->entry);
        }
    }
}
