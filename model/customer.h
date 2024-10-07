#include "common.h"

typedef enum e_acc_state {
    INACTIVE,
    ACTIVE
} acc_state;

typedef struct s_customer {
    long uid;
    long cuid;
    acc_state state;
    pinfo pers_info;
    long acc_number;
    long acc_balance;
    long acc_created_at;
    long update_time;
} customer;