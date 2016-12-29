
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hash_table.h"

#define FNV_32_PRIME 16777619u
#define FNV_32_BASIS 2166136261u

int
main(int argc, char *argv[])
{
    hash_table *tbl;
    char *a, *b, *a2, *b2, *c2, *key, *val;

    hash_table_iter iter;

    tbl = hash_table_create(32, &hash_string, &hash_string_cmp);
    hash_table_put(tbl, "a", "anton");
    hash_table_put(tbl, "b", "berta");

    a = (char *)hash_table_get(tbl, "a");
    b = (char *)hash_table_get(tbl, "b");

    printf("a = %s\n", a);
    printf("b = %s\n", b);
    printf("size = %d\n", hash_table_size(tbl));

    hash_table_put(tbl, "a", "alpha");
    hash_table_put(tbl, "b", "bravo");
    hash_table_put(tbl, "c", "charly");

    a2 = (char *)hash_table_get(tbl, "a");
    b2 = (char *)hash_table_get(tbl, "b");
    c2 = (char *)hash_table_get(tbl, "c");

    printf("a = %s\n", a2);
    printf("b = %s\n", b2);
    printf("c = %s\n", c2);
    printf("size = %d\n", hash_table_size(tbl));

    printf("Dumping hash table content ::\n");
    hash_table_iterator_init(tbl, &iter);
    while (hash_table_iterator_has_next(&iter)) {
        hash_table_iterator_next(&iter);
        key = (char *)hash_table_iterator_get_key(&iter);
        val = (char *)hash_table_iterator_get_value(&iter);
        printf("%s = %s\n", key, val);
    }

    hash_table_destroy(tbl);

    return EXIT_SUCCESS;
}
