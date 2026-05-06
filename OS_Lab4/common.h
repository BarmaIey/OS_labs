#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

#include <string>

const int MESSAGE_SIZE = 20;

struct FileHeader {
    int maxRecords;
    int readIndex;
    int writeIndex;
};

struct MessageRecord {
    char text[MESSAGE_SIZE];
};

#endif