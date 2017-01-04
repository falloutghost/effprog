#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "hash_table.h"
#include "life.h"

static HashTable *tbl_gen_current;
static HashTable *tbl_gen_next;

// Calculates a FNV hash for a Point2D instance.
static inline unsigned int
hash_point2d(const void *p)
{
  return hash_bytes(p, sizeof(Point2D));
}

// Used to compare two Point2D instances.
static inline int
point2d_cmp(const void *a, const void *b)
{
  Point2D *p1 = (Point2D *)a, *p2 = (Point2D *)b;
  int dx = p1->x - p2->x;
  return (dx == 0) ? (p1->y - p2->y) : dx;
}

// Used to free all cell instances put into the cell table(s).
static void
hash_table_gen_free(HashTableEntry *entry)
{
  free(entry->val);
  entry->val = NULL;
}

// Creates a cell instance, allocated on the heap.
static inline Cell *
create_cell(long x, long y, Status status)
{
    Cell *c = (Cell *)malloc(sizeof(Cell));
    if (c == NULL) {
        return NULL;
    }
    c->coordinates.x = x;
    c->coordinates.y = y;
    c->status = status;
    return c;
}

// Checks if a cell is alive in the current generation.
static inline
long alive(long x, long y)
{
  Point2D p;
  p.x = x;
  p.y = y;
  return hash_table_contains(tbl_gen_current, &p);
}

// Checks if a cell should be alive in the next generation;
// if the cell is alive, it is created and stored for the next generation.
static void
checkcell(long x, long y)
{
  Cell *c;
  int n=0;

  n += alive(x-1, y-1);
  n += alive(x-1, y+0);
  n += alive(x-1, y+1);
  n += alive(x+0, y-1);
  n += alive(x+0, y+1);
  n += alive(x+1, y-1);
  n += alive(x+1, y+0);
  n += alive(x+1, y+1);

  /*fprintf(stderr,"checkcell x=%ld y=%ld old=%p new=%p n=%d\n",x,y,old,new,n);*/

  if (n == 3 || (n == 2 && alive(x, y))) {
    c = create_cell(x, y, ALIVE);
    if (c == NULL) {
      perror("create_cell");
      exit(1);
    }
    hash_table_put(tbl_gen_next, &c->coordinates, c);
  }
}

// Advanced the game of life by one generation.
static void
onegeneration()
{
  HashTable *tbl_gen_tmp;
  HashTableIter iter;
  Point2D *p;
  long x, y;

  hash_table_iter_init(tbl_gen_current, &iter);
  while (hash_table_iter_has_next(&iter)) {
    hash_table_iter_next(&iter);

    p = hash_table_iter_get_key(&iter);
    x = p->x;
    y = p->y;

    checkcell(x-1, y-1);
    checkcell(x-1, y+0);
    checkcell(x-1, y+1);
    checkcell(x+0, y-1);
    checkcell(x+0, y+0);
    checkcell(x+0, y+1);
    checkcell(x+1, y-1);
    checkcell(x+1, y+0);
    checkcell(x+1, y+1);
  }

  // use calculated, next generation as current generation
  tbl_gen_tmp = tbl_gen_current;
  tbl_gen_current = tbl_gen_next;
  tbl_gen_next = tbl_gen_tmp;

  // clean next generation cell table
  hash_table_map(tbl_gen_next, &hash_table_gen_free);
  hash_table_clear(tbl_gen_next);
}

// Reads the initial state of the cells from an input file.
static void
readlife(FILE *f)
{
  struct stat sb;
  int fd;
  char *begin, *s, *end;
  long x, y;
  Cell *c;

  fd = fileno(f);

  // get file size
  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    exit(1);
  }

  // map file into memory
  begin = s = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fileno(f), 0);

  // TODO: skip header, see readlife.y

  // read cells of input file
  end = s + sb.st_size;
  while (s < end) {
    char *endptr;

    while (*s == ' ') s++;
    x = strtol(s, &endptr, 10);
    if (s == endptr) {
      perror("strtol");
      exit(1);
    }
    s = endptr;

    while (*s == ' ') s++;
    y = strtol(s, &endptr, 10);
    if (s == endptr) {
      perror("strtol");
      exit(1);
    }
    s = endptr;

    c = create_cell(x, y, ALIVE);
    if (c == NULL) {
      perror("create_cell");
      exit(1);
    }

    hash_table_put(tbl_gen_current, &c->coordinates, c);

    while (*s == ' ' || *s == '\n') s++;
  }

  munmap(begin, sb.st_size);
}

// Writes the cells which are alive in the current generation to an output file.
static void
writelife(FILE *f)
{
  HashTableIter iter;
  Point2D *p;

  hash_table_iter_init(tbl_gen_current, &iter);
  while (hash_table_iter_has_next(&iter)) {
    hash_table_iter_next(&iter);
    p = hash_table_iter_get_key(&iter);
    fprintf(f, "%ld %ld\n", p->x, p->y);
  }
}

// Counts how many cells are alive in the current generation.
static inline size_t
countcells()
{
  return hash_table_size(tbl_gen_current);
}

int main(int argc, char **argv)
{
  long generations;
  long i;
  char *endptr;

  // arguments checking.
  if (argc!=2) {
    fprintf(stderr, "Usage: %s #generations <startfile | sort >endfile\n", argv[0]);
    exit(1);
  }

  // parse nr of generations.
  generations = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0') {
    fprintf(stderr, "\"%s\" not a valid generation count\n", argv[3]);
    exit(1);
  }

  // create cell tables.
  tbl_gen_current = hash_table_create(1024, 0.75f, &hash_point2d, &point2d_cmp);
  tbl_gen_next    = hash_table_create(1024, 0.75f, &hash_point2d, &point2d_cmp);

  // read in initial generation.
  readlife(stdin);

  // advance generations.
  for (i=0; i<generations; i++) {
    onegeneration();
  }

  writelife(stdout);

  fprintf(stderr,"%zu cells alive\n", countcells());

  // free memory allocated for cells.
  hash_table_map(tbl_gen_current, &hash_table_gen_free);

  // destroy cell tables.
  hash_table_destroy(tbl_gen_current);
  hash_table_destroy(tbl_gen_next);

  return 0;
}
