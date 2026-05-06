#include <windows.h>

#include <iostream>
#include <sstream>

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


int main() {

    
    return 0;
}