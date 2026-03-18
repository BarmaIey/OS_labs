#ifndef EMPLOYEE_H
#define EMPLOYEE_H

struct Employee {
    int num;
    char name[10];
    double hours;

    Employee() : num(0), hours(0.0) {
        name[0] = '\0';
    }
};

#endif