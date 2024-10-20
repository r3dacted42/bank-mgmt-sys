#ifndef RESPONSE_MODEL
#define RESPONSE_MODEL

#include "user.h"
#include "transaction.h"

typedef enum e_res_type {
    RESSUCCESS,
    RESEMPTY,
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

typedef struct s_tran_list {
    int page_len;
    int total_pages;
    tran_list_item lst[5];
} tran_list;

typedef union u_res_data
{
    user_role login;
    get_usr_data getusr;
    user_role getusrrole;
    float getbal;
    tran_list viewtran;
    int bufcount; // number of objects to read
    char msg[64]; // error message
} res_data;

typedef struct s_response {
    res_type type;
    res_data data;
} Response;

#endif // RESPONSE_MODEL