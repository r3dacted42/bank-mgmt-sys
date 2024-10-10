#ifndef CUSTOMER_CONTROLLER
#define CUSTOMER_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "../model/customer.h"

#define CUST_DB_PATH "database/customers.db"

bool cust_read(const char *uname, Customer *cdata) {
    int fd = open(CUST_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    bool found = false;
    while (read(fd, cdata, sizeof(Customer)) > 0) {
        if (strcmp(cdata->uname, uname) == 0) {
            found = true;
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    if (!found) cdata = NULL;
    return found;
}

bool cust_create(const Customer *cdata) {
    int fd = open(CUST_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Customer temp;
    while (read(fd, &temp, sizeof(Customer)) > 0) {
        if (strcmp(temp.uname, cdata->uname) == 0) {
            lck.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lck);
            close(fd);
            return false;
        }
    }
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, 0, SEEK_END);
    write(fd, cdata, sizeof(Customer));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

bool cust_update(const char *uname, const Customer *cdata) {
    int fd = open(CUST_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Customer temp;
    while (read(fd, &temp, sizeof(Customer)) > 0) {
        if (strcmp(temp.uname, uname) == 0) {
            lck.l_type = F_WRLCK;
            lck.l_whence = SEEK_CUR,
            lck.l_start = lseek(fd, -1 * sizeof(Customer), SEEK_CUR),
            lck.l_len = sizeof(Customer);
            fcntl(fd, F_SETLKW, &lck);
            write(fd, cdata, sizeof(Customer));
            lck.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lck);
            close(fd);
            return true;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return false;
}

bool cust_delete(const char *uname) {
    if (uname == NULL) return false;
    int fd = open(CUST_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Customer cdata;
    bool found = false;
    int cidx = 0;
    while (read(fd, &cdata, sizeof(Customer)) > 0) {
        if (strcmp(cdata.uname, uname) == 0) {
            found = true;
            lck.l_type = F_WRLCK;
            fcntl(fd, F_SETLKW, &lck);
            int fd2 = dup(fd);
            int new_size = lseek(fd2, -1 * sizeof(Customer), SEEK_END);
            Customer last_cdata;
            read(fd2, &last_cdata, sizeof(Customer));
            close(fd2);
            if (strcmp(cdata.uname, last_cdata.uname)) {
                lseek(fd, cidx * sizeof(Customer), SEEK_SET);
                write(fd, &last_cdata, sizeof(Customer));
            }
            ftruncate(fd, new_size);
            break;
        }
        cidx++;
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return found;
}

#endif // CUSTOMER_CONTROLLER