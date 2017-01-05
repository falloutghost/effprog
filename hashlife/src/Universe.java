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
        
//        double stepSize = Math.pow(2.0, this.root.level - 2);
//        System.out.println("stepSize: " + stepSize);
//        this.root = this.root.nextGeneration();
//        this.generationCount += stepSize;
    }

    public double getPopulation() {
        return this.root.population;
    }

    public String toString() {
        return "Generation: " + this.generationCount + "\n" +
                "Population: " + this.root.population;
    }

}
