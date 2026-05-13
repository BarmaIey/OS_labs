#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

const int NAME_SIZE = 10;
const int PIPE_BUFFER_SIZE = 512;

const int REQUEST_READ = 1;
const int REQUEST_WRITE = 2;
const int REQUEST_EXIT = 3;
const int REQUEST_RELEASE_READ = 4;
const int REQUEST_RELEASE_WRITE = 5;

const int RESPONSE_OK = 1;
const int RESPONSE_ERROR = 2;


struct employee {
    int num;
    char name[10];
    double hours;
};

struct PipeMessage {
    int command;
    int status;
    int id;
    employee data;
    char text[128];
};

std::string getLastErrorMessage(const std::string& prefix) {
    DWORD errorCode;
    std::ostringstream stream;

    errorCode = GetLastError();
    stream << prefix << " Error code: " << errorCode;

    return stream.str();
}

int readPositiveInt(const std::string& message) {
    int value;

    value = 0;

    while (true) {
        std::cout << message;
        std::cin >> value;

        if (!std::cin.fail() && value > 0) {
            std::cin.ignore(10000, '\n');
            return value;
        }

        std::cin.clear();
        std::cin.ignore(10000, '\n');

        std::cout << "Invalid value. Please enter a positive integer.\n";
    }
}

void closeHandle(HANDLE& handle) {
    if (handle != NULL && handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
        handle = NULL;
    }
}

void clearEmployee(employee& item) {
    int i;

    item.num = 0;
    item.hours = 0.0;

    for (i = 0; i < NAME_SIZE; ++i) {
        item.name[i] = '\0';
    }
}

void clearMessage(PipeMessage& message) {
    int i;

    message.command = 0;
    message.status = 0;
    message.id = 0;
    clearEmployee(message.data);

    for (i = 0; i < 128; ++i) {
        message.text[i] = '\0';
    }
}

void copyStringToCharArray(char* destination, int destinationSize, const std::string& source) {
    int i;

    for (i = 0; i < destinationSize; ++i) {
        destination[i] = '\0';
    }

    for (i = 0; i < destinationSize - 1 && i < static_cast<int>(source.size()); ++i) {
        destination[i] = source[static_cast<size_t>(i)];
    }
}

void printEmployee(const employee& item) {
    std::cout << "ID: " << item.num
              << ", name: " << item.name
              << ", hours: " << item.hours << "\n";
}

#endif