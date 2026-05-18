package OS_Lab3_Java;

import java.util.Scanner;

public class MarkerThreadsLab {
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
        Scanner scanner;

        int arraySize;
        int markerCount;

        int[] array;

        scanner = new Scanner(System.in);

        arraySize = readPositiveInt(
            scanner,
            "Enter array size: "
        );

        markerCount = readPositiveInt(
            scanner,
            "Enter marker thread count: "
        );

        array = new int[arraySize];

        printArray(array);

        scanner.close();
    }
}
