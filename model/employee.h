#ifndef EMPLOYEE_MODEL
#define EMPLOYEE_MODEL

#include "common.h"

typedef struct s_employee {
    char uname[128];
    PersonalInfo pers_info;
} Employee;

#endif // EMPLOYEE_MODEL