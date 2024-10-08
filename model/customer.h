#ifndef CUSTOMER_MODEL
#define CUSTOMER_NODEL

#include "common.h"

typedef enum e_acc_state {
    INACTIVE,
    ACTIVE
} acc_state;

typedef struct s_customer {
    long uid;
    long cuid;
    acc_state state;
    PersonalInfo pers_info;
    long acc_number;
    long acc_balance;
    long acc_created_at;
    long update_time;
} Customer;

#endif // CUSTOMER_MODEL