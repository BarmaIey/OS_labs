#include "common.h"

#include <cstring>
#include <fstream>

void writeMessageToFile(const std::string& fileName, const std::string& message) {
    std::fstream file;
    FileHeader header;
    MessageRecord record;
    long position;
    int i;

    for (i = 0; i < MESSAGE_SIZE; ++i) {
        record.text[i] = '\0';
    }

    for (i = 0; i < static_cast<int>(message.size()) && i < MESSAGE_SIZE - 1; ++i) {
        record.text[i] = message[static_cast<size_t>(i)];
    }

    file.open(fileName.c_str(), std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open binary file for writing.");
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(FileHeader));

    position = static_cast<long>(sizeof(FileHeader))
        + static_cast<long>(header.writeIndex) * static_cast<long>(sizeof(MessageRecord));

    file.seekp(position, std::ios::beg);
    file.write(reinterpret_cast<char*>(&record), sizeof(MessageRecord));

    header.writeIndex = (header.writeIndex + 1) % header.maxRecords;

    file.seekp(0, std::ios::beg);
    file.write(reinterpret_cast<char*>(&header), sizeof(FileHeader));

    if (!file.good()) {
        throw std::runtime_error("Failed to write message to file.");
    }

    file.close();
}

int main(int argc, char* argv[]) {
    std::string fileName;
    std::string message;
    int command;

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

        while (true) {
            std::cout << "\n1 - send message\n";
            std::cout << "2 - exit\n";
            std::cout << "Enter command: ";
            std::cin >> command;

            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                std::cout << "Invalid command.\n";
                continue;
            }

            std::cin.ignore(10000, '\n');

            if (command == 1) {
                std::cout << "Enter message, max 19 characters: ";
                std::getline(std::cin, message);

                if (message.empty()) {
                    std::cout << "Message cannot be empty.\n";
                    continue;
                }

                if (message.size() >= MESSAGE_SIZE) {
                    std::cout << "Message is too long. Maximum is 19 characters.\n";
                    continue;
                }

                if (WaitForSingleObject(emptySemaphore, INFINITE) != WAIT_OBJECT_0) {
                    throw std::runtime_error(getLastErrorMessage("Failed to wait empty semaphore."));
                }

                if (WaitForSingleObject(mutexHandle, INFINITE) != WAIT_OBJECT_0) {
                    throw std::runtime_error(getLastErrorMessage("Failed to wait mutex."));
                }

                writeMessageToFile(fileName, message);

                if (!ReleaseMutex(mutexHandle)) {
                    throw std::runtime_error(getLastErrorMessage("Failed to release mutex."));
                }

                if (!ReleaseSemaphore(fullSemaphore, 1, NULL)) {
                    throw std::runtime_error(getLastErrorMessage("Failed to release full semaphore."));
                }

                std::cout << "Message sent.\n";
            } else if (command == 2) {
                break;
            } else {
                std::cout << "Unknown command.\n";
            }
        }

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