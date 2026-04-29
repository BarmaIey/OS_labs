#ifndef LAB_CONTEXT_H
#define LAB_CONTEXT_H

#include <cstddef>
#include <vector>

struct LabContext
{
    std::vector<int> values;
    int minValue;
    int maxValue;
    size_t minIndex;
    size_t maxIndex;
    double averageValue;

    LabContext()
        : minValue(0),
          maxValue(0),
          minIndex(0),
          maxIndex(0),
          averageValue(0.0)
    {
    }

private:
    LabContext(const LabContext&);
    LabContext& operator=(const LabContext&);
};

#endif