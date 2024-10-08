#ifndef USER_MODEL
#define USER_MODEL

#include "common.h"
#include <bcrypt.h>

typedef struct s_user {
    char uname[128];
    char pwhash[BCRYPT_HASHSIZE];
    user_role role;
    long creation_time;
} User;

#endif // USER_MODEL