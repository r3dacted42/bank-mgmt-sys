#ifndef REQUEST_MODEL
#define REQUEST_MODEL

#include "user.h"
#include "../controller/customer.h"
#include "../controller/loan.h"

typedef enum e_req_type {
    REQLOGIN,
    REQGETUSR,
    REQGETUSRROLE,
    REQCHPW,
    REQREGISTER,
    REQUPDTUSR,
    REQDLTUSR,
    REQDEPOSIT,
    REQWITHDRAW,
    REQTRANSFER,
    REQVIEWTRAN,
    REQLOANAPPL,
    // need no data
    REQGETBAL,
    REQLOGOUT
} req_type;

typedef struct s_req_login_data {
    char uname[UN_LEN];
    char pw[PW_LEN];
} req_login_data;

typedef struct s_req_getusr_data {
    char uname[UN_LEN];
} req_getusr_data;

typedef struct s_req_chpw_data {
    char oldpw[PW_LEN];
    char newpw[PW_LEN];
} req_chpw_data;

typedef struct s_req_ureg_data {
    char uname[UN_LEN];
    char pw[PW_LEN];
    user_role role;
    PersonalInfo info;
} req_ureg_data;

typedef struct s_req_uupdt_data {
    char uname[UN_LEN];
    char nuname[UN_LEN];
    char pw[PW_LEN];
    // common to s_get_usr_data
    user_role role;
    PersonalInfo info;
    acc_state cust_state;
    long cust_balance;
} req_uupdt_data;

typedef struct s_transfer_data {
    char oun[UN_LEN];
    float amt;
} transfer_data;

typedef struct s_view_tran_data {
    char un[UN_LEN];
    int page_num;
} view_tran_data;

typedef union u_req_data
{
    req_login_data login; // REQLOGIN
    char getusr[UN_LEN]; // REQGETUSR, REQGETUSRROLE
    req_chpw_data chpw; // REQCHPW
    req_ureg_data ureg; // REQREGISTER
    req_uupdt_data uupdt; // REQUPDTUSR
    char udlt[UN_LEN]; // REQDLTUSR
    float baldelta; // REQDEPOSIT, REQWITHDRAW
    transfer_data transfer; // REQTRANSFER
    view_tran_data viewtran; // REQVIEWTRAN
    Loan loanappl; // REQLOANAPPL
} req_data;

typedef struct s_request {
    req_type type;
    req_data data;
} Request;

#endif // REQUEST_MODEL