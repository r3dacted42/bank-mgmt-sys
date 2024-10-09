#ifndef RESPONSE_MODEL
#define RESPONSE_MODEL

#include <stdlib.h>
#include <string.h>
#include "user.h"

typedef enum e_res_type {
    RESSUCCESS,
    RESBADREQ,
    RESUNAUTH,
    RESSERVERR
} res_type;

typedef union u_res_data
{
    user_role login;
} res_data;

typedef struct s_response {
    res_type type;
    res_data data;
} Response;

#endif // RESPONSE_MODEL