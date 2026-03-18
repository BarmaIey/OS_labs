#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <cstdlib> 
#include "../common/employee.h"

const int MAX_NAME_LENGTH = 10;

Employee readEmployee() {
    Employee emp;

    std::string tempName;

    std::cout << "Enter ID: ";
    std::cin >> emp.num;

    if (std::cin.fail()) {
        throw std::runtime_error("Invalid ID input");
    }

    std::cout << "Enter name: ";
    std::cin >> tempName;

    if (tempName.length() >= MAX_NAME_LENGTH) {
        throw std::runtime_error("Name too long (max 9 characters)");
    }

    std::strcpy(emp.name, tempName.c_str());

    std::cout << "Enter hours: ";
    std::cin >> emp.hours;

    if (std::cin.fail()) {
        throw std::runtime_error("Invalid hours input");
    }

    return emp;
}

void writeToFile(const char* filename, int count) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Cannot open file");
    }

    int i; 
    for (i = 0; i < count; ++i) {
        Employee emp = readEmployee();
        file.write(reinterpret_cast<char*>(&emp), sizeof(Employee));

        if (!file) {
            throw std::runtime_error("Write error (disk full?)");
        }
    }

    file.close();
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 3) {
            throw std::runtime_error("Usage: creator <file> <count>");
        }

        const char* filename = argv[1];
        int count = atoi(argv[2]);

        if (count <= 0) {
            throw std::runtime_error("Invalid record count");
        }

        writeToFile(filename, count);

        std::cout << "File created successfully\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}