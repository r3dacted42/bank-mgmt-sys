#ifndef EMPLOYEE_CONTROLLER
#define EMPLOYEE_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "../model/employee.h"

#define EMP_DB_PATH "database/employees.db"

bool emp_read(const char *uname, Employee *emdata) {
    int fd = open(EMP_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    bool found = false;
    while (read(fd, emdata, sizeof(Employee)) > 0) {
        if (strcmp(emdata->uname, uname) == 0) {
            found = true;
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    if (!found) emdata = NULL;
    return found;
}

bool emp_create(const Employee *emdata) {
    int fd = open(EMP_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Employee temp;
    while (read(fd, &temp, sizeof(Employee)) > 0) {
        if (strcmp(temp.uname, emdata->uname) == 0) {
            lck.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lck);
            close(fd);
            return false;
        }
    }
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, 0, SEEK_END);
    write(fd, emdata, sizeof(Employee));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

bool emp_update(const char *uname, const Employee *emdata) {
    int fd = open(EMP_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Employee temp;
    while (read(fd, &temp, sizeof(Employee)) > 0) {
        if (strcmp(temp.uname, uname) == 0) {
            lck.l_type = F_WRLCK;
            lck.l_whence = SEEK_CUR,
            lck.l_start = lseek(fd, -1 * sizeof(Employee), SEEK_CUR),
            lck.l_len = sizeof(Employee);
            fcntl(fd, F_SETLKW, &lck);
            lseek(fd, 0, SEEK_END);
            write(fd, emdata, sizeof(Employee));
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

bool emp_delete(const char *uname) {
    if (uname == NULL) return false;
    int fd = open(EMP_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Employee emdata;
    bool found = false;
    while (read(fd, &emdata, sizeof(Employee)) > 0) {
        if (strcmp(emdata.uname, uname) == 0) {
            found = true;
            lck.l_type = F_WRLCK;
            fcntl(fd, F_SETLKW, &lck);
            int fd2 = dup(fd);
            int new_size = lseek(fd2, -1 * sizeof(Employee), SEEK_END);
            Employee last_emdata;
            read(fd2, &last_emdata, sizeof(Employee));
            close(fd2);
            if (strcmp(emdata.uname, last_emdata.uname)) {
                lseek(fd, -1 * sizeof(Employee), SEEK_CUR);
                write(fd, &last_emdata, sizeof(Employee));
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

#endif // EMPLOYEE_CONTROLLER