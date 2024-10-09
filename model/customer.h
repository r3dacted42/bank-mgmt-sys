#ifndef CUSTOMER_MODEL
#define CUSTOMER_NODEL

#include "common.h"

typedef enum e_acc_state {
    INACTIVE,
    ACTIVE
} acc_state;

typedef struct s_customer {
    char uname[128];
    acc_state state;
    PersonalInfo pers_info;
    long balance;
} Customer;

#endif // CUSTOMER_MODEL