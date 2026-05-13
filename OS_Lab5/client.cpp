#include "common.h"

int readCommand() {
    int command;

    command = 0;

    std::cout << "\n1 - modify employee record\n";
    std::cout << "2 - read employee record\n";
    std::cout << "3 - exit\n";
    std::cout << "Enter command: ";
    std::cin >> command;

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return 0;
    }

    std::cin.ignore(10000, '\n');

    return command;
}

void sendReadRequest(HANDLE pipe) {
    PipeMessage request;
    PipeMessage response;
    int id;

    id = readPositiveInt("Enter employee ID: ");

    clearMessage(request);
    request.command = REQUEST_READ;
    request.id = id;

    if (!writePipeMessage(pipe, request)) {
        throw std::runtime_error(getLastErrorMessage("Failed to send read request."));
    }

    if (!readPipeMessage(pipe, response)) {
        throw std::runtime_error(getLastErrorMessage("Failed to receive read response."));
    }

    if (response.status == RESPONSE_ERROR) {
        std::cout << "Server error: " << response.text << "\n";
        return;
    }

    std::cout << "Received employee record:\n";
    printEmployee(response.data);

    std::cout << "Press Enter to release read access...";
    std::cin.get();

    clearMessage(request);
    request.command = REQUEST_RELEASE_READ;

    if (!writePipeMessage(pipe, request)) {
        throw std::runtime_error(getLastErrorMessage("Failed to release read access."));
    }

    readPipeMessage(pipe, response);
}

void sendWriteRequest(HANDLE pipe) {
    PipeMessage request;
    PipeMessage response;
    employee updatedEmployee;
    std::string newName;

    int id;

    id = readPositiveInt("Enter employee ID: ");

    clearMessage(request);
    request.command = REQUEST_WRITE;
    request.id = id;

    if (!writePipeMessage(pipe, request)) {
        throw std::runtime_error(getLastErrorMessage("Failed to send write request."));
    }

    if (!readPipeMessage(pipe, response)) {
        throw std::runtime_error(getLastErrorMessage("Failed to receive write response."));
    }

    if (response.status == RESPONSE_ERROR) {
        std::cout << "Server error: " << response.text << "\n";
        return;
    }

    updatedEmployee = response.data;

    std::cout << "Current employee record:\n";
    printEmployee(updatedEmployee);

    std::cout << "Enter new name, max 9 characters: ";
    std::getline(std::cin, newName);

    while (newName.empty() || newName.size() >= NAME_SIZE) {
        std::cout << "Invalid name. Enter new name, max 9 characters: ";
        std::getline(std::cin, newName);
    }

    copyStringToCharArray(updatedEmployee.name, NAME_SIZE, newName);

    std::cout << "Enter new hours: ";
    std::cin >> updatedEmployee.hours;

    while (std::cin.fail() || updatedEmployee.hours < 0.0) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid hours. Enter non-negative number: ";
        std::cin >> updatedEmployee.hours;
    }

    std::cin.ignore(10000, '\n');

    clearMessage(request);
    request.command = REQUEST_RELEASE_WRITE;
    request.data = updatedEmployee;

    std::cout << "Press Enter to send modified record and release write access...";
    std::cin.get();

    if (!writePipeMessage(pipe, request)) {
        throw std::runtime_error(getLastErrorMessage("Failed to send modified record."));
    }

    if (!readPipeMessage(pipe, response)) {
        throw std::runtime_error(getLastErrorMessage("Failed to receive release response."));
    }

    if (response.status == RESPONSE_OK) {
        std::cout << response.text << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::string pipeName;
    HANDLE pipe;
    int command;
    PipeMessage request;

    pipe = INVALID_HANDLE_VALUE;

    try {
        if (argc != 2) {
            throw std::runtime_error("Usage: Client.exe <pipe_name>");
        }

        pipeName = argv[1];

        while (true) {
            pipe = CreateFileA(
                pipeName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );

            if (pipe != INVALID_HANDLE_VALUE) {
                break;
            }

            if (GetLastError() != ERROR_PIPE_BUSY) {
                throw std::runtime_error(getLastErrorMessage("Failed to open named pipe."));
            }

            if (!WaitNamedPipeA(pipeName.c_str(), 5000)) {
                throw std::runtime_error(getLastErrorMessage("Named pipe is busy."));
            }
        }

        std::cout << "Client connected to server.\n";

        while (true) {
            command = readCommand();

            if (command == 1) {
                sendWriteRequest(pipe);
            } else if (command == 2) {
                sendReadRequest(pipe);
            } else if (command == 3) {
                clearMessage(request);
                request.command = REQUEST_EXIT;
                writePipeMessage(pipe, request);
                break;
            } else {
                std::cout << "Unknown command.\n";
            }
        }

        closeHandle(pipe);
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << "\n";
        closeHandle(pipe);
        return 1;
    }

    return 0;
}