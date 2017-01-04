#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>

/**
 * A basic hash table implementation.
 */

typedef void* hash_table_key_t;
typedef void* hash_table_val_t;

/**
 * Constants used to indicate none.
 */
#define HASH_TABLE_KEY_NONE NULL
#define HASH_TABLE_VAL_NONE NULL

/**
 * Represents a hash table entry.
 */
typedef struct hash_table_entry {

    hash_table_key_t key;

    hash_table_val_t val;

} HashTableEntry;

/**
 * Represents an element put into a bucket of the hash table.
 */
typedef struct hash_table_elem {

    /**
     * the hash table entry (containing key and value).
     */
    HashTableEntry entry;

    /**
     * The bucket the element resides in.
     */
    size_t bucket_idx;

    /**
     * a pointer to the next hash element in the bucket.
     */
    struct hash_table_elem *next;

} HashTableElem;

/**
 * Define signatures for hash and compare functions.
 */
typedef unsigned int hash_function(const void *data);
typedef int compare_function(const void *a, const void *b);
typedef void map_function(HashTableEntry *entry);

/**
 * Represents a hash table.
 */
typedef struct hash_table {

    /**
     * the number of buckets.
     */
    size_t num_buckets;

    /**
     * a factor that controls growing + rehashing of the hash table.
     */
    float load_factor;

    /**
     * the number of elements currently held by the hash table.
     */
    size_t num_elems;

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
    HashTableElem **buckets;

} HashTable;

/**
 * Represents a hash table iterator.
 */
typedef struct hash_table_iter {

    /**
     * the hash table for iterating over.
     */
    HashTable *tbl;

    /**
     * a pointer to the current element in the current bucket.
     */
    HashTableElem *current;

    /**
     * a pointer to the next element (may be in another bucket).
     */
    HashTableElem *next;

} HashTableIter;

/**
 * Creates a hash table.
 * @param num_buckets the (initial) number of buckets the hash table shall use.
 * @param load_factor a factor controlling growing / rehashing of the hash table.
 * @param hash_func the hash function to use when adding / retrieving elements.
 * @param cmp_func the compare function used to compare two keys.
 * @return a pointer to a hash instance created on the heap; use hash_table_destroy() for cleanup!
 */
HashTable *
hash_table_create(size_t num_buckets, float load_factor, hash_function *hash_func, compare_function *cmp_func);

/**
 * Puts a key, value pair into the hash table.
 * @param tbl a pointer to the hash table instance as returned by hash_table_create().
 * @param key the key; when already present in the table, the entry gets overwritten!
 * @param val the value.
 * @return an integer interpreted as true when the pair was successfully inserted, false otherwise.
 */
int
hash_table_put(HashTable *tbl, const hash_table_key_t key, const hash_table_val_t val);

/**
 * Retrieves a value for a given key from the hash table instance.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return the value, or NULL.
 */
hash_table_val_t
hash_table_get(HashTable *tbl, const hash_table_key_t key);

/**
 * Checks if the hash table contains a pair for a given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return
 */
int
hash_table_contains(HashTable *tbl, const hash_table_key_t key);

/**
 * Removes an entry from the hash table with given key.
 * @param tbl a pointer to the hash table instance.
 * @param key the key.
 * @return the value of the hash table entry or NULL if no entry with the given key was present.
 */
hash_table_val_t
hash_table_remove(HashTable *tbl, const hash_table_key_t key);

/**
 * Applies a function to all elements present in the hash table.
 * @param tbl a pointer to the hash table instance.
 * @param map_func the function to apply.
 */
void
hash_table_map(HashTable *tbl, map_function map_func);

/**
 * Returns the number of elements currently present in the hash table.
 * @param tbl a pointer to the hash table instance.
 * @return the number of hash table entries
 */
size_t
hash_table_size(HashTable *tbl);

/**
 * Initializes an iterator instance for iterating over the entries of a hash table.
 * WARNING: do not modify the hash table upon iteration!
 * @param tbl a pointer to the hash table instance.
 * @param iter a pointer to a allocated hash table iterator instance.
 */
void
hash_table_iter_init(HashTable *tbl, HashTableIter *iter);

/**
 * Checks if the iterator can deliver another item.
 * @param iter a pointer to the hash table iterator instance.
 * @return true if the iterator can deliver another item, false otherwise.
 */
int
hash_table_iter_has_next(HashTableIter *iter);

/**
 * Lets the iterator point to the next hash table element.
 * @param iter a pointer to the hash table iterator instance.
 */
void
hash_table_iter_next(HashTableIter *iter);

/**
 * Returns the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the hash table entry.
 */
HashTableEntry *
hash_table_iter_get(HashTableIter *iter);

/**
 * Returns the key of the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the key value.
 */
hash_table_key_t
hash_table_iter_get_key(HashTableIter *iter);

/**
 * Returns the value of the current hash table element the iterator points to.
 * @param iter a pointer to the hash table iterator instance.
 * @return a pointer to the key value.
 */
hash_table_val_t
hash_table_iter_get_value(HashTableIter *iter);

/**
 * Removes all entries currently present in the hash table.
 * @param tbl a pointer to the hash table instance.
 */
void
hash_table_clear(HashTable *tbl);

/**
 * Destroys a hash table created by hash_table_create().
 * @param tbl a pointer to the hash table instance.
 */
void
hash_table_destroy(HashTable *tbl);

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
hash_bytes(const void *val, size_t size);

/**
 * FNV hash function implementation for strings.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_string(const void *val);

/**
 * FNV hash function implementation for integers.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_int(const void *val);

/**
 * FNV hash function implementation for longs.
 * @param key the value to hash
 * @return a calculated hash value.
 */
unsigned int
hash_long(const void *val);

/**
 * A compare function implementation for strings.
 * @param a the first value for comparison.
 * @param b the second value for comparison.
 * @return returns a value < 0 if a is less than b, 0 if a is equal to b or a value > 0 otherwise.
 */
int
hash_string_cmp(const void *a, const void *b);

#endif /* HASH_TABLE_H */
