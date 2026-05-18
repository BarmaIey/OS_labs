package OS_Lab3_Java;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.Scanner;

public class MarkerThreadsLab {
    private static final Object arrayLock = new Object();
    private static final Object coordinatorLock = new Object();

    private static int blockedThreadCount = 0;

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
        private final Object commandLock;

        private boolean hasCommand;
        private boolean shouldTerminate;

        MarkerThread(MarkerData data, int[] array) {
            this.data = data;
            this.array = array;
            this.commandLock = new Object();
            this.hasCommand = false;
            this.shouldTerminate = false;
        }

        public void sendContinueSignal() {
            synchronized (commandLock) {
                hasCommand = true;
                shouldTerminate = false;
                commandLock.notify();
            }
        }

        public void sendTerminateSignal() {
            synchronized (commandLock) {
                hasCommand = true;
                shouldTerminate = true;
                commandLock.notify();
            }
        }

        @Override
        public void run() {
            Random random = new Random(data.id);

            try {
                while (true) {
                    int index = random.nextInt(array.length);

                    synchronized (arrayLock) {
                        if (array[index] == 0) {
                            Thread.sleep(5);

                            array[index] = data.id;
                            data.markedCount++;

                            Thread.sleep(5);
                            continue;
                        }

                        data.blockedIndex = index;

                        System.out.println(
                            "Marker #" + data.id
                            + " blocked. Marked count: " + data.markedCount
                            + ", blocked index: " + data.blockedIndex
                        );
                    }

                    synchronized (coordinatorLock) {
                        blockedThreadCount++;
                        coordinatorLock.notifyAll();
                    }

                    synchronized (commandLock) {
                        while (!hasCommand) {
                            commandLock.wait();
                        }

                        hasCommand = false;

                        if (shouldTerminate) {
                            synchronized (arrayLock) {
                                for (int i = 0; i < array.length; i++) {
                                    if (array[i] == data.id) {
                                        array[i] = 0;
                                    }
                                }
                            }

                            data.terminated = true;
                            break;
                        }
                    }
                }
            } catch (InterruptedException exception) {
                Thread.currentThread().interrupt();
                System.err.println("Marker thread was interrupted.");
            }
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
        synchronized (arrayLock) {
            System.out.print("Array: ");

            for (int i = 0; i < array.length; i++) {
                System.out.print(array[i] + " ");
            }

            System.out.println();
        }
    }

    private static void printAvailableMarkers(List<MarkerData> markerDataList) {
        System.out.print("Available markers: ");

        for (int i = 0; i < markerDataList.size(); i++) {
            MarkerData data = markerDataList.get(i);

            if (!data.terminated) {
                System.out.print(data.id + " ");
            }
        }

        System.out.println();
    }

    private static int readMarkerToStop(
        Scanner scanner,
        List<MarkerData> markerDataList
    ) {
        int markerToStop;

        while (true) {
            System.out.print("Enter marker number to stop: ");

            if (scanner.hasNextInt()) {
                markerToStop = scanner.nextInt();

                if (markerToStop >= 1
                    && markerToStop <= markerDataList.size()
                    && !markerDataList.get(markerToStop - 1).terminated) {
                    return markerToStop;
                }
            } else {
                scanner.next();
            }

            System.out.println("Invalid marker number. Try again.");
        }
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

        int activeMarkers = markerCount;

        try {
            while (activeMarkers > 0) {
                synchronized (coordinatorLock) {
                    while (blockedThreadCount < activeMarkers) {
                        coordinatorLock.wait();
                    }
                }

                printArray(array);
                printAvailableMarkers(markerDataList);

                int markerToStop = readMarkerToStop(scanner, markerDataList);
                MarkerThread selectedThread = markerThreads.get(markerToStop - 1);

                selectedThread.sendTerminateSignal();
                selectedThread.join();

                activeMarkers--;

                printArray(array);

                synchronized (coordinatorLock) {
                    blockedThreadCount = 0;
                }

                for (int i = 0; i < markerThreads.size(); i++) {
                    MarkerData data = markerDataList.get(i);

                    if (!data.terminated) {
                        markerThreads.get(i).sendContinueSignal();
                    }
                }
            }

            System.out.println("All marker threads finished.");
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            System.err.println("Main thread was interrupted.");
        }

        scanner.close();
    }
}
