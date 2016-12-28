#include <stdio.h>

typedef struct celllist {
  long x, y;
  struct celllist *next;
} Celllist;

extern Celllist *gen0;
extern FILE *infile;

int yyparse(void);
Celllist *newcell(long x, long y, Celllist *l);
