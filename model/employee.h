#ifndef EMPLOYEE_MODEL
#define EMPLOYEE_MODEL

#include "common.h"

typedef struct s_employee {
    char uname[UN_LEN];
    PersonalInfo pers_info;
} Employee;

#endif // EMPLOYEE_MODEL