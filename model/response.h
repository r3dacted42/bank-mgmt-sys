#ifndef RESPONSE_MODEL
#define RESPONSE_MODEL

#include "user.h"

typedef enum e_res_type {
    RESSUCCESS,
    RESBADREQ,
    RESUNAUTH,
    RESSERVERR
} res_type;

typedef struct s_get_usr_data {
    user_role role;
    PersonalInfo info;
    acc_state cust_state;
    long cust_balance;
} get_usr_data;

typedef union u_res_data
{
    user_role login;
    get_usr_data getusr;
    float getbal;
} res_data;

typedef struct s_response {
    res_type type;
    res_data data;
} Response;

#endif // RESPONSE_MODEL