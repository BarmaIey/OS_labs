#include "common.h"

int main(int argc, char* argv[]) {
    std::string pipeName;

    HANDLE pipe;

    pipe = INVALID_HANDLE_VALUE;

    try {
        if (argc != 2) {
            throw std::runtime_error(
                "Usage: Client.exe <pipe_name>"
            );
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
                throw std::runtime_error(
                    getLastErrorMessage(
                        "Failed to open named pipe."
                    )
                );
            }

            if (!WaitNamedPipeA(
                    pipeName.c_str(),
                    5000)) {

                throw std::runtime_error(
                    getLastErrorMessage(
                        "Named pipe is busy."
                    )
                );
            }
        }

        std::cout << "Client connected to server.\n";

        closeHandle(pipe);
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: "
                  << exception.what()
                  << "\n";

        closeHandle(pipe);

        return 1;
    }

    return 0;
}