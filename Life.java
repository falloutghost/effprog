import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Scanner;

public class Life {

    /**
     * A class representing 2D points.
     */
    class Point2D {
        /**
         * value of X coordinate.
         */
        long x;
        /**
         * value of Y coordinate.
         */
        long y;

        /**
         * Constructor.
         * @param x X coordinate.
         * @param y Y coordinate.
         */
        public Point2D (long x, long y) {
            this.x = x;
            this.y = y;
        }

        /**
         * Equals implementation.
         * @param object the object to compare.
         * @return true if the objects are considered equal, false otherwise.
         */
        public boolean equals(Object object) {
            if (this == object) return true;
            if (object == null || getClass() != object.getClass()) return false;

            Point2D point = (Point2D) object;

            if (x != point.x) return false;
            if (y != point.y) return false;

            return true;
        }

        /**
         * Hash code implementation.
         * @return the hash code.
         */
        public int hashCode() {
            int result = 0;
            result = 31 * result + (int) (x ^ (x >>> 32));
            result = 31 * result + (int) (y ^ (y >>> 32));
            return result;
        }
    }

    /**
     * Enum for cell status.
     */
    enum Status {
        DEAD, ALIVE;
    }

    /**
     * A class representing a cell.
     */
    class Cell {
        /**
         * The coordinates.
         */
        Point2D coordinates;
        /**
         * The cell's status.
         */
        Status status;

        /**
         * Constructor.
         * @param coordinates the cell's coordinates.
         * @param status the cell's status.
         */
        public Cell(Point2D coordinates, Status status) {
            this.coordinates = coordinates;
            this.status = status;
        }
    }

    /**
     * Hash map for current generation.
     */
    private HashMap<Point2D, Cell> genCurrent;

    /**
     * Hash map used for building the next generation.
     */
    private HashMap<Point2D, Cell> genNext;

    /**
     * Hash set used to store already checked coordinates for the current generation.
     */
    HashSet<Point2D> checked;


    /**
     * Constructor.
     */
    public Life() {
        this.genCurrent = new HashMap<>(2048);
        this.genNext = new HashMap<>(2048);
    }

    /**
     * Reads the initial cell generation from an input stream into the current generation map.
     * @param inStream the input stream.
     */
    private void readLife(InputStream inStream) {
        Scanner scanner = new Scanner(inStream);

        // TODO: skip headers (see grammar in readlife.y)

        while (scanner.hasNextLine()) {
            long x = scanner.nextLong();
            long y = scanner.nextLong();

            Cell c = new Cell(new Point2D(x, y), Status.ALIVE);
            genCurrent.put(c.coordinates, c);

            scanner.nextLine();
        }
    }

    /**
     * Writes the current cell generation to an output stream.
     * @param outStream the output stream.
     */
    private void writeLife(OutputStream outStream) {
        PrintWriter writer = new PrintWriter(outStream);
        for (Point2D p : genCurrent.keySet()) {
            writer.format("%d %d%n", p.x, p.y);
        }
        writer.flush();
    }

    /**
     * Determines whether a cell at (x, y) is alive.
     * @param x the X coordinate.
     * @param y the Y coordinate.
     * @return 1 if the cell is alive, 0 otherwise.
     */
    private int alive(long x, long y) {
        return genCurrent.containsKey(new Point2D(x, y)) ? 1 : 0;
    }

    /**
     * Checks if a cell is alive in the next generation, and if so put the cell into the next generation map.
     * @param x the X coordinate of the cell.
     * @param y the Y coordinate of the cell.
     */
    private void checkCell(long x, long y) {
        Point2D coordinates = new Point2D(x, y);
        if (checked.contains(coordinates)) return;

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
            Cell c = new Cell(coordinates, Status.ALIVE);
            genNext.put(coordinates, c);
            checked.add(coordinates);
        }
    }

    /**
     * Returns the number of alive cells in the current generation.
     * @return the number of alive cells in the current generation.
     */
    private int countCells() {
        return genCurrent.size();
    }

    /**
     * Advance the current generation.
     */
    private void oneGeneration() {
        checked = new HashSet<Point2D>();

        for (Point2D p : genCurrent.keySet()) {
            checkCell(p.x - 1, p.y - 1);
            checkCell(p.x - 1, p.y + 0);
            checkCell(p.x - 1, p.y + 1);
            checkCell(p.x + 0, p.y - 1);
            checkCell(p.x + 0, p.y + 0);
            checkCell(p.x + 0, p.y + 1);
            checkCell(p.x + 1, p.y - 1);
            checkCell(p.x + 1, p.y + 0);
            checkCell(p.x + 1, p.y + 1);
        }

        HashMap<Point2D, Cell> genTmp = genCurrent;
        genCurrent = genNext;
        genNext = genTmp;

        genNext.clear();
        checked.clear();
    }

    /**
     * main().
     * @param args cmd line arguments
     */
    public static void main(String[] args) {
        // arguments checking.
        if (args.length != 1) {
            System.err.format("Usage: java %s #generations <startfile | sort >endfile%n", Life.class.getName());
        }

        // parse nr of generations.
        long generations = Long.parseLong(args[0]);

        Life life = new Life();

        // read in initial generation.
        life.readLife(System.in);

        // advance generations.
        for (long i = 0; i < generations; ++i) {
            life.oneGeneration();
        }

        life.writeLife(System.out);
        System.err.format("%d cells alive%n", life.countCells());
    }
}
