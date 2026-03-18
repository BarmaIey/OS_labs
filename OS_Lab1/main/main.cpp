#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "../common/employee.h"

void runProcess(const std::string& commandLine) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char* cmd = _strdup(commandLine.c_str());

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        free(cmd);
        throw std::runtime_error("CreateProcess failed");
    }

    free(cmd);

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

template <typename T>
std::string toString(const T& value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

void printBinaryFile(const std::string& filename) {
    std::ifstream file(filename.c_str(), std::ios::binary);

    if (!file) {
        throw std::runtime_error("Cannot open binary file");
    }

    Employee emp;

    std::cout << "\n--- Binary file content ---\n";

    while (file.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        std::cout << "ID: " << emp.num
                  << ", Name: " << emp.name
                  << ", Hours: " << emp.hours
                  << std::endl;
    }

    if (!file.eof()) {
        throw std::runtime_error("Error reading binary file");
    }

    file.close();
}

void printTextFile(const std::string& filename) {
    std::ifstream file(filename.c_str());

    if (!file) {
        throw std::runtime_error("Cannot open report file");
    }

    std::cout << "\n--- Report content ---\n";

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << std::endl;
    }

    file.close();
}

int main() {
    try {
        std::string binFile;
        int count;

        std::cout << "Enter binary file name: ";
        std::cin >> binFile;

        std::cout << "Enter number of records: ";
        std::cin >> count;

        if (count <= 0) {
            throw std::runtime_error("Invalid count");
        }

        // --- Creator ---
        std::string creatorCmd =
            "creator.exe " + binFile + " " + toString(count);

        runProcess(creatorCmd);
        printBinaryFile(binFile);

        std::string reportFile;
        double rate;

        std::cout << "\nEnter report file name: ";
        std::cin >> reportFile;

        std::cout << "Enter hourly rate: ";
        std::cin >> rate;

        if (rate <= 0) {
            throw std::runtime_error("Invalid rate");
        }

        // --- Reporter ---
        std::string reporterCmd =
            "reporter.exe " + binFile + " " + reportFile + " " + toString(rate);

        runProcess(reporterCmd);
        printTextFile(reportFile);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}