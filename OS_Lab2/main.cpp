#include "LabContext.h"
#include "ThreadProcedures.h"
#include "WinApiError.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {
    static const int kMinArraySize = 1;
    static const int kMaxArraySize = 1000000;

     int ReadInteger(const std::string& prompt)
    {
        for (;;)
        {
            std::cout << prompt;

            int value = 0;
            if (std::cin >> value)
            {
                return value;
            }

            if (std::cin.eof())
            {
                throw std::runtime_error("Input stream ended unexpectedly");
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter an integer value." << std::endl;
        }
    }

    int ReadBoundedInteger(const std::string& prompt, int minValue, int maxValue)
    {
        for (;;)
        {
            int value = ReadInteger(prompt);
            if (value >= minValue && value <= maxValue)
            {
                return value;
            }

            std::cout << "The value must be between " << minValue
                      << " and " << maxValue << "." << std::endl;
        }
    }

    void ReadArray(std::vector<int>& values)
    {
        for (size_t index = 0; index < values.size(); ++index)
        {
            std::ostringstream prompt;
            prompt << "Enter element [" << index << "]: ";
            values[index] = ReadInteger(prompt.str());
        }
    }

    std::string FormatArray(const std::vector<int>& values)
    {
        std::ostringstream out;
        out << "[";
        for (size_t index = 0; index < values.size(); ++index)
        {
            if (index != 0)
            {
                out << ", ";
            }
            out << values[index];
        }
        out << "]";
        return out.str();
    }

    void PrintArray(const std::string& title, const std::vector<int>& values)
    {
        std::cout << title << FormatArray(values) << std::endl;
    }

    int ToReplacementValue(double averageValue)
    {
        return static_cast<int>(averageValue);
    }

    void ReplaceMinAndMaxWithAverage(std::vector<int>& values,
                                     size_t minIndex,
                                     size_t maxIndex,
                                     int replacementValue)
    {
        if (minIndex < values.size())
        {
            values[minIndex] = replacementValue;
        }

        if (maxIndex < values.size() && maxIndex != minIndex)
        {
            values[maxIndex] = replacementValue;
        }
    }

    void CloseHandleIfNeeded(HANDLE& handle)
    {
        if (handle != NULL)
        {
            CloseHandle(handle);
            handle = NULL;
        }
    }

    DWORD GetThreadExitCodeOrThrow(HANDLE handle, const std::string& messagePrefix)
    {
        DWORD exitCode = 0;
        if (!GetExitCodeThread(handle, &exitCode))
        {
            ThrowWindowsError(messagePrefix);
        }
        return exitCode;
    }
}

int main()
{
    HANDLE minMaxThread = NULL;
    HANDLE averageThread = NULL;

    try
    {
        std::cout << "Win32 multithreading laboratory work" << std::endl;

        LabContext context;

        int arraySize = ReadBoundedInteger("Enter array size: ", kMinArraySize, kMaxArraySize);
        context.values.resize(static_cast<size_t>(arraySize));
        ReadArray(context.values);

        PrintArray("Initial array: ", context.values);

        minMaxThread = CreateThread(NULL,
                                    0,
                                    &MinMaxThreadProc,
                                    &context,
                                    0,
                                    NULL);
        CheckHandle(minMaxThread, "Failed to create min_max thread");

        averageThread = CreateThread(NULL,
                                    0,
                                    &AverageThreadProc,
                                    &context,
                                    0,
                                    NULL);
        CheckHandle(averageThread, "Failed to create average thread");

        CheckWaitResult(WaitForSingleObject(minMaxThread, INFINITE),
                        "Failed while waiting for min_max thread");
        CheckWaitResult(WaitForSingleObject(averageThread, INFINITE),
                        "Failed while waiting for average thread");

        if (GetThreadExitCodeOrThrow(minMaxThread, "Failed to query min_max thread exit code") != 0)
        {
            throw std::runtime_error("min_max thread reported an error");
        }

        if (GetThreadExitCodeOrThrow(averageThread, "Failed to query average thread exit code") != 0)
        {
            throw std::runtime_error("average thread reported an error");
        }

        int replacementValue = ToReplacementValue(context.averageValue);
        ReplaceMinAndMaxWithAverage(context.values,
                                    context.minIndex,
                                    context.maxIndex,
                                    replacementValue);

        std::cout << "Average used for replacement: " << replacementValue << std::endl;
        PrintArray("Result array: ", context.values);

        CloseHandleIfNeeded(minMaxThread);
        CloseHandleIfNeeded(averageThread);

        std::cout << "Program finished successfully." << std::endl;
    }
    catch (const std::exception& ex)
    {
        CloseHandleIfNeeded(minMaxThread);
        CloseHandleIfNeeded(averageThread);
        std::cerr << "Error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        CloseHandleIfNeeded(minMaxThread);
        CloseHandleIfNeeded(averageThread);
        std::cerr << "Error: unknown exception" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}