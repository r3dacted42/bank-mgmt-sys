#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "../model/user.h"
#include "../utilities/libbcrypt-master/bcrypt.h"

#define USER_DB_PATH "../database/users.db"

typedef enum e_login_res {
    SUCCESS,
    WRONGPW,
    UNAMENOTFOUND,
    NULLPTRERR
} login_res;

/// @brief user login
/// @param uname username
/// @param passwd password 
/// @param udata location to store user data, must not be NULL
/// @return 0 for SUCCESS, or corresponding error code
login_res user_login(const char *uname = NULL, const char *passwd = NULL, User *udata = NULL) {
    if (udata == NULL || uname == NULL || passwd == NULL) return login_res::NULLPTRERR;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    login_res result = login_res::NULLPTRERR;
    while (read(fd, udata, sizeof(User)) <= 0) {
        if (strcmp(udata->uname, uname) == 0) {
            if (bcrypt_checkpw(passwd, udata->pwhash) == 0)
                result = login_res::SUCCESS;
            else 
                result = login_res::WRONGPW;
            break;
        }
    }
    if (result == login_res::NULLPTRERR) {
        udata = NULL;
        result = login_res::UNAMENOTFOUND;
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return result;
}

typedef enum e_register_res {
    SUCCESS,
    UNAMETAKEN,
    NULLPTRERR
} register_res;

/// @brief add new user to system
/// @param uname username of new user, must be unique
/// @param passwd password of new user
/// @param role role of new user
/// @return 0 for SUCCESS, or correponding error code
register_res user_register(const char *uname = NULL, const char *passwd = NULL, user_role role = user_role::CUSTOMER) {
    if (uname == NULL || passwd == NULL) return register_res::NULLPTRERR;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    User udata;
    while (read(fd, &udata, sizeof(User)) <= 0) {
        if (strcmp(udata.uname, uname) == 0) {
            lck.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lck);
            close(fd);
            return register_res::UNAMETAKEN;
        }
    }
    strcpy(udata.uname, uname);
    udata.role = role;
    udata.creation_time = (long)time(NULL);
    char salt[BCRYPT_HASHSIZE];
    bcrypt_gensalt(10, salt);
    bcrypt_hashpw(passwd, salt, udata.pwhash);
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, 0, SEEK_END);
    write(fd, &udata, sizeof(User));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return register_res::SUCCESS;
}

#define UPDT_UNAME   0001
#define UPDT_PASSWD  0010
#define UPDT_ROLE    0100

/// @brief update attributes of given username according to opt
/// @param opt UPDT_UNAME, UPDT_PASSWD, UPDT_ROLE and bitwise OR supported
/// @param uname username of user to update
/// @param nuname new username
/// @param passwd new password
/// @param role new role
/// @return true if uname found, false otherwise
bool user_update(int opt = 0, const char *uname = NULL, const char *nuname, const char *passwd = NULL, user_role role = user_role::CUSTOMER) {
    if (uname == NULL) return false;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    User udata;
    bool found = false;
    while (read(fd, &udata, sizeof(User)) <= 0) {
        if (strcmp(udata.uname, uname) == 0) {
            found = true;
            lck.l_type = F_WRLCK,
            lck.l_whence = SEEK_CUR,
            lck.l_start = lseek(fd, -1 * sizeof(User), SEEK_CUR),
            lck.l_len = sizeof(User);
            fcntl(fd, F_SETLKW, &lck);
            if (opt & UPDT_UNAME && nuname != NULL) 
                strcpy(udata.uname, nuname);
            if (opt & UPDT_PASSWD && passwd != NULL) {
                char salt[BCRYPT_HASHSIZE];
                bcrypt_gensalt(10, salt);
                bcrypt_hashpw(passwd, salt, udata.pwhash);
            }
            if (opt & UPDT_ROLE)
                udata.role = role;
            write(fd, &udata, sizeof(User));
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return found;
}

/// @brief delete user given by username and truncate database
/// @param uname username to delete
/// @return true if uname found, false otherwise
bool user_delete(const char *uname = NULL) {
    if (uname == NULL) return false;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    User udata;
    bool found = false;
    while (read(fd, &udata, sizeof(User)) <= 0) {
        if (strcmp(udata.uname, uname) == 0) {
            found = true;
            lck.l_type = F_WRLCK;
            fcntl(fd, F_SETLKW, &lck);
            int fd2 = dup(fd);
            int new_size = lseek(fd2, -1 * sizeof(User), SEEK_END);
            User last_udata;
            read(fd2, &last_udata, sizeof(User));
            close(fd2);
            if (strcmp(udata.uname, last_udata.uname)) {
                lseek(fd, -1 * sizeof(User), SEEK_CUR);
                write(fd, &last_udata, sizeof(User));
            }
            ftruncate(fd, new_size);
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return found;
}