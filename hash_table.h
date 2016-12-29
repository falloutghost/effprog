#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>

/**
 * A basic hash table implementation.
 */

typedef void* hash_table_key_t;
typedef void* hash_table_val_t;

// TODO: define value for None (no key, on value)

/**
 * Define signatures for hash and compare functions.
 */
typedef unsigned int hash_function(void *data);
typedef int compare_function(void *v1, void *v2);

/**
 * Define types for the exposed structures.
 */
typedef struct hash_table_elem hash_table_elem;
typedef struct hash_table hash_table;
typedef struct hash_table_iter hash_table_iter;
typedef struct hash_table_entry hash_table_entry;

// TODO: implement auto growing + rehashing

/**
 * Represents a hash table.
 */
struct hash_table {
    /**
     * the number of buckets.
     */
    size_t bucket_cnt;
    /**
     * the number of elements currently held by the hash table.
     */
    size_t elem_cnt;
    /**
     * the hash function to use.
     */
    hash_function *hash_func;
    /**
     * the compare function to use for finding elements in a bucket.
     */
    compare_function *cmp_func;
    /**
     * an array of buckets.
     */
    hash_table_elem **buckets;
};

/**
 * Represents a hash table iterator.
 */
struct hash_table_iter {
    /**
     * the hash table for iterating over.
     */
    hash_table *tbl;
    /**
     * the current bucket index.
     */
    size_t bucket_idx;
    /**
     * a pointer to the current element in the current bucket.
     */
    hash_table_elem *current;
    /**
     * a pointer to the next element (may be in another bucket).
     */
    hash_table_elem *next;
};

/**
 * Represents a hash table entry.
 */
struct hash_table_entry {
    hash_table_key_t key;
    hash_table_val_t val;
};

/**
 * Creates a hash table.
 * @param bucket_cnt the (initial) number of buckets the hash table shall use.
 * @param hash_func the hash function to use when adding / retrieving elements.
 * @param cmp_func the compare function used to compare two keys.
 * @return a pointer to a hash instance created on the heap; use hash_table_destroy() for cleanup!
 */
hash_table *
hash_table_create(size_t bucket_count, hash_function *hash_func, compare_function *cmp_func);

/**
 * Puts a key, value pair into the hash table.
 * @param tbl a pointer to the hash table instance as returned by hash_table_create().
 * @param key the key; when already present in the table, the entry gets overwritten!
 * @param val the value.
 * @return an integer interpreted as true when the pair was successfully inserted, false otherwise.
 */
int
hash_table_put(hash_table *tbl, hash_table_key_t key, hash_table_val_t val);

/**
 * Retrieves a value for a given key from the hash table instance.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return the value, or NULL.
 */
hash_table_val_t
hash_table_get(hash_table *tbl, hash_table_key_t key);

/**
 * Checks if the hash table contains a pair for a given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return
 */
int
hash_table_contains(hash_table *tbl, hash_table_key_t key);

/**
 * Removes an entry from the hash table with given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return the value of the hash table entry or NULL if no entry with the given key was present.
 */
hash_table_val_t
hash_table_remove(hash_table *tbl, hash_table_key_t key);

/**
 * Returns the number of elements currently present in the hash table.
 * @param tbl a pointer to the hash table instance.
 * @return the number of hash table entries
 */
size_t
hash_table_size(hash_table *tbl);

/**
 * Initializes an iterator instance for iterating over the entries of a hash table.
 * WARNING: do not modify the hash table upon iteration!
 * @param tbl a pointer to the hash table instance.
 * @param iter a pointer to a allocated hash table iterator instance.
 */
void
hash_table_iterator_init(hash_table *tbl, hash_table_iter *iter);

/**
 * Checks if the iterator can deliver another item.
 * @param iter a pointer to the hash table iterator instance.
 * @return true if the iterator can deliver another item, false otherwise.
 */
int
hash_table_iterator_has_next(hash_table_iter *iter);

/**
 * Lets the iterator point to the next hash table element.
 * @param iter a pointer to the hash table iterator instance.
 */
void
hash_table_iterator_next(hash_table_iter *iter);

/**
 * Returns the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @param out the hash table entry instance to populate.
 */
void
hash_table_iterator_get(hash_table_iter *iter, hash_table_entry *out);

/**
 * Returns the key of the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the key value.
 */
hash_table_key_t
hash_table_iterator_get_key(hash_table_iter *iter);

/**
 * Returns the value of the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the key value.
 */
hash_table_val_t
hash_table_iterator_get_value(hash_table_iter *iter);

/**
 * Removes all entries currently present in the hash table.
 * @param tbl a pointer to the hash table instance.
 */
void
hash_table_clear(hash_table *tbl);

/**
 * Destroys a hash table created by hash_table_create().
 * @param tbl a pointer to the hash table instance.
 */
void
hash_table_destroy(hash_table *tbl);

/**
 * Fowler-Noll-Vo 32-bit constants
 * @see https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 */
#define FNV_32_PRIME 16777619u
#define FNV_32_BASIS 2166136261u

/**
 * FNV hash function implementation for arbitrary bytes.
 * @param key the value to hash
 * @return the calculated hash value.
 */
unsigned int
hash_bytes(void *val, size_t size);

/**
 * FNV hash function implementation for strings.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_string(void *val);

/**
 * FNV hash function implementation for integers.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_int(void *val);

/**
 * FNV hash function implementation for longs.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_long(void *val);

/**
 * A compare function implementation for strings.
 * @param val1 the first value for comparison.
 * @param val2 the second value for comparison.
 * @return returns a value < 0 if val1 is less than val2, 0 if val1 is equal to val2 or a value > 0 otherwise.
 */
int
hash_string_cmp(void *val1, void *val2);

#endif /* HASH_TABLE_H */
