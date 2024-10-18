#ifndef EMPLOYEE_CONTROLLER
#define EMPLOYEE_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "../model/employee.h"
#include "../controller/user.h"

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

int emp_read_all(Employee ***emdata) {
    int fd = open(EMP_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int count = lseek(fd, 0, SEEK_END) / sizeof(Employee);
    *emdata = (Employee**)calloc(count + 1, sizeof(Employee*));
    (*emdata)[count] = NULL; // trailing NULL
    lseek(fd, 0, SEEK_SET);
    for (int i = 0; i < count; i++) {
        (*emdata)[i] = (Employee*)malloc(sizeof(Employee));
        int rd = read(fd, (*emdata)[i], sizeof(Employee));
        if (rd != sizeof(Employee)) break;
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return count;
}

int emp_read_all_no_man(Employee ***emdata) {
    int count = 0;
    int ufd = open(USER_DB_PATH, O_RDONLY);
    struct flock ulck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(ufd, F_SETLKW, &ulck);
    User temp;
    while (read(ufd, &temp, sizeof(User)) > 0) if (temp.role == EMPLOYEE) count++;
    *emdata = (Employee**)calloc(count + 1, sizeof(Employee*));
    (*emdata)[count] = NULL; // trailing NULL
    lseek(ufd, 0, SEEK_SET);
    for (int i = 0; i < count; ) {
        read(ufd, &temp, sizeof(User));
        if (temp.role == EMPLOYEE) {
            (*emdata)[i] = (Employee*)malloc(sizeof(Employee));
            emp_read(temp.uname, (*emdata)[i]);
            i++;
        }
    }
    ulck.l_type = F_UNLCK;
    fcntl(ufd, F_SETLK, &ulck);
    return count;
}

void emp_free(Employee ***emdata) {
    for (int i = 0; (*emdata)[i] != NULL; i++) free((*emdata)[i]);
    free(*emdata);
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
    int eidx = 0;
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
                lseek(fd, eidx * sizeof(Employee), SEEK_SET);
                write(fd, &last_emdata, sizeof(Employee));
            }
            ftruncate(fd, new_size);
            break;
        }
        eidx++;
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return found;
}

#endif // EMPLOYEE_CONTROLLER