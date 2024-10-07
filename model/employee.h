#include "common.h"

typedef struct s_employee {
    long uid;
    long emid;
    pinfo pers_info;
    char job_title[64];
    int hire_date[3]; // YYYY, MM, DD
    long salary;
    long update_time;
} employee;