#include "WinApiError.h"

#include <sstream>

std::string GetWindowsErrorMessage(DWORD errorCode)
{
    LPVOID buffer = NULL;
    DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS;
    DWORD length = FormatMessageA(flags,
                                  NULL,
                                  errorCode,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  reinterpret_cast<LPSTR>(&buffer),
                                  0,
                                  NULL);

    if (length == 0 || buffer == NULL)
    {
        std::ostringstream fallback;
        fallback << "Windows error code " << errorCode;
        return fallback.str();
    }

    std::string message(static_cast<LPCSTR>(buffer));
    LocalFree(buffer);

    while (!message.empty() &&
           (message[message.size() - 1] == '\r' ||
            message[message.size() - 1] == '\n' ||
            message[message.size() - 1] == ' ' ||
            message[message.size() - 1] == '\t'))
    {
        message.erase(message.size() - 1);
    }

    return message;
}

void ThrowWindowsError(const std::string& messagePrefix)
{
    DWORD errorCode = GetLastError();
    std::ostringstream out;
    out << messagePrefix << ": " << GetWindowsErrorMessage(errorCode)
        << " (error code " << errorCode << ")";
    throw std::runtime_error(out.str());
}

void CheckWindowsCall(BOOL success, const std::string& messagePrefix)
{
    if (!success)
    {
        ThrowWindowsError(messagePrefix);
    }
}

void CheckHandle(HANDLE handle, const std::string& messagePrefix)
{
    if (handle == NULL || handle == INVALID_HANDLE_VALUE)
    {
        ThrowWindowsError(messagePrefix);
    }
}

void CheckWaitResult(DWORD waitResult, const std::string& messagePrefix)
{
    if (waitResult == WAIT_OBJECT_0)
    {
        return;
    }

    if (waitResult == WAIT_TIMEOUT)
    {
        throw std::runtime_error(messagePrefix + ": wait timed out");
    }

    if (waitResult == WAIT_FAILED)
    {
        ThrowWindowsError(messagePrefix);
    }

    std::ostringstream out;
    out << messagePrefix << ": unexpected wait result " << waitResult;
    throw std::runtime_error(out.str());
}
