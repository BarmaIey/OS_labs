package OS_Lab3_Java;

import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

public class MarkerThreadsLab {

    private static class MarkerData {
    int id;
    int markedCount;
    int blockedIndex;
    boolean terminated;

        MarkerData(int id) {
            this.id = id;
            this.markedCount = 0;
            this.blockedIndex = -1;
            this.terminated = false;
        }
    }

    private static class MarkerThread extends Thread {
        private final MarkerData data;
        private final int[] array;

        MarkerThread(MarkerData data, int[] array) {
            this.data = data;
            this.array = array;
        }

        @Override
        public void run() {
            System.out.println(
                "Marker thread #" + data.id + " started."
            );
        }
    }

    private static int readPositiveInt(
        Scanner scanner,
        String message
    ) {
        int value;

        while (true) {
            System.out.print(message);

            if (scanner.hasNextInt()) {
                value = scanner.nextInt();

                if (value > 0) {
                    return value;
                }
            } else {
                scanner.next();
            }

            System.out.println(
                "Invalid value. Please enter a positive integer."
            );
        }
    }

    private static void printArray(int[] array) {
        int i;

        System.out.print("Array: ");

        for (i = 0; i < array.length; i++) {
            System.out.print(array[i] + " ");
        }

        System.out.println();
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        int arraySize = readPositiveInt(scanner, "Enter array size: ");
        int markerCount = readPositiveInt(scanner, "Enter marker thread count: ");

        int[] array = new int[arraySize];

        List<MarkerData> markerDataList = new ArrayList<MarkerData>();
        List<MarkerThread> markerThreads = new ArrayList<MarkerThread>();

        for (int i = 0; i < markerCount; i++) {
            MarkerData data = new MarkerData(i + 1);
            MarkerThread thread = new MarkerThread(data, array);

            markerDataList.add(data);
            markerThreads.add(thread);

            thread.start();
        }

        scanner.close();
    }
}
