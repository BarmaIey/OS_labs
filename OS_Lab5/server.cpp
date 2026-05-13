#include "common.h"

#include <fstream>
#include <vector>

bool fileExists(const std::string& fileName) {
    std::ifstream file;

    file.open(fileName.c_str(), std::ios::binary);
    return file.good();
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

    return 0;
}