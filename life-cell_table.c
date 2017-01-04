#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cell_table.h"
#include "life.h"

static CellTable *tbl_gen_current;
static CellTable *tbl_gen_next;

// Used to free all cell instances put into the cell table(s).
static void
cell_table_gen_free(CellTableEntry *entry)
{
  free(entry->value);
  entry->value = NULL;
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
  return cell_table_contains(tbl_gen_current, &p);
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
    cell_table_put(tbl_gen_next, &c->coordinates, c);
  }
}

// Advanced the game of life by one generation.
static void
onegeneration()
{
  CellTable *tbl_gen_tmp;
  CellTableIter iter;
  Point2D *p;
  long x, y;

  cell_table_iter_init(tbl_gen_current, &iter);
  while (cell_table_iter_has_next(&iter)) {
    cell_table_iter_next(&iter);

    p = cell_table_iter_get_key(&iter);
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
  cell_table_map(tbl_gen_next, &cell_table_gen_free);
  cell_table_clear(tbl_gen_next);
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

    cell_table_put(tbl_gen_current, &c->coordinates, c);

    while (*s == ' ' || *s == '\n') s++;
  }

  munmap(begin, sb.st_size);
}

// Writes the cells which are alive in the current generation to an output file.
static void
writelife(FILE *f)
{
  CellTableIter iter;
  Point2D *p;

  cell_table_iter_init(tbl_gen_current, &iter);
  while (cell_table_iter_has_next(&iter)) {
    cell_table_iter_next(&iter);
    p = cell_table_iter_get_key(&iter);
    fprintf(f, "%ld %ld\n", p->x, p->y);
  }
}

// Counts how many cells are alive in the current generation.
static inline size_t
countcells()
{
  return cell_table_size(tbl_gen_current);
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
  tbl_gen_current = cell_table_create(1024, 0.75f);
  tbl_gen_next    = cell_table_create(1024, 0.75f);

  // read in initial generation.
  readlife(stdin);

  // advance generations.
  for (i=0; i<generations; i++) {
    onegeneration();
  }

  writelife(stdout);

  fprintf(stderr,"%zu cells alive\n", countcells());

  // free memory allocated for cells.
  cell_table_map(tbl_gen_current, &cell_table_gen_free);

  // destroy cell tables.
  cell_table_destroy(tbl_gen_current);
  cell_table_destroy(tbl_gen_next);

  return 0;
}
