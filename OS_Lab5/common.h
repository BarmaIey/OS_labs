#ifndef COMMON_H
#define COMMON_H

const int REQUEST_READ = 1;
const int REQUEST_WRITE = 2;
const int REQUEST_EXIT = 3;
const int REQUEST_RELEASE_READ = 4;
const int REQUEST_RELEASE_WRITE = 5;

const int RESPONSE_OK = 1;
const int RESPONSE_ERROR = 2;


struct employee {
    int num;
    char name[10];
    double hours;
};

struct PipeMessage {
    int command;
    int status;
    int id;
    employee data;
    char text[128];
};

#endif