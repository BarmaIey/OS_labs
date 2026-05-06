#include <windows.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct MarkerData {
    int id;
    int arraySize;
    int markedCount;
    int blockedIndex;
    bool terminate;

    int* array;
    CRITICAL_SECTION* criticalSection;

    HANDLE startEvent;
    HANDLE blockedEvent;
    HANDLE continueEvent;
};

std::string getLastErrorMessage(const std::string& prefix) {
    DWORD errorCode;
    std::ostringstream stream;

    errorCode = GetLastError();
    stream << prefix << " Error code: " << errorCode;

    return stream.str();
}

void initMarkerData(MarkerData& data) {
    data.id = 0;
    data.arraySize = 0;
    data.markedCount = 0;
    data.blockedIndex = -1;
    data.terminate = false;

    data.array = NULL;
    data.criticalSection = NULL;

    data.startEvent = NULL;
    data.blockedEvent = NULL;
    data.continueEvent = NULL;
}

int readPositiveInt(const std::string& message) {
    int value;

    value = 0;

    while (true) {
        std::cout << message;
        std::cin >> value;

        if (!std::cin.fail() && value > 0) {
            return value;
        }

        std::cin.clear();
        std::cin.ignore(10000, '\n');

        std::cout << "Invalid value. Please enter a positive integer.\n";
    }
}

void printArray(const std::vector<int>& array) {
    size_t i;

    std::cout << "Array: ";

    for (i = 0; i < array.size(); ++i) {
        std::cout << array[i] << ' ';
    }

    std::cout << "\n";
}

