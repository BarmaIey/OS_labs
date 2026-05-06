#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

const int MESSAGE_SIZE = 20;

struct FileHeader {
    int maxRecords;
    int readIndex;
    int writeIndex;
};

struct MessageRecord {
    char text[MESSAGE_SIZE];
};

std::string getLastErrorMessage(const std::string& prefix) {
    DWORD errorCode;
    std::ostringstream stream;

    errorCode = GetLastError();
    stream << prefix << " Error code: " << errorCode;

    return stream.str();
}

std::string makeSafeName(const std::string& fileName) {
    std::string result;
    size_t i;
    char ch;

    result = "MsgLab_";

    for (i = 0; i < fileName.size(); ++i) {
        ch = fileName[i];

        if (std::isalnum(static_cast<unsigned char>(ch))) {
            result += ch;
        } else {
            result += '_';
        }
    }

    return result;
}

std::string getMutexName(const std::string& fileName) {
    return makeSafeName(fileName) + "_mutex";
}

std::string getEmptySemaphoreName(const std::string& fileName) {
    return makeSafeName(fileName) + "_empty";
}

std::string getFullSemaphoreName(const std::string& fileName) {
    return makeSafeName(fileName) + "_full";
}

std::string getReadySemaphoreName(const std::string& fileName) {
    return makeSafeName(fileName) + "_ready";
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
    if (handle != NULL) {
        CloseHandle(handle);
        handle = NULL;
    }
}

#endif