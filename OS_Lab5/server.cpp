#include "common.h"

#include <fstream>
#include <vector>

struct RecordLock {
    int readers;
    bool writer;
    CRITICAL_SECTION section;
    HANDLE canRead;
    HANDLE canWrite;
};

struct ClientThreadData {
    HANDLE pipe;
    std::string fileName;
    std::vector<employee>* employees;
    std::vector<RecordLock>* locks;
};

bool fileExists(const std::string& fileName) {
    std::ifstream file;

    file.open(fileName.c_str(), std::ios::binary);
    return file.good();
}

void initRecordLock(RecordLock& lock) {
    lock.readers = 0;
    lock.writer = false;
    InitializeCriticalSection(&lock.section);

    lock.canRead = CreateEventA(NULL, TRUE, TRUE, NULL);
    lock.canWrite = CreateEventA(NULL, TRUE, TRUE, NULL);

    if (lock.canRead == NULL || lock.canWrite == NULL) {
        throw std::runtime_error(getLastErrorMessage("Failed to create record lock events."));
    }
}

void destroyRecordLock(RecordLock& lock) {
    DeleteCriticalSection(&lock.section);
    closeHandle(lock.canRead);
    closeHandle(lock.canWrite);
}

void beginRead(RecordLock& lock) {
    while (true) {
        WaitForSingleObject(lock.canRead, INFINITE);

        EnterCriticalSection(&lock.section);

        if (!lock.writer) {
            ++lock.readers;
            ResetEvent(lock.canWrite);
            LeaveCriticalSection(&lock.section);
            return;
        }

        LeaveCriticalSection(&lock.section);
    }
}

void endRead(RecordLock& lock) {
    EnterCriticalSection(&lock.section);

    if (lock.readers > 0) {
        --lock.readers;
    }

    if (lock.readers == 0) {
        SetEvent(lock.canWrite);
    }

    LeaveCriticalSection(&lock.section);
}

void beginWrite(RecordLock& lock) {
    while (true) {
        WaitForSingleObject(lock.canWrite, INFINITE);

        EnterCriticalSection(&lock.section);

        if (!lock.writer && lock.readers == 0) {
            lock.writer = true;
            ResetEvent(lock.canRead);
            ResetEvent(lock.canWrite);
            LeaveCriticalSection(&lock.section);
            return;
        }

        LeaveCriticalSection(&lock.section);
    }
}

void endWrite(RecordLock& lock) {
    EnterCriticalSection(&lock.section);

    lock.writer = false;

    SetEvent(lock.canRead);
    SetEvent(lock.canWrite);

    LeaveCriticalSection(&lock.section);
}

void saveEmployeesToFile(const std::string& fileName, const std::vector<employee>& employees) {
    std::ofstream file;
    size_t i;

    file.open(fileName.c_str(), std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open employee file for writing.");
    }

    for (i = 0; i < employees.size(); ++i) {
        file.write(reinterpret_cast<const char*>(&employees[i]), sizeof(employee));
    }

    if (!file.good()) {
        throw std::runtime_error("Failed to write employee file.");
    }

    file.close();
}

void printEmployees(const std::vector<employee>& employees) {
    size_t i;

    std::cout << "\nEmployee file content:\n";

    for (i = 0; i < employees.size(); ++i) {
        printEmployee(employees[i]);
    }
}

int findEmployeeIndexById(const std::vector<employee>& employees, int id) {
    size_t i;

    for (i = 0; i < employees.size(); ++i) {
        if (employees[i].num == id) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

void sendError(HANDLE pipe, const std::string& text) {
    PipeMessage response;

    clearMessage(response);
    response.status = RESPONSE_ERROR;
    copyStringToCharArray(response.text, 128, text);

    writePipeMessage(pipe, response);
}

DWORD WINAPI clientThread(LPVOID parameter) {
    ClientThreadData* data;
    PipeMessage request;
    PipeMessage response;
    int index;
    bool hasReadLock;
    bool hasWriteLock;
    int lockedIndex;

    data = static_cast<ClientThreadData*>(parameter);

    hasReadLock = false;
    hasWriteLock = false;
    lockedIndex = -1;

    try {
        while (true) {
            if (!readPipeMessage(data->pipe, request)) {
                break;
            }

            if (request.command == REQUEST_EXIT) {
                break;
            }

            if (request.command == REQUEST_READ) {
                index = findEmployeeIndexById(*(data->employees), request.id);

                if (index == -1) {
                    sendError(data->pipe, "Employee was not found.");
                    continue;
                }

                beginRead((*(data->locks))[static_cast<size_t>(index)]);

                hasReadLock = true;
                hasWriteLock = false;
                lockedIndex = index;

                clearMessage(response);
                response.status = RESPONSE_OK;
                response.data = (*(data->employees))[static_cast<size_t>(index)];

                if (!writePipeMessage(data->pipe, response)) {
                    break;
                }
            } else if (request.command == REQUEST_RELEASE_READ) {
                if (hasReadLock && lockedIndex >= 0) {
                    endRead((*(data->locks))[static_cast<size_t>(lockedIndex)]);
                }

                hasReadLock = false;
                lockedIndex = -1;

                clearMessage(response);
                response.status = RESPONSE_OK;
                copyStringToCharArray(response.text, 128, "Read access was released.");
                writePipeMessage(data->pipe, response);
            } else if (request.command == REQUEST_WRITE) {
                index = findEmployeeIndexById(*(data->employees), request.id);

                if (index == -1) {
                    sendError(data->pipe, "Employee was not found.");
                    continue;
                }

                beginWrite((*(data->locks))[static_cast<size_t>(index)]);

                hasWriteLock = true;
                hasReadLock = false;
                lockedIndex = index;

                clearMessage(response);
                response.status = RESPONSE_OK;
                response.data = (*(data->employees))[static_cast<size_t>(index)];

                if (!writePipeMessage(data->pipe, response)) {
                    break;
                }
            } else if (request.command == REQUEST_RELEASE_WRITE) {
                if (hasWriteLock && lockedIndex >= 0) {
                    (*(data->employees))[static_cast<size_t>(lockedIndex)] = request.data;
                    saveEmployeesToFile(data->fileName, *(data->employees));
                    endWrite((*(data->locks))[static_cast<size_t>(lockedIndex)]);
                }

                hasWriteLock = false;
                lockedIndex = -1;

                clearMessage(response);
                response.status = RESPONSE_OK;
                copyStringToCharArray(response.text, 128, "Write access was released.");
                writePipeMessage(data->pipe, response);
            } else {
                sendError(data->pipe, "Unknown request.");
            }
        }

        if (hasReadLock && lockedIndex >= 0) {
            endRead((*(data->locks))[static_cast<size_t>(lockedIndex)]);
        }

        if (hasWriteLock && lockedIndex >= 0) {
            endWrite((*(data->locks))[static_cast<size_t>(lockedIndex)]);
        }

        FlushFileBuffers(data->pipe);
        DisconnectNamedPipe(data->pipe);
        closeHandle(data->pipe);

        delete data;
    } catch (const std::exception& exception) {
        std::cerr << "Client thread error: " << exception.what() << "\n";

        if (hasReadLock && lockedIndex >= 0) {
            endRead((*(data->locks))[static_cast<size_t>(lockedIndex)]);
        }

        if (hasWriteLock && lockedIndex >= 0) {
            endWrite((*(data->locks))[static_cast<size_t>(lockedIndex)]);
        }

        if (data != NULL) {
            closeHandle(data->pipe);
            delete data;
        }

        return 1;
    }

    return 0;
}

std::string makeClientCommandLine(const std::string& pipeName) {
    std::string commandLine;

    commandLine = "Client.exe \"";
    commandLine += pipeName;
    commandLine += "\"";

    return commandLine;
}

void createInitialEmployeeFile(
    const std::string& fileName,
    std::vector<employee>& employees
) {
    int count;
    int i;
    std::string name;

    count = readPositiveInt("Enter employee count: ");

    employees.resize(static_cast<size_t>(count));

    for (i = 0; i < count; ++i) {
        clearEmployee(employees[static_cast<size_t>(i)]);

        std::cout << "\nEmployee #" << i + 1 << "\n";

        employees[static_cast<size_t>(i)].num = readPositiveInt("Enter employee ID: ");

        std::cout << "Enter employee name, max 9 characters: ";
        std::getline(std::cin, name);

        while (name.empty() || name.size() >= NAME_SIZE) {
            std::cout << "Invalid name. Enter name, max 9 characters: ";
            std::getline(std::cin, name);
        }

        copyStringToCharArray(employees[static_cast<size_t>(i)].name, NAME_SIZE, name);

        std::cout << "Enter employee hours: ";
        std::cin >> employees[static_cast<size_t>(i)].hours;

        while (std::cin.fail() || employees[static_cast<size_t>(i)].hours < 0.0) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid hours. Enter non-negative number: ";
            std::cin >> employees[static_cast<size_t>(i)].hours;
        }

        std::cin.ignore(10000, '\n');
    }

    saveEmployeesToFile(fileName, employees);
}

int main() {
    std::string fileName;
    std::string pipeName;
    int clientCount;
    int initializedLockCount = 0;
    int i;
    int command;

    std::vector<employee> employees;
    std::vector<RecordLock> locks;
    std::vector<HANDLE> serverThreads;
    std::vector<PROCESS_INFORMATION> clientProcesses;
    std::vector<STARTUPINFOA> startupInfos;

    try {
        pipeName = "\\\\.\\pipe\\EmployeePipeLab";

        std::cout << "Enter binary file name: ";
        std::getline(std::cin, fileName);

        if (fileName.empty()) {
            throw std::runtime_error("File name cannot be empty.");
        }

        if (fileExists(fileName)) {
            throw std::runtime_error("File already exists. Please remove it or choose another name.");
        }

        createInitialEmployeeFile(fileName, employees);

        locks.resize(employees.size());

        for (i = 0; i < static_cast<int>(locks.size()); ++i) {
            initRecordLock(locks[static_cast<size_t>(i)]);
            ++initializedLockCount;
        }

        printEmployees(employees);

        clientCount = readPositiveInt("\nEnter client process count: ");

        serverThreads.resize(static_cast<size_t>(clientCount), NULL);
        clientProcesses.resize(static_cast<size_t>(clientCount));
        startupInfos.resize(static_cast<size_t>(clientCount));

        for (i = 0; i < clientCount; ++i) {
            HANDLE pipe;
            ClientThreadData* threadData;
            DWORD threadId;
            std::string commandLine;

            pipe = CreateNamedPipeA(
                pipeName.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                clientCount,
                PIPE_BUFFER_SIZE,
                PIPE_BUFFER_SIZE,
                0,
                NULL
            );

            if (pipe == INVALID_HANDLE_VALUE) {
                throw std::runtime_error(getLastErrorMessage("Failed to create named pipe."));
            }

            ZeroMemory(&clientProcesses[static_cast<size_t>(i)], sizeof(PROCESS_INFORMATION));
            ZeroMemory(&startupInfos[static_cast<size_t>(i)], sizeof(STARTUPINFOA));
            startupInfos[static_cast<size_t>(i)].cb = sizeof(STARTUPINFOA);

            commandLine = makeClientCommandLine(pipeName);

            if (!CreateProcessA(
                    NULL,
                    const_cast<char*>(commandLine.c_str()),
                    NULL,
                    NULL,
                    FALSE,
                    CREATE_NEW_CONSOLE,
                    NULL,
                    NULL,
                    &startupInfos[static_cast<size_t>(i)],
                    &clientProcesses[static_cast<size_t>(i)])) {
                closeHandle(pipe);
                throw std::runtime_error(getLastErrorMessage("Failed to create client process."));
            }

            if (!ConnectNamedPipe(pipe, NULL)) {
                if (GetLastError() != ERROR_PIPE_CONNECTED) {
                    closeHandle(pipe);
                    throw std::runtime_error(getLastErrorMessage("Failed to connect named pipe."));
                }
            }

            threadData = new ClientThreadData;
            threadData->pipe = pipe;
            threadData->fileName = fileName;
            threadData->employees = &employees;
            threadData->locks = &locks;

            serverThreads[static_cast<size_t>(i)] = CreateThread(
                NULL,
                0,
                clientThread,
                threadData,
                0,
                &threadId
            );

            if (serverThreads[static_cast<size_t>(i)] == NULL) {
                closeHandle(pipe);
                delete threadData;
                throw std::runtime_error(getLastErrorMessage("Failed to create server thread."));
            }
        }

        std::cout << "\nAll client processes were started.\n";

        while (true) {
            std::cout << "\n1 - print file\n";
            std::cout << "2 - exit server\n";
            std::cout << "Enter command: ";
            std::cin >> command;

            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                std::cout << "Invalid command.\n";
                continue;
            }

            if (command == 1) {
                printEmployees(employees);
            } else if (command == 2) {
                break;
            } else {
                std::cout << "Unknown command.\n";
            }
        }

        std::cout << "Waiting for clients to finish...\n";

        WaitForMultipleObjects(
            static_cast<DWORD>(serverThreads.size()),
            &serverThreads[0],
            TRUE,
            INFINITE
        );

        printEmployees(employees);

        for (i = 0; i < static_cast<int>(serverThreads.size()); ++i) {
            closeHandle(serverThreads[static_cast<size_t>(i)]);
        }

        for (i = 0; i < static_cast<int>(clientProcesses.size()); ++i) {
            closeHandle(clientProcesses[static_cast<size_t>(i)].hProcess);
            closeHandle(clientProcesses[static_cast<size_t>(i)].hThread);
        }

        for (i = 0; i < static_cast<int>(locks.size()); ++i) {
            destroyRecordLock(locks[static_cast<size_t>(i)]);
        }

        std::cout << "Server finished.\n";
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";

        for (i = 0; i < static_cast<int>(serverThreads.size()); ++i) {
            closeHandle(serverThreads[static_cast<size_t>(i)]);
        }

        for (i = 0; i < static_cast<int>(clientProcesses.size()); ++i) {
            closeHandle(clientProcesses[static_cast<size_t>(i)].hProcess);
            closeHandle(clientProcesses[static_cast<size_t>(i)].hThread);
        }

        for (i = 0; i < initializedLockCount; ++i) {
            destroyRecordLock(locks[static_cast<size_t>(i)]);
        }

        return 1;
    }

    return 0;
}