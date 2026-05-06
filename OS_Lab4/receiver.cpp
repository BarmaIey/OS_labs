#include "common.h"

#include <iostream>

int main() {
    int recordCount;

    try {
        recordCount = readPositiveInt("Enter record count: ");

        std::cout << "Record count: " << recordCount << "\n";
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}