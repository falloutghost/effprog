/**
 * Represents a node in a quadtree.
 */
public class TreeNode {

    protected final TreeNode northWest;
    protected final TreeNode northEast;
    protected final TreeNode southWest;
    protected final TreeNode southEast;

    protected final int level;
    protected final boolean alive;
    protected final double population;

    /**
     * Constructor for a leaf node.
     *
     * @param alive Indicates whether this leaf represents an alive or a dead cell
     */
    public TreeNode(boolean alive) {
        this.northWest = null;
        this.northEast = null;
        this.southWest = null;
        this.southEast = null;

        this.level = 0;
        this.alive = alive;
        this.population = alive ? 1 : 0;
    }

    /**
     * Constructor that constructs a node given its four quadrants
     * (i.e., the node's children).
     *
     * @param northWest The north west quadrant
     * @param northEast The north east quadrant
     * @param southWest The south west quadrant
     * @param southEast The south east quadrant
     */
    public TreeNode(TreeNode northWest, TreeNode northEast, TreeNode southWest, TreeNode southEast) {
        this.northWest = northWest;
        this.northEast = northEast;
        this.southWest = southWest;
        this.southEast = southEast;

        this.level = northWest.level + 1;
        this.population = northWest.population + northEast.population +
                          southWest.population + southEast.population;

        this.alive = this.population > 0;
    }

    public static TreeNode createRoot() {
        return new TreeNode(false).createEmptyTree(3);
    }

    public TreeNode setBit(int x, int y) {
        if (this.level == 0) return new TreeNode(true);

        int offset = 1 << (level - 2);
        if (x < 0) { // west
            if (y < 0) { // south
                return new TreeNode(this.northWest.setBit(x + offset, y + offset),
                                    this.northEast,
                                    this.southWest,
                                    this.southEast);
            } else { // north
                return new TreeNode(this.northWest,
                                    this.northEast,
                                    this.southWest.setBit(x + offset, y - offset),
                                    this.southEast);
            }
        } else { // east
            if (y < 0) { // south
                return new TreeNode(this.northWest,
                                    this.northEast.setBit(x - offset, y + offset),
                                    this.southWest,
                                    this.southEast);
            } else { // north
                return new TreeNode(this.northWest,
                                    this.northEast,
                                    this.southWest,
                                    this.southEast.setBit(x - offset, y - offset));
            }
        }
    }

    public int getBit(int x, int y) {
        if (level == 0) return this.alive ? 1 : 0;
        int offset = 1 << (level - 2);
        if (x < 0) {
            if (y < 0) {
                return this.northWest.getBit(x + offset, y + offset);
            } else {
                return this.southWest.getBit(x + offset, y - offset);
            }
        } else {
            if (y < 0) {
                return this.northEast.getBit(x - offset, y + offset);
            } else {
                return this.southEast.getBit(x - offset, y - offset);
            }
        }
    }

    public TreeNode createEmptyTree(int level) {
        if (level == 0) return new TreeNode(false);
        TreeNode emptyTree = this.createEmptyTree(level - 1);
        return new TreeNode(emptyTree, emptyTree, emptyTree, emptyTree);
    }

    public TreeNode expandUniverse() {
        TreeNode border = this.createEmptyTree(this.level - 1);
        return new TreeNode(new TreeNode(border, border, border, this.northWest),
                            new TreeNode(border, border, this.northEast, border),
                            new TreeNode(border, this.southWest, border, border),
                            new TreeNode(this.southEast, border, border, border));
    }

    public TreeNode oneGeneration(int bitmask) {
        if (bitmask == 0) return new TreeNode(false);
        int self = (bitmask >> 5) & 1;
        int neighbourCount = 0;

        bitmask &= 0x757;
        while (bitmask != 0) {
            bitmask &= bitmask - 1;
            neighbourCount++;
        }

        if (neighbourCount == 3 || (neighbourCount == 2 && self != 0)) {
            return new TreeNode(true);
        } else {
            return new TreeNode(false);
        }
    }

    public TreeNode slowSimulation() {
        int allbits = 0;
        for (int y=-2;y<2;y++) {
            for (int x=-2;x<2;x++) {
                allbits = (allbits << 1) + this.getBit(x, y);
            }
        }

        return new TreeNode(this.oneGeneration(allbits >> 5), oneGeneration(allbits >> 4),
                            this.oneGeneration(allbits >> 1), oneGeneration(allbits));
    }

    public TreeNode centeredSubnode() {
        return new TreeNode(this.northWest.southEast, this.northEast.southWest,
                            this.southWest.northEast, this.southEast.northWest);
    }

    public TreeNode centeredHorizontal(TreeNode west, TreeNode east) {
        return new TreeNode(west.northEast.southEast, east.northWest.southWest,
                            west.southEast.northEast, east.southWest.northWest);
    }

    public TreeNode centeredVertical(TreeNode north, TreeNode south) {
        return new TreeNode(north.southWest.southEast, north.southEast.southWest,
                            south.northWest.northEast, south.northEast.northWest);
    }

    public TreeNode centeredSubSubnode() {
        return new TreeNode(this.northWest.southEast.southEast, this.northEast.southWest.southWest,
                            this.southWest.northEast.northEast, this.southEast.northWest.northWest);
    }

    public TreeNode nextGeneration() {
        if (population == 0) return this.northWest;
        if (level == 2) return this.slowSimulation();

        TreeNode n00 = this.northWest.centeredSubnode(),
                 n01 = this.centeredHorizontal(this.northWest, this.northEast),
                 n02 = this.northEast.centeredSubnode(),
                 n10 = this.centeredVertical(this.northWest, this.southWest),
                 n11 = this.centeredSubSubnode(),
                 n12 = this.centeredVertical(this.northEast, this.southEast),
                 n20 = this.southWest.centeredSubnode(),
                 n21 = this.centeredHorizontal(this.southWest, this.southEast),
                 n22 = this.southEast.centeredSubnode();

        return new TreeNode(new TreeNode(n00, n01, n10, n11).nextGeneration(),
                            new TreeNode(n01, n02, n11, n12).nextGeneration(),
                            new TreeNode(n10, n11, n20, n21).nextGeneration(),
                            new TreeNode(n11, n12, n21, n22).nextGeneration());
    }

}
