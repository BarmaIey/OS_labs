#include "ThreadProcedures.h"

#include "LabContext.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstddef>

namespace
{
    static const DWORD kMinMaxSleepMs = 7;
    static const DWORD kAverageSleepMs = 12;
}

DWORD WINAPI MinMaxThreadProc(LPVOID parameter)
{
    LabContext* context = static_cast<LabContext*>(parameter);
    if (context == NULL)
    {
        return 1;
    }

    if (context->values.empty())
    {
        return 2;
    }

    int minValue = context->values[0];
    int maxValue = context->values[0];
    size_t minIndex = 0;
    size_t maxIndex = 0;

    for (size_t index = 1; index < context->values.size(); ++index)
    {
        int currentValue = context->values[index];

        if (currentValue < minValue)
        {
            minValue = currentValue;
            minIndex = index;
        }

        if (currentValue > maxValue)
        {
            maxValue = currentValue;
            maxIndex = index;
        }

        Sleep(kMinMaxSleepMs);
    }

    context->minValue = minValue;
    context->maxValue = maxValue;
    context->minIndex = minIndex;
    context->maxIndex = maxIndex;

    std::cout << "[min_max thread] Minimum = " << minValue
              << ", maximum = " << maxValue << std::endl;

    return 0;
}

DWORD WINAPI AverageThreadProc(LPVOID parameter)
{
    LabContext* context = static_cast<LabContext*>(parameter);
    if (context == NULL)
    {
        return 1;
    }

    if (context->values.empty())
    {
        return 2;
    }

    double sum = 0.0;
    for (size_t index = 0; index < context->values.size(); ++index)
    {
        sum += static_cast<double>(context->values[index]);
        Sleep(kAverageSleepMs);
    }

    context->averageValue = sum / static_cast<double>(context->values.size());

    std::cout << "[average thread] Arithmetic mean = "
              << context->averageValue << std::endl;

    return 0;
}

