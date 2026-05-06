#include "common.h"

#include <fstream>
#include <iostream>

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


int main() {
    std::string fileName;
    int recordCount;

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

        std::cout << "Binary message file was created.\n";
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";
        return 1;
    }

    return 0;
}