#ifndef REQUEST_MODEL
#define REQUEST_MODEL

#include <stdlib.h>
#include <string.h>
#include "user.h"

typedef enum e_req_type {
    REQLOGIN,
    REQREGISTER,
    REQUPDATEUSER,
    REQDELETEUSER,
    REQLOGOUT
} req_type;

typedef struct s_req_login_data {
    char uname[128];
    char pw[128];
} req_login_data;

typedef union u_req_data
{
    req_login_data login;
} req_data;

typedef struct s_request {
    req_type type;
    req_data data;
} Request;

#endif // REQUEST_MODEL