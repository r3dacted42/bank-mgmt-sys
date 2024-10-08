#ifndef RESPONSE_MODEL
#define RESPONSE_MODEL

#include <stdlib.h>
#include <string.h>
#include "user.h"

typedef enum e_res_type {
    RESLOGIN,
    RESREGISTER,
    RESUPDATEUSER,
    RESDELETEUSER,
    RESBADREQ,
    RESUNAUTH,
    RESSERVERR
} res_type;

typedef struct s_res_login_data {
    char uname[128];
    user_role role;
} res_login_data;

typedef union u_res_data
{
    res_login_data login;
} res_data;

typedef struct s_response {
    res_type type;
    res_data data;
} Response;

#endif // RESPONSE_MODEL