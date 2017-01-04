
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <unordered_map>

/**
 * Enum for the cell status.
 */
enum Status { DEAD, ALIVE };

/**
 * A class representing a 2D point.
 */
class Point2D
{
public:
    /**
     * The X coordinate.
     */
    long x;

    /**
     * The Y coordinate.
     */
    long y;

    /**
     * Constructor.
     */
    Point2D() {}

    /**
     * Constructor
     * @param x the X coordinate.
     * @param y the Y coordinate.
     */
    Point2D(long x, long y) {
        this->x = x;
        this->y = y;
    }

    bool operator==(const Point2D &other) const {
        return (this->x == other.x && this->y == other.y);
    }
};

/**
 * Fowler-Noll-Vo 32-bit constants
 * @see https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 */
#define FNV_32_PRIME 16777619u
#define FNV_32_BASIS 2166136261u

/**
 * FNV-based hash implementation for class Point2D.
 */
class Point2DHash
{
public:
    size_t operator() (Point2D const& key) const
    {
        unsigned int hash;
        size_t size = sizeof(Point2D);
        unsigned char *key_ptr = (unsigned char *)&key;

        hash = FNV_32_BASIS;
        while (size-- > 0)
            hash = (hash * FNV_32_PRIME) ^ *key_ptr++;

        return hash;
    }
};

/**
 * A class representing a cell.
 */
class Cell
{
public:
    /**
     * The cell's coordinates.
     */
    Point2D coordinates;

    /**
     * The cell's status.
     */
    Status status;

    /**
     * Constructor.
     */
    Cell() {}

    /**
     * Constructor.
     * @param coordinates the cell's coordinates.
     * @param status the cell's status.
     */
    Cell(Point2D coordinates, Status status) {
        this->coordinates = coordinates;
        this->status = status;
    }
};

/**
 * Game-of-life implementation.
 */
class Life
{
public:
    /**
     * A map containing the cells of the current generation.
     */
    std::unordered_map <Point2D, Cell*, Point2DHash> gen_current;

    /**
     * A map used to generate the next generation.
     */
    std::unordered_map <Point2D, Cell*, Point2DHash> gen_next;

    Life() {
        gen_current.rehash(1024);
        gen_current.max_load_factor(0.75f);
        gen_next.rehash(1024);
        gen_next.max_load_factor(0.75f);
    }

    /**
     * Reads the initial cell generation from an input stream into the current generation map.
     * @param in the input stream.
     */
    void readlife(std::istream& in)
    {
        // TODO: skip headers (see grammar in readlife.y)

        Point2D p;

        while (!in.eof()) {
            in >> std::skipws >> p.x;
            in >> std::skipws >> p.y;
            in >> std::skipws;

            Cell *c = new Cell(p, ALIVE);
            gen_current[c->coordinates] = c;
        }
    }

    /**
     * Writes the current cell generation to an output stream.
     * @param outStream the output stream.
     */
    void writelife(std::ostream& out) {
        std::unordered_map<Point2D, Cell*, Point2DHash>::iterator iter;
        for (iter = gen_current.begin(); iter != gen_current.end(); ++iter) {
            out << iter->first.x << " " << iter->first.y << "\n";
        }
    }

    /**
     * Returns the number of alive cells in the current generation.
     * @return the number of alive cells in the current generation.
     */
    int countcells()
    {
        return gen_current.size();
    }

    /**
     * Advance the current generation.
     */
    void onegeneration() {
        std::unordered_map<Point2D, Cell*, Point2DHash>::iterator iter;
        for (iter = gen_current.begin(); iter != gen_current.end(); ++iter) {
            const Point2D &p = iter->first;
            checkcell(p.x-1, p.y-1);
            checkcell(p.x-1, p.y+0);
            checkcell(p.x-1, p.y+1);
            checkcell(p.x+0, p.y-1);
            checkcell(p.x+0, p.y+0);
            checkcell(p.x+0, p.y+1);
            checkcell(p.x+1, p.y-1);
            checkcell(p.x+1, p.y+0);
            checkcell(p.x+1, p.y+1);
        }

        gen_current.swap(gen_next);

        for (iter = gen_next.begin(); iter != gen_next.end(); ++iter) {
            free(iter->second);
        }
        gen_next.clear();
    }

private:
    /**
     * Determines whether a cell at (x, y) is alive.
     * @param x the X coordinate.
     * @param y the Y coordinate.
     * @return 1 if the cell is alive, 0 otherwise.
     */
    int alive(long x, long y) {
        Point2D p(x, y);
        return gen_current.find(p) != gen_current.end();
    }

    /**
     * Checks if a cell is alive in the next generation, and if so put the cell into the next generation map.
     * @param x the X coordinate of the cell.
     * @param y the Y coordinate of the cell.
     */
     void checkcell(long x, long y) {
        int n = 0;

        n += alive(x-1, y-1);
        n += alive(x-1, y+0);
        n += alive(x-1, y+1);
        n += alive(x+0, y-1);
        n += alive(x+0, y+1);
        n += alive(x+1, y-1);
        n += alive(x+1, y+0);
        n += alive(x+1, y+1);

        if (n == 3 || (n == 2 && alive(x, y) == 1)) {
            Point2D p(x, y);
            Cell *c = new Cell(p, ALIVE);
            gen_next[c->coordinates] = c;
        }
    }
};

int main(int argc, char **argv)
{
    // arguments checking.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s #generations <startfile | sort >endfile\n", argv[0]);
        exit(1);
    }

    // parse nr of generations.
    char *endptr;
    long generations = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "\"%s\" not a valid generation count\n", argv[3]);
        exit(1);
    }

    Life *life = new Life();

    // read in initial generation.
    life->readlife(std::cin);

    // advance generations.
    for (long i=0; i<generations; i++) {
        life->onegeneration();
    }

    life->writelife(std::cout);
    fprintf(stderr, "%d cells alive\n", life->countcells());

    return 0;
}