DWORD WINAPI markerThread(LPVOID parameter) {
    MarkerData* data;
    int randomNumber;
    int index;
    int i;
    DWORD waitResult;

    data = static_cast<MarkerData*>(parameter);

    try {
        waitResult = WaitForSingleObject(data->startEvent, INFINITE);

        if (waitResult != WAIT_OBJECT_0) {
            throw std::runtime_error(getLastErrorMessage("Failed to wait start event."));
        }

        srand(static_cast<unsigned int>(data->id));

        while (true) {
            randomNumber = rand();
            index = randomNumber % data->arraySize;

            EnterCriticalSection(data->criticalSection);

            if (data->array[index] == 0) {
                Sleep(5);

                data->array[index] = data->id;
                ++data->markedCount;

                Sleep(5);

                LeaveCriticalSection(data->criticalSection);
            } else {
                data->blockedIndex = index;

                std::cout << "Marker #" << data->id
                          << " blocked. Marked count: " << data->markedCount
                          << ", blocked index: " << data->blockedIndex << "\n";

                LeaveCriticalSection(data->criticalSection);

                if (!SetEvent(data->blockedEvent)) {
                    throw std::runtime_error(getLastErrorMessage("Failed to set blocked event."));
                }

                waitResult = WaitForSingleObject(data->continueEvent, INFINITE);

                if (waitResult != WAIT_OBJECT_0) {
                    throw std::runtime_error(getLastErrorMessage("Failed to wait continue event."));
                }

                if (data->terminate) {
                    EnterCriticalSection(data->criticalSection);

                    for (i = 0; i < data->arraySize; ++i) {
                        if (data->array[i] == data->id) {
                            data->array[i] = 0;
                        }
                    }

                    LeaveCriticalSection(data->criticalSection);

                    break;
                }
            }
        }
    } catch (const std::exception& exception) {
        std::cerr << "Marker thread error: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}

void closeHandles(std::vector<HANDLE>& handles) {
    size_t i;

    for (i = 0; i < handles.size(); ++i) {
        if (handles[i] != NULL) {
            CloseHandle(handles[i]);
            handles[i] = NULL;
        }
    }
}


int main() {
    int arraySize;
    int markerCount;
    int activeMarkers;
    int markerToStop;
    int i;

    CRITICAL_SECTION criticalSection;
    bool criticalSectionInitialized;

    std::vector<int> array;
    std::vector<MarkerData> markerData;

    std::vector<HANDLE> threads;
    std::vector<HANDLE> startEvents;
    std::vector<HANDLE> blockedEvents;
    std::vector<HANDLE> continueEvents;
    std::vector<HANDLE> currentBlockedEvents;

    HANDLE thread;
    DWORD waitResult;

    criticalSectionInitialized = false;

    try {
        arraySize = readPositiveInt("Enter array size: ");
        markerCount = readPositiveInt("Enter marker thread count: ");

        if (markerCount > MAXIMUM_WAIT_OBJECTS) {
            throw std::runtime_error("Too many marker threads. Maximum is 64.");
        }

        array.assign(static_cast<size_t>(arraySize), 0);

        markerData.resize(static_cast<size_t>(markerCount));
        threads.assign(static_cast<size_t>(markerCount), NULL);
        startEvents.assign(static_cast<size_t>(markerCount), NULL);
        blockedEvents.assign(static_cast<size_t>(markerCount), NULL);
        continueEvents.assign(static_cast<size_t>(markerCount), NULL);

        InitializeCriticalSection(&criticalSection);
        criticalSectionInitialized = true;

        for (i = 0; i < markerCount; ++i) {
            initMarkerData(markerData[i]);

            startEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
            blockedEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
            continueEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);

            if (startEvents[i] == NULL || blockedEvents[i] == NULL || continueEvents[i] == NULL) {
                throw std::runtime_error(getLastErrorMessage("Failed to create event."));
            }

            markerData[i].id = i + 1;
            markerData[i].arraySize = arraySize;
            markerData[i].array = &array[0];
            markerData[i].criticalSection = &criticalSection;
            markerData[i].startEvent = startEvents[i];
            markerData[i].blockedEvent = blockedEvents[i];
            markerData[i].continueEvent = continueEvents[i];

            thread = CreateThread(
                NULL,
                0,
                markerThread,
                &markerData[i],
                0,
                NULL
            );

            if (thread == NULL) {
                throw std::runtime_error(getLastErrorMessage("Failed to create marker thread."));
            }

            threads[i] = thread;
        }

        for (i = 0; i < markerCount; ++i) {
            if (!SetEvent(startEvents[i])) {
                throw std::runtime_error(getLastErrorMessage("Failed to set start event."));
            }
        }

        activeMarkers = markerCount;

        while (activeMarkers > 0) {
            currentBlockedEvents.clear();

            for (i = 0; i < markerCount; ++i) {
                if (!markerData[i].terminate) {
                    currentBlockedEvents.push_back(blockedEvents[i]);
                }
            }

            waitResult = WaitForMultipleObjects(
                static_cast<DWORD>(currentBlockedEvents.size()),
                &currentBlockedEvents[0],
                TRUE,
                INFINITE
            );

            if (waitResult == WAIT_FAILED) {
                throw std::runtime_error(getLastErrorMessage("Failed to wait marker events."));
            }

            printArray(array);

            std::cout << "Available markers: ";

            for (i = 0; i < markerCount; ++i) {
                if (!markerData[i].terminate) {
                    std::cout << markerData[i].id << ' ';
                }
            }

            std::cout << "\n";

            while (true) {
                std::cout << "Enter marker number to stop: ";
                std::cin >> markerToStop;

                if (!std::cin.fail()
                    && markerToStop >= 1
                    && markerToStop <= markerCount
                    && !markerData[markerToStop - 1].terminate) {
                    break;
                }

                std::cin.clear();
                std::cin.ignore(10000, '\n');

                std::cout << "Invalid marker number. Try again.\n";
            }

            markerData[markerToStop - 1].terminate = true;

            if (!SetEvent(markerData[markerToStop - 1].continueEvent)) {
                throw std::runtime_error(getLastErrorMessage("Failed to stop selected marker."));
            }

            waitResult = WaitForSingleObject(threads[markerToStop - 1], INFINITE);

            if (waitResult != WAIT_OBJECT_0) {
                throw std::runtime_error(getLastErrorMessage("Failed to wait selected marker thread."));
            }

            --activeMarkers;

            printArray(array);

            for (i = 0; i < markerCount; ++i) {
                if (!markerData[i].terminate) {
                    if (!SetEvent(markerData[i].continueEvent)) {
                        throw std::runtime_error(getLastErrorMessage("Failed to continue marker thread."));
                    }
                }
            }
        }

        closeHandles(threads);
        closeHandles(startEvents);
        closeHandles(blockedEvents);
        closeHandles(continueEvents);

        if (criticalSectionInitialized) {
            DeleteCriticalSection(&criticalSection);
            criticalSectionInitialized = false;
        }

        std::cout << "All marker threads finished.\n";
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";

        closeHandles(threads);
        closeHandles(startEvents);
        closeHandles(blockedEvents);
        closeHandles(continueEvents);

        if (criticalSectionInitialized) {
            DeleteCriticalSection(&criticalSection);
        }

        return 1;
    }

    return 0;
}