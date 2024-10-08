#ifndef EMPLOYEE_MODEL
#define EMPLOYEE_MODEL

#include "common.h"

typedef struct s_employee {
    long uid;
    long emid;
    PersonalInfo pers_info;
    char job_title[64];
    int hire_date[3]; // YYYY, MM, DD
    long salary;
    long update_time;
} Employee;

#endif // EMPLOYEE_MODEL