#include <stdio.h>

typedef enum { ALIVE, DEAD } Status;

typedef struct {
    long x, y;
} Point;

typedef struct {
    Point coordinates;
    Status status;
} Cell;
