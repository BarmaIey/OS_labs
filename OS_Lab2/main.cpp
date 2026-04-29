#include "LabContext.h"
#include "ThreadProcedures.h"

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
}

int main()
{
    HANDLE minMaxThread = NULL;
    HANDLE averageThread = NULL;

    std::cout << "Win32 multithreading laboratory work" << std::endl;

    LabContext context;

    int arraySize = ReadBoundedInteger("Enter array size: ", kMinArraySize, kMaxArraySize);
    context.values.resize(static_cast<size_t>(arraySize));
    ReadArray(context.values);

    minMaxThread = CreateThread(NULL,
                                    0,
                                    &MinMaxThreadProc,
                                    &context,
                                    0,
                                    NULL);

    averageThread = CreateThread(NULL,
                                    0,
                                    &AverageThreadProc,
                                    &context,
                                    0,
                                    NULL);

    WaitForSingleObject(minMaxThread, INFINITE);
    WaitForSingleObject(averageThread, INFINITE);

    return EXIT_SUCCESS;
}