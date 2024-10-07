#include "common.h"
#include "../utilities/libbcrypt-master/bcrypt.h"

typedef struct s_user {
    char uname[128];
    char pwhash[BCRYPT_HASHSIZE];
    user_role role;
    long creation_time;
} User;