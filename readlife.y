%{
#include <stdlib.h>
#include "life.h"

long lineno = 1;

int yylex(void)
{
  int c = fgetc(infile);
  if (c=='\n')
    lineno++;
  if (c==EOF)
    return 0;
  else
    return c;
}

void yyerror (char const *s)
{
  fprintf (stderr, "%ld: %s\n", lineno, s);
  exit(1);
}
%}

%union {
  long num;
  Celllist *cp;
}

%type <num> digit unum num
%type <cp> line lines

%%
start: aheader lines { gen0 = $2; }
     ;

aheader:
        | '#''A''\n'
        ;

white:
     | white ' '
     ;

lines: { $$ = NULL; }
     | lines line { $$ = $2; $$->next = $1; }
     ;

line: white num white num white '\n' { $$ = newcell($2, $4, NULL); }
    ;

num: '-' unum { $$ = - $2; }
   | unum     { $$ = $1; }
   ;

unum: digit      { $$ = $1; }
    | unum digit { $$ = $1 * 10 + $2; }
    ;

digit: '0' { $$ = 0; }
     | '1' { $$ = 1; }
     | '2' { $$ = 2; }
     | '3' { $$ = 3; }
     | '4' { $$ = 4; }
     | '5' { $$ = 5; }
     | '6' { $$ = 6; }
     | '7' { $$ = 7; }
     | '8' { $$ = 8; }
     | '9' { $$ = 9; }
     ;
%%

