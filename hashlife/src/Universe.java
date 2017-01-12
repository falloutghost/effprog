public class Universe {

    private double generationCount;
    private TreeNode root;

    public Universe() {
        this.generationCount = 0;
        this.root = TreeNode.createRoot();
    }

    public void setBit(int x, int y) {
        while (true) {
            int maxCoordinate = 1 << (this.root.level - 1);
            if (-maxCoordinate <= x && x <= maxCoordinate - 1 &&
                -maxCoordinate <= y && y <= maxCoordinate - 1) {
                break;
            }
            root = root.expandUniverse();
        }

        root = root.setBit(x, y);
    }

    public void runStep() {
        while (this.root.level < 3 ||
                this.root.northWest.population != this.root.northWest.southEast.southEast.population ||
                this.root.northEast.population != this.root.northEast.southWest.southWest.population ||
                this.root.southWest.population != this.root.southWest.northEast.northEast.population ||
                this.root.southEast.population != this.root.southEast.northWest.northWest.population) {
            this.root = this.root.expandUniverse();
        }

        this.root = root.nextGeneration();
        this.generationCount++;
    }

    public void runHashlifeStep() {
        while (this.root.level < 3 ||
                this.root.northWest.population != this.root.northWest.southEast.southEast.population ||
                this.root.northEast.population != this.root.northEast.southWest.southWest.population ||
                this.root.southWest.population != this.root.southWest.northEast.northEast.population ||
                this.root.southEast.population != this.root.southEast.northWest.northWest.population) {
            this.root = this.root.expandUniverse();
        }

        double stepSize = Math.pow(2.0, this.root.level - 2);
        System.out.println("stepSize: " + stepSize);
        this.root = this.root.nextHashlifeGeneration();
        this.generationCount += stepSize;
    }

    public double getPopulation() {
        return this.root.population;
    }

//    public int getBit(int x, int y) {
//        if (level == 0) {
//            return this.alive ? 1 : 0;
//        }
//        int offset = 1 << (level - 2);
//        if (x < 0) {
//            if (y < 0) {
//                return this.northWest.getBit(x + offset, y + offset);
//            } else {
//                return this.southWest.getBit(x + offset, y - offset);
//            }
//        } else {
//            if (y < 0) {
//                return this.northEast.getBit(x - offset, y + offset);
//            } else {
//                return this.southEast.getBit(x - offset, y - offset);
//            }
//        }
//    }

    public void traverse() {
        for (int x = 0;x<TreeNode.getSize();x++) {
            for (int y = 0;y<TreeNode.getSize();y++) {
                int bit = this.root.getBit(x, y);
                if (bit == 1) {
                    System.out.format("%1$d %2$d\n", x, y);
                }
            }
        }
    }

    public String toString() {
        return "Generation: " + this.generationCount + "\n" +
                "Population: " + this.root.population;
    }

}
