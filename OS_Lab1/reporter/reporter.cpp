#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include "../common/employee.h"

double calculateSalary(double hours, double rate) {
    return hours * rate;
}

void generateReport(const char* inputFile, const char* outputFile, double rate) {
    std::ifstream in(inputFile, std::ios::binary);

    if (!in) {
        throw std::runtime_error("Cannot open input file");
    }

    std::ofstream out(outputFile);

    if (!out) {
        throw std::runtime_error("Cannot open output file");
    }

    out << "Report for file: " << inputFile << "\n";
    out << "ID\tName\tHours\tSalary\n";
    out << "---------------------------------\n";

    Employee emp;

    while (in.read(reinterpret_cast<char*>(&emp), sizeof(Employee))) {
        double salary = calculateSalary(emp.hours, rate);

        out << emp.num << "\t" << emp.name << "\t"
            << emp.hours << "\t" << salary << "\n";
    }

    if (!in.eof()) {
        throw std::runtime_error("Read error");
    }

    in.close();
    out.close();
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            throw std::runtime_error("Usage: reporter <input> <output> <rate>");
        }

        const char* inputFile = argv[1];
        const char* outputFile = argv[2];
        double rate = atof(argv[3]);

        if (rate <= 0) {
            throw std::runtime_error("Invalid rate");
        }

        generateReport(inputFile, outputFile, rate);

        std::cout << "Report generated\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}