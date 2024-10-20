#ifndef CUSTOMER_MODEL
#define CUSTOMER_NODEL

#include "common.h"

typedef enum e_acc_state {
    CACC_INACTIVE,
    CACC_ACTIVE
} acc_state;

typedef struct s_customer {
    char uname[UN_LEN];
    acc_state state;
    PersonalInfo pers_info;
    float balance;
} Customer;

#endif // CUSTOMER_MODEL