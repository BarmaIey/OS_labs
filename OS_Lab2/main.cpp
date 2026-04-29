#include <iostream>
#include <limits>

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
}

int main()
{
    std::cout << "Win32 multithreading laboratory work" << std::endl;

    int arraySize = ReadBoundedInteger("Enter array size: ", kMinArraySize, kMaxArraySize);
    
    return EXIT_SUCCESS;
}