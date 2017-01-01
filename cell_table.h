#ifndef CELL_TABLE_H
#define CELL_TABLE_H

#include <stdlib.h>

#include "life.h"

/**
 * a type representing a single cell table entry.
 */
typedef struct cell_table_entry {

    /**
     * The key (a 2D point).
     */
    Point2D key;

    /**
     * The value (a pointer to the cell).
     */
    Cell *value;

} CellTableEntry;

/**
 * a type representing a cell table element.
 */
typedef struct cell_table_elem {

    /**
     * the cell table entry (containing key and value).
     */
    CellTableEntry entry;

    /**
     * the index of the bucket the element resides in.
     */
    size_t bucket_idx;

    /**
     * a flag indicating if the cell table entry is occupied.
     */
    int is_occpuied;

    /**
     * a pointer to the next cell table element.
     */
    struct cell_table_elem* next;

    /**
     * a pointer to the prev. cell table element.
     */
    struct cell_table_elem* prev;

    /**
     * unused member for memory alignment purposes.
     */
    unsigned char padding[8];

} CellTableElem;

/**
 * a type representing the cell table.
 */
typedef struct cell_table {

    /**
     * The number of buckets.
     */
    size_t num_buckets;

    /**
     * The number of elements currently stored in the table.
     */
    size_t num_elems;

    /**
     * The buckets.
     */
    CellTableElem *buckets;

} CellTable;

/**
 * a type representing the cell table iterator.
 */
typedef struct cell_table_iter {

    /**
     * The cell table the iterator iterates over.
     */
    CellTable *tbl;

    /**
     * The cell table element the iterator currently points at.
     */
    CellTableElem *current;

    /**
     * The cell table element the iterator points next.
     */
    CellTableElem *next;

} CellTableIter;

typedef void map_function(CellTableEntry *e);

/**
 * Creates a cell table
 * @param num_buckets the number of buckets to allocate.
 * @return a pointer to the cell table created on the heap.
 */
CellTable *
cell_table_create(size_t num_buckets);

/**
 * Adds an entry to the cell table.
 * @param tbl the cell table.
 * @param key the key.
 * @param value the value.
 * @return true if the entry was added successfully, false otherwise.
 */
int
cell_table_put(CellTable *tbl, const Point2D *key, const Cell *value);

/**
 * Retrieves a value by key from the cell table.
 * @param tbl the cell table.
 * @param key the key.
 * @return the value or NULL if the table does not contain an entry with given key.
 */
Cell *
cell_table_get(CellTable *tbl, const Point2D *key);

/**
 * Checks if the cell table contains an entry with a given key.
 * @param tbl the cell table.
 * @param key the key.
 * @return true if the cell table contains an entry for the given key, false otherwise.
 */
int
cell_table_contains(CellTable *tbl, const Point2D *key);

/**
 * Removes an entry from the cell table.
 * @param tbl the cell table.
 * @param key the key
 * @return the value of the removed cell table entry.
 */
Cell *
cell_table_remove(CellTable *tbl, const Point2D *key);

/**
 * Removes all entries from the cell table.
 * @param tbl the cell table.
 */
void
cell_table_clear(CellTable *tbl);

/**
 * Returns the no. of elements the cell table contains.
 * @param tbl the cell table
 * @return the number of elements the cell table contains.
 */
size_t
cell_table_size(CellTable *tbl);

/**
 * Destroys a cell table and frees all resources.
 * @param tbl the cell table.
 */
void
cell_table_destroy(CellTable *tbl);

/**
 * Initializes a cell table iterator.
 * WARNING: do not alter the cell table upon iteration!
 * @param tbl the cell table.
 * @param iter a pointer to an allocated cell table iterator instance.
 */
void
cell_table_iter_init(CellTable *tbl, CellTableIter *iter);

/**
 * Check whether the iterator can deliver a next element.
 * @param iter the cell table iterator.
 * @return true if the iterator can return a next element, false otherwise.
 */
int
cell_table_iter_has_next(CellTableIter *iter);

/**
 * Moves the iterator one step to the next element.
 * @param iter the cell table iterator.
 */
void
cell_table_iter_next(CellTableIter *iter);

/**
 * Returns the cell table entry an iterator currently points at.
 * @param iter the cell table iterator.
 * @return the cell table entry the iterator currently points at.
 */
CellTableEntry *
cell_table_iter_get(CellTableIter *iter);

/**
 * Returns the key of the cell table entry an iterator currently points at.
 * @param iter the cell table iterator.
 * @return the key of the cell table entry the iterator currently points at.
 */
Point2D *
cell_table_iter_get_key(CellTableIter *iter);

/**
 * Returns the value of the cell table entry an iterator currently points at.
 * @param iter the cell table iterator.
 * @return the value of the cell table entry the iterator currently points at.
 */
Cell *
cell_table_iter_get_val(CellTableIter *iter);

/**
 * Applies a given function to all cell table entries.
 * @param tbl the cell table
 * @param f the function to apply
 */
void
cell_table_map(CellTable *tbl, map_function f);

#endif
