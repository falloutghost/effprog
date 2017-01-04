
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
 * Returns the index of the bucket for a given key.
 * @param tbl a pointer to the hash table instance as returned by hash_table_create().
 * @param key the key.
 * @return the bucket index.
 */
static inline size_t
bucket_idx(CellTable *tbl, const Point2D *key)
{
    assert(is_pow2(tbl->num_buckets));
    return hash_point2d(key) & (tbl->num_buckets - 1);
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
 * Creates a cell table element.
 * @param key the key value.
 * @param value the value element.
 * @param bucket_idx the bucket index.
 * @return the created cell table element or NULL if allocation failed.
 */
static inline CellTableElem *
create_elem(const Point2D *key, const Cell *value, size_t bucket_idx)
{
    CellTableElem *new_elem = malloc(sizeof(CellTableElem));
    if (new_elem == NULL) {
        return NULL;
    }
    write_entry(&new_elem->entry, key, value);
    new_elem->bucket_idx = bucket_idx;
    new_elem->is_occpuied = 1;
    new_elem->next = NULL;
    new_elem->prev = NULL;
    return new_elem;
}

/**
 * Look up a cell table element by key.
 * @param tbl the cell table.
 * @param key the key.
 * @param bucket_idx the bucket to search through.
 * @return the found cell table element or NULL if no element with given key is stored in the cell table.
 */
static inline CellTableElem *
find_elem(CellTable *tbl, const Point2D *key, size_t bucket_idx)
{
    CellTableElem *head_elem, *elem;

    head_elem = &tbl->buckets[bucket_idx];

    if (!head_elem->is_occpuied) {
        return NULL;
    }

    elem = head_elem;
    do {
        if (point2d_cmp(&elem->entry.key, key) == 0) {
            return elem;
        }
        elem = elem->next;
    } while(elem != head_elem);

    return NULL;
}

/**
 * Returns the next element in the cell table.
 * @param tbl the cell table
 * @param current the current element
 * @return the next element or NULL if there is no next element
 */
static inline CellTableElem *
next_elem(CellTable *tbl, CellTableElem *current)
{
    size_t idx;
    CellTableElem *elem;

    // case 1: bucket contains another element
    if (current->next != &tbl->buckets[current->bucket_idx]) {
        return current->next;
    }

    // case 2: look for next non-empty bucket
    for (idx = current->bucket_idx + 1; idx < tbl->num_buckets; ++idx) {
        elem = &tbl->buckets[idx];
        if (elem->is_occpuied) {
            return elem;
        }
    }

    return NULL;
}

/**
 * Returns the first element in the cell table.
 * @param tbl the cell table
 * @return the first element or NULL if the cell table is empty
 */
static inline CellTableElem *
first_elem(CellTable *tbl)
{
    size_t idx;
    CellTableElem *elem;

    if (tbl->num_elems == 0) {
        return NULL;
    }

    for (idx = 0; idx < tbl->num_buckets; ++idx) {
        elem = &tbl->buckets[idx];
        if (elem->is_occpuied) {
            return elem;
        }
    }

    return NULL;
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
 * Rehashes the hash table with 2x no. of buckets.
 * @param tbl the hash table to rehash
 * @return true if the operation succeeded, false otherwise
 */
static int
rehash(CellTable *tbl)
{
    CellTableElem *new_buckets;
    size_t new_num_buckets, idx;
    CellTableElem *elem, *elem_next, *bucket_head;

    // allocate new bucket array
    new_num_buckets = tbl->num_buckets * 2;
    new_buckets = calloc(new_num_buckets, sizeof(CellTableElem));

    // perform rehashing
    elem = first_elem(tbl);
    while (elem != NULL) {
        elem_next = next_elem(tbl, elem);

        // calculate new bucket index
        idx = hash_point2d(&elem->entry.key) & (new_num_buckets - 1);

        bucket_head = &new_buckets[idx];

        // case 1: bucket is empty
        if (!bucket_head->is_occpuied) {
            write_entry(&bucket_head->entry, &elem->entry.key, elem->entry.value);
            bucket_head->bucket_idx = idx;
            bucket_head->is_occpuied = 1;
            bucket_head->next = bucket_head;
            bucket_head->prev = bucket_head;
        }
        // case 2: bucket is already occupied => collision
        else {
            // add element at the end of the list
            elem->next = bucket_head;
            elem->prev = bucket_head->prev;
            elem->bucket_idx = idx;
            bucket_head->prev->next = elem;
            bucket_head->prev = elem;
        }

        elem = elem_next;
    }

    free(tbl->buckets);
    tbl->num_buckets = new_num_buckets;
    tbl->buckets = new_buckets;

    return 1;
}

CellTable *
cell_table_create(size_t num_buckets, float load_factor)
{
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
    size_t idx;
    CellTableElem *head_elem, *elem, *new_elem;

    idx = bucket_idx(tbl, key);
    head_elem = &tbl->buckets[idx];

    // case 1: bucket is empty
    if (!head_elem->is_occpuied) {
        write_entry(&head_elem->entry, key, value);
        head_elem->bucket_idx = idx;
        head_elem->is_occpuied = 1;
        head_elem->next = head_elem;
        head_elem->prev = head_elem;
    }
    // case 2: bucket already occupied => collision
    else {
        // check if we have to update an already existing value
        elem = find_elem(tbl, key, idx);
        if (elem != NULL) {
            elem->entry.value = (Cell *)value;
            return 1;
        }

        // add new element at the end of the list
        new_elem = create_elem(key, value, idx);
        if (new_elem == NULL) {
            return 0;
        }
        new_elem->next = head_elem;
        new_elem->prev = head_elem->prev;
        head_elem->prev->next = new_elem;
        head_elem->prev = new_elem;
    }

    tbl->num_elems++;

    if (current_load(tbl) > tbl->load_factor) {
        rehash(tbl);
    }

    return 1;
}

int
cell_table_contains(CellTable *tbl, const Point2D *key)
{
    size_t idx = bucket_idx(tbl, key);
    CellTableElem *elem = find_elem(tbl, key, idx);
    return elem != NULL;
}

Cell *
cell_table_get(CellTable *tbl, const Point2D *key)
{
    size_t idx = bucket_idx(tbl, key);
    CellTableElem *elem = find_elem(tbl, key, idx);
    return (elem != NULL) ? elem->entry.value : NULL;
}

Cell *
cell_table_remove(CellTable *tbl, const Point2D *key)
{
    size_t idx;
    CellTableElem *head_elem, *elem, *prev_elem;
    Cell *value;

    idx = bucket_idx(tbl, key);
    elem = find_elem(tbl, key, idx);
    head_elem = &tbl->buckets[idx];

    if (elem == NULL) {
        return NULL;
    }

    value = elem->entry.value;

    if (elem == head_elem) {
        // case 1: found element is first element in bucket
        elem->is_occpuied = 0;
    } else {
        // case 2: found element is an element of the linked list
        prev_elem = elem->prev;
        prev_elem->next = elem->next;
        elem->next->prev = prev_elem;
        free(elem);
    }
    tbl->num_elems--;
    return value;
}

void
cell_table_clear(CellTable *tbl)
{
    size_t idx;
    CellTableElem *head_elem, *elem, *next_elem;

    for (idx = 0; idx < tbl->num_buckets; ++idx) {
        head_elem = &tbl->buckets[idx];

        // do not touch empty buckets
        if (!head_elem->is_occpuied) {
            continue;
        }

        // mark bucket heads as free
        head_elem->is_occpuied = 0;

        // free all but first elements if bucket contains multiple elements
        if (head_elem->prev != head_elem) {
            elem = head_elem->next;
            do {
                next_elem = elem->next;
                free(elem);
                elem = next_elem;
            } while(elem != head_elem);
        }
    }

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
    cell_table_clear(tbl);
    free(tbl->buckets);
    free(tbl);
}

void
cell_table_iter_init(CellTable *tbl, CellTableIter *iter)
{
    iter->tbl = tbl;
    iter->current = NULL;
    iter->next = first_elem(tbl);
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
    iter->next = next_elem(iter->tbl, iter->current);
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
    CellTableIter iter;
    CellTableEntry *entry;

    cell_table_iter_init(tbl, &iter);
    while (cell_table_iter_has_next(&iter)) {
        cell_table_iter_next(&iter);
        entry = cell_table_iter_get(&iter);
        map_func(entry);
    }
}
