#ifndef THREAD_PROCEDURES_H
#define THREAD_PROCEDURES_H

#include <windows.h>

DWORD WINAPI MinMaxThreadProc(LPVOID parameter);
DWORD WINAPI AverageThreadProc(LPVOID parameter);

#endif