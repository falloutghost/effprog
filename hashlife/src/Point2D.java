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
