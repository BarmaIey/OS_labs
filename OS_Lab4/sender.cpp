#include "common.h"

#include <iostream>

int main(int argc, char* argv[]) {
    std::string fileName;

    HANDLE mutexHandle;
    HANDLE emptySemaphore;
    HANDLE fullSemaphore;
    HANDLE readySemaphore;

    mutexHandle = NULL;
    emptySemaphore = NULL;
    fullSemaphore = NULL;
    readySemaphore = NULL;

    try {
        if (argc != 2) {
            throw std::runtime_error("Usage: Sender.exe <binary_file_name>");
        }

        fileName = argv[1];

        mutexHandle = OpenMutexA(
            SYNCHRONIZE | MUTEX_MODIFY_STATE,
            FALSE,
            getMutexName(fileName).c_str()
        );

        if (mutexHandle == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to open mutex."));
        }

        emptySemaphore = OpenSemaphoreA(
            SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
            FALSE,
            getEmptySemaphoreName(fileName).c_str()
        );

        if (emptySemaphore == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to open empty semaphore."));
        }

        fullSemaphore = OpenSemaphoreA(
            SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
            FALSE,
            getFullSemaphoreName(fileName).c_str()
        );

        if (fullSemaphore == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to open full semaphore."));
        }

        readySemaphore = OpenSemaphoreA(
            SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
            FALSE,
            getReadySemaphoreName(fileName).c_str()
        );

        if (readySemaphore == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to open ready semaphore."));
        }

        if (!ReleaseSemaphore(readySemaphore, 1, NULL)) {
            throw std::runtime_error(getLastErrorMessage("Failed to send ready signal."));
        }

        std::cout << "Sender is ready.\n";

        closeHandle(mutexHandle);
        closeHandle(emptySemaphore);
        closeHandle(fullSemaphore);
        closeHandle(readySemaphore);
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";

        closeHandle(mutexHandle);
        closeHandle(emptySemaphore);
        closeHandle(fullSemaphore);
        closeHandle(readySemaphore);

        return 1;
    }

    return 0;
}