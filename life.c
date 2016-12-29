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

FILE *infile;
Celllist *gen0;

hash_table *hash_tbl;

static unsigned int
hash_point(void *val)
{
  Point *p = (Point *)val;
  return hash_long(&p->x) + hash_long(&p->y);
}

static int
hash_point_cmp(void *val1, void *val2)
{
  Point *p1 = (Point *)val1;
  Point *p2 = (Point *)val2;
  return ((p1->x - p2->x) == 0) ? (p1->y - p2->y) : (p1->x - p2->x);
}

long alive(long x, long y, Celllist *l)
{
  /*fprintf(stderr,"alive x=%ld y=%ld l=%p",x,y,l);*/
  for (; l; l = l->next) {
    /* fprintf(stderr,"alive x=%ld y=%ld l=(%ld,%ld)",x,y,l->x, l->y);*/
    if (x==l->x && y==l->y) {
      /*fprintf(stderr,"=1\n");*/
      return 1;
    }
  }
  /*fprintf(stderr,"=0\n");*/
  return 0;
}

Celllist *newcell(long x, long y, Celllist *l)
{
  Celllist *c = malloc(sizeof(Celllist));
  /*fprintf(stderr,"newcell x=%ld y=%ld\n",x,y);*/
  c->x = x;
  c->y = y;
  c->next = l;
  return c;
}

Celllist *checkcell(long x, long y, Celllist *old, Celllist *new)
{
  int n=0;
  if (alive(x,y,new))
    return new;
  n += alive(x-1, y-1, old);
  n += alive(x-1, y+0, old);
  n += alive(x-1, y+1, old);
  n += alive(x+0, y-1, old);
  n += alive(x+0, y+1, old);
  n += alive(x+1, y-1, old);
  n += alive(x+1, y+0, old);
  n += alive(x+1, y+1, old);
  /*fprintf(stderr,"checkcell x=%ld y=%ld old=%p new=%p n=%d\n",x,y,old,new,n);*/
  if (n==3)
    return newcell(x,y,new);
  else if (n==2 && alive(x,y,old))
    return newcell(x,y,new);
  else
    return new;
}

Celllist *onegeneration(Celllist *old)
{
  Celllist *new = NULL;
  Celllist *l;

  for (l=old; l; l = l->next) {
    long x = l->x;
    long y = l->y;
    new = checkcell(x-1, y-1, old, new);
    new = checkcell(x-1, y+0, old, new);
    new = checkcell(x-1, y+1, old, new);
    new = checkcell(x+0, y-1, old, new);
    new = checkcell(x+0, y+0, old, new);
    new = checkcell(x+0, y+1, old, new);
    new = checkcell(x+1, y-1, old, new);
    new = checkcell(x+1, y+0, old, new);
    new = checkcell(x+1, y+1, old, new);
  }
  return new;
}

void freecelllist(Celllist *l)
{
  while (l) {
    Celllist *old = l;
    l = l->next;
    free(old);
  }
}

Celllist *readlife(FILE *f)
{
  struct stat sb;
  int fd;
  long x, y;
  char *begin, *s, *end;

  Celllist *list = NULL;
  Cell *c = NULL;

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

    list = newcell(x, y, list);

    c = malloc(sizeof(Cell));
    if (c == NULL) {
      perror("malloc");
      exit(1);
    }
    c->coordinates.x = x;
    c->coordinates.y = y;

    hash_table_put(hash_tbl, &c->coordinates, c);

    while (*s == ' ' || *s == '\n') s++;
  }

  munmap(begin, sb.st_size);

  return list;
}

void writelife(FILE *f, Celllist *l)
{
  for (; l; l = l->next)
    fprintf(f, "%ld %ld\n", l->x, l->y);
}

long countcells(Celllist *l)
{
  long c=0;
  for (; l; l = l->next)
    c++;
  return c;
}

int main(int argc, char **argv)
{
  Celllist *current;
  long generations;
  long i;
  char *endptr;

  if (argc!=2) {
    fprintf(stderr, "Usage: %s #generations <startfile | sort >endfile\n", argv[0]);
    exit(1);
  }
  generations = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0') {
    fprintf(stderr, "\"%s\" not a valid generation count\n", argv[3]);
    exit(1);
  }

  hash_tbl = hash_table_create(512, hash_point, hash_point_cmp);

  current = readlife(stdin);

  fprintf(stderr, "DEBUG: hash table size: %d\n", hash_table_size(hash_tbl));
  fprintf(stderr, "DEBUG: list size: %ld\n", countcells(current));

  for (i=0; i<generations; i++) {
    Celllist *old = current;
    current = onegeneration(current);
    freecelllist(old);
  }
  writelife(stdout, current);
  fprintf(stderr,"%ld cells alive\n", countcells(current));

  hash_table_destroy(hash_tbl);

  return 0;
}
