#include "common.h"

#include <fstream>
#include <iostream>
#include <vector>

bool fileExists(const std::string& fileName) {
    std::ifstream file;

    file.open(fileName.c_str(), std::ios::binary);
    return file.good();
}

void createMessageFile(const std::string& fileName, int maxRecords) {
    std::ofstream file;
    FileHeader header;
    MessageRecord emptyRecord;
    int i;
    int j;

    header.maxRecords = maxRecords;
    header.readIndex = 0;
    header.writeIndex = 0;

    for (j = 0; j < MESSAGE_SIZE; ++j) {
        emptyRecord.text[j] = '\0';
    }

    file.open(fileName.c_str(), std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to create binary file.");
    }

    file.write(reinterpret_cast<char*>(&header), sizeof(FileHeader));

    for (i = 0; i < maxRecords; ++i) {
        file.write(reinterpret_cast<char*>(&emptyRecord), sizeof(MessageRecord));
    }

    if (!file.good()) {
        throw std::runtime_error("Failed to write initial file content.");
    }

    file.close();
}

std::string makeSenderCommandLine(const std::string& fileName) {
    std::string commandLine;

    commandLine = "Sender.exe \"";
    commandLine += fileName;
    commandLine += "\"";

    return commandLine;
}

int main() {
    std::string fileName;
    int recordCount;
    int senderCount;
    int i;
    int command;

    HANDLE mutexHandle;
    HANDLE emptySemaphore;
    HANDLE fullSemaphore;
    HANDLE readySemaphore;

    std::vector<PROCESS_INFORMATION> senderProcesses;
    std::vector<STARTUPINFOA> startupInfos;

    mutexHandle = NULL;
    emptySemaphore = NULL;
    fullSemaphore = NULL;
    readySemaphore = NULL;

    try {
        std::cout << "Enter binary file name: ";
        std::getline(std::cin, fileName);

        if (fileName.empty()) {
            throw std::runtime_error("File name cannot be empty.");
        }

        if (fileExists(fileName)) {
            throw std::runtime_error("File already exists. Please remove it or choose another name.");
        }

        recordCount = readPositiveInt("Enter record count: ");
        createMessageFile(fileName, recordCount);

        mutexHandle = CreateMutexA(NULL, FALSE, getMutexName(fileName).c_str());

        if (mutexHandle == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to create mutex."));
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            throw std::runtime_error("Synchronization mutex already exists.");
        }

        emptySemaphore = CreateSemaphoreA(
            NULL,
            recordCount,
            recordCount,
            getEmptySemaphoreName(fileName).c_str()
        );

        if (emptySemaphore == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to create empty semaphore."));
        }

        fullSemaphore = CreateSemaphoreA(
            NULL,
            0,
            recordCount,
            getFullSemaphoreName(fileName).c_str()
        );

        if (fullSemaphore == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to create full semaphore."));
        }

        readySemaphore = CreateSemaphoreA(
            NULL,
            0,
            1000,
            getReadySemaphoreName(fileName).c_str()
        );

        if (readySemaphore == NULL) {
            throw std::runtime_error(getLastErrorMessage("Failed to create ready semaphore."));
        }

                senderCount = readPositiveInt("Enter sender process count: ");

        senderProcesses.resize(static_cast<size_t>(senderCount));
        startupInfos.resize(static_cast<size_t>(senderCount));

        for (i = 0; i < senderCount; ++i) {
            ZeroMemory(&senderProcesses[i], sizeof(PROCESS_INFORMATION));
            ZeroMemory(&startupInfos[i], sizeof(STARTUPINFOA));
            startupInfos[i].cb = sizeof(STARTUPINFOA);

            std::string commandLine = makeSenderCommandLine(fileName);

            if (!CreateProcessA(
                    NULL,
                    const_cast<char*>(commandLine.c_str()),
                    NULL,
                    NULL,
                    FALSE,
                    CREATE_NEW_CONSOLE,
                    NULL,
                    NULL,
                    &startupInfos[i],
                    &senderProcesses[i])) {
                throw std::runtime_error(getLastErrorMessage("Failed to create sender process."));
            }
        }

        for (i = 0; i < senderCount; ++i) {
            if (WaitForSingleObject(readySemaphore, INFINITE) != WAIT_OBJECT_0) {
                throw std::runtime_error(getLastErrorMessage("Failed to wait sender readiness."));
            }
        }

        std::cout << "All senders are ready.\n";

        for (i = 0; i < senderCount; ++i) {
            closeHandle(senderProcesses[i].hProcess);
            closeHandle(senderProcesses[i].hThread);
        }

        closeHandle(mutexHandle);
        closeHandle(emptySemaphore);
        closeHandle(fullSemaphore);
        closeHandle(readySemaphore);
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";

        for (i = 0; i < senderCount; ++i) {
            closeHandle(senderProcesses[i].hProcess);
            closeHandle(senderProcesses[i].hThread);
        }

        closeHandle(mutexHandle);
        closeHandle(emptySemaphore);
        closeHandle(fullSemaphore);
        closeHandle(readySemaphore);

        return 1;
    }

    return 0;
}