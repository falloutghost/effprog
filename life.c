#include <stdlib.h>
#include <stdio.h>
#include "life.h"

FILE *infile;
Celllist *gen0;

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
  infile = f;
  if (yyparse() != 0) {
    fprintf(stderr, "syntax error");
    exit(1);
  }
  return gen0;
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
  current = readlife(stdin);
  for (i=0; i<generations; i++) {
    Celllist *old = current;
    current = onegeneration(current);
    freecelllist(old);
  }
  writelife(stdout, current);
  fprintf(stderr,"%ld cells alive\n", countcells(current));
  return 0;
}
