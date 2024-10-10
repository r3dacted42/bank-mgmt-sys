#ifndef USER_CONTROLLER
#define USER_CONTROLLER

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "../model/user.h"
#include <bcrypt.h>

#define USER_DB_PATH "database/users.db"

int get_user_count() {
    int fd = open(USER_DB_PATH, O_CREAT | O_RDONLY, 0644);
    if (fd < 0) return 0;
    int sz = lseek(fd, 0, SEEK_END);
    close(fd);
    return (sz / sizeof(User));
}

bool user_read(const char *uname, User *udata) {
    int fd = open(USER_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    bool found = false;
    while (read(fd, udata, sizeof(User)) > 0) {
        if (strcmp(udata->uname, uname) == 0) {
            found = true;
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    if (!found) udata = NULL;
    return found;
}

typedef enum e_login_res {
    LGNSUCCESS,
    LGNWRONGPW,
    LGNUNAMENOTFOUND,
    LGNNULLPTRERR
} login_res;

/// @brief user login
/// @param uname username
/// @param passwd password 
/// @param udata location to store user data, must not be NULL
/// @return 0 for success, or corresponding error code
login_res user_login(const char *uname, const char *passwd, User *udata) {
    if (udata == NULL || uname == NULL || passwd == NULL) return LGNNULLPTRERR;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    login_res result = LGNNULLPTRERR;
    while (read(fd, udata, sizeof(User)) > 0) {
        if (strcmp(udata->uname, uname) == 0) {
            if (bcrypt_checkpw(passwd, udata->pwhash) == 0)
                result = LGNSUCCESS;
            else 
                result = LGNWRONGPW;
            break;
        }
    }
    if (result == LGNNULLPTRERR) {
        udata = NULL;
        result = LGNUNAMENOTFOUND;
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return result;
}

typedef enum e_register_res {
    REGSUCCESS,
    UNAMETAKEN,
    REGNULLPTRERR
} register_res;

/// @brief add new user to system
/// @param uname username of new user, must be unique
/// @param passwd password of new user
/// @param role role of new user
/// @return 0 for success, or correponding error code
register_res user_register(const char *uname, const char *passwd, user_role role) {
    if (uname == NULL || passwd == NULL) return REGNULLPTRERR;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    User udata;
    while (read(fd, &udata, sizeof(User)) > 0) {
        if (strcmp(udata.uname, uname) == 0) {
            lck.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lck);
            close(fd);
            return UNAMETAKEN;
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
    return REGSUCCESS;
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
bool user_update(int opt, const char *uname, const char *nuname, const char *passwd, user_role role) {
    if (uname == NULL) return false;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    User udata;
    bool found = false;
    while (read(fd, &udata, sizeof(User)) > 0) {
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
bool user_delete(const char *uname) {
    if (uname == NULL) return false;
    int fd = open(USER_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    User udata;
    bool found = false;
    // printf("trying to delete %s\n", uname);
    int uidx = 0;
    while (read(fd, &udata, sizeof(User)) > 0) {
        if (strcmp(udata.uname, uname) == 0) {
            found = true;
            lck.l_type = F_WRLCK;
            fcntl(fd, F_SETLKW, &lck);
            int fd2 = dup(fd);
            int new_size = lseek(fd2, -1 * sizeof(User), SEEK_END);
            // printf("new size stores %ld users\n", new_size / sizeof(User));
            User last_udata;
            read(fd2, &last_udata, sizeof(User));
            // printf("read last_udata: %s\n", last_udata.uname);
            close(fd2);
            if (strcmp(udata.uname, last_udata.uname)) {
                lseek(fd, uidx * sizeof(User), SEEK_SET);
                // printf("fd will overwrite user %ld\n", lseek(fd, 0, SEEK_CUR) / sizeof(User));
                write(fd, &last_udata, sizeof(User));
            }
            ftruncate(fd, new_size);
            break;
        }
        uidx++;
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return found;
}

#endif // USER_CONTROLLER