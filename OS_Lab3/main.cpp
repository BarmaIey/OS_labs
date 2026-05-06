#include <windows.h>

#include <iostream>

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


int main() {
    
    return 0;
}