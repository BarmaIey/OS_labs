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

int main()
{
    std::string fileName;
    std::string pipeName;
    int clientCount;
    int i;
    int command;

    std::vector<employee> employees;
    std::vector<RecordLock> locks;

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
        }

        printEmployees(employees);

        for (i = 0; i < static_cast<int>(locks.size()); ++i) {
            destroyRecordLock(locks[static_cast<size_t>(i)]);
        }

        } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";
        return 1;
    }


    return 0;
}