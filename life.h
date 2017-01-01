#ifndef LIFE_H
#define LIFE_H

#include <stdio.h>

typedef enum { DEAD, ALIVE } Status;

typedef struct point2d {
    long x, y;
} Point2D;

typedef struct cell {
    Point2D coordinates;
    Status status;
} Cell;

#endif
