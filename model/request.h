#ifndef REQUEST_MODEL
#define REQUEST_MODEL

#include <stdlib.h>
#include <string.h>
#include "user.h"

typedef enum e_req_type {
    REQLOGIN,
    REQCHPW,
    REQREGISTER,
    REQUPDATEUSER,
    REQDELETEUSER,
    REQLOGOUT,
    REQNEWCUST,
    REQNEWEMP
} req_type;

typedef struct s_req_login_data {
    char uname[128];
    char pw[128];
} req_login_data;

typedef struct s_req_chpw_data {
    char oldpw[128];
    char newpw[128];
} req_chpw_data;

typedef struct s_req_uregupdt_data {
    char uname[128];
    char pw[128];
    user_role role;
    PersonalInfo info;
} req_uregupdt_data;

typedef union u_req_data
{
    req_login_data login;
    req_chpw_data chpw;
    req_uregupdt_data uregupdt;
    char udelete[128]; // only uname required
} req_data;

typedef struct s_request {
    req_type type;
    req_data data;
} Request;

#endif // REQUEST_MODEL