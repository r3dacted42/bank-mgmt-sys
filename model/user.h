#ifndef USER_MODEL
#define USER_MODEL

#include "common.h"
#include <bcrypt.h>
#define UN_LEN 32
#define PW_LEN 32

typedef struct s_user {
    char uname[UN_LEN];
    char pwhash[BCRYPT_HASHSIZE];
    user_role role;
    long creation_time;
} User;

#endif // USER_MODEL