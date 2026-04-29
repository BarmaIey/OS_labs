#ifndef WIN_API_ERROR_H
#define WIN_API_ERROR_H

#include <stdexcept>
#include <string>
#include <windows.h>

std::string GetWindowsErrorMessage(DWORD errorCode);
void ThrowWindowsError(const std::string& messagePrefix);
void CheckWindowsCall(BOOL success, const std::string& messagePrefix);
void CheckHandle(HANDLE handle, const std::string& messagePrefix);
void CheckWaitResult(DWORD waitResult, const std::string& messagePrefix);

#endif
