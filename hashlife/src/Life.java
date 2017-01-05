import java.io.InputStream;
import java.util.Scanner;

public class Life {

    private static Universe universe = new Universe();

    public void readLife(InputStream inStream) {
        Scanner scanner = new Scanner(inStream);

        // TODO: skip headers (see grammar in readlife.y)

        while (scanner.hasNextLine()) {
            int x = scanner.nextInt();
            int y = scanner.nextInt();

            universe.setBit(x, y);

            scanner.nextLine();
        }
    }

    public static void main(String[] args) {
        // arguments checking.
        if (args.length != 1) {
            System.err.format("Usage: java %s #generations <startfile | sort >endfile%n", Life.class.getName());
        }

        // parse nr of generations.
        int generations = Integer.parseInt(args[0]);

        Life life = new Life();

        // read in initial generation.
        life.readLife(System.in);

        for (int i = 0;i<=generations;i++) {
            System.out.println(universe);
            universe.runStep();
        }
    }

}
