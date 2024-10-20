#ifndef REQUEST_MODEL
#define REQUEST_MODEL

#include "user.h"
#include "../controller/customer.h"
#include "../controller/loan.h"
#include "../controller/feedback.h"

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
    REQLNAPPL,
    REQLNASSGNPOST,
    REQLNRVPOST,
    REQADDFDBK,
    // need no data
    REQGETBAL,
    REQLNASSGNGET,
    REQLNRVGET,
    REQLNCUSTGET,
    REQGETEMPS,
    REQGETFDBK,
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

typedef struct s_ln_assgn_post_data {
    long loan_id;
    char eun[UN_LEN];
} ln_assgn_post_data;

typedef struct s_ln_rv_post_data {
    long loan_id;
    loan_status status;
    float acpt_amt;
    float rate;
    char reason[128];
} ln_rv_post_data;

typedef struct s_add_fdbk_data {
    feedback_category cat;
    char text[FDBK_TEXT_LEN];
} add_fdbk_data;

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
    Loan loanappl; // REQLNAPPL
    ln_assgn_post_data lnassgn; // REQLNASSGNPOST
    ln_rv_post_data lnrv; // REQLNRVPOST
    add_fdbk_data fdbk; // REQADDFDBK
} req_data;

typedef struct s_request {
    req_type type;
    req_data data;
} Request;

#endif // REQUEST_MODEL