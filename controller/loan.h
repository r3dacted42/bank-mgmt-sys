#ifndef LOAN_CONTROLLER
#define LOAN_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "../model/loan.h"
#include "transaction.h"

#define LOAN_DB_PATH "database/loans.db"

// store max_id in the first sizeof(id) bytes of the db file
#define SKIP_ID lseek(fd, sizeof(long), SEEK_SET)

int loan_read_cust(const char *cun, Loan ***lndata) {
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int count = 0;
    Loan tmp;
    SKIP_ID;
    while (read(fd, &tmp, sizeof(Loan)) > 0) {
        if (strcmp(tmp.applicant_cust, cun) == 0) count++;
    }
    // printf("found %d records!\n", count);
    *lndata = (Loan**)calloc(count + 1, sizeof(Loan*));
    (*lndata)[count] = NULL;
    SKIP_ID;
    for (int index = 0; read(fd, &tmp, sizeof(Loan)) > 0; ) {
        if (strcmp(tmp.applicant_cust, cun) == 0) {
            (*lndata)[index] = (Loan*)malloc(sizeof(Loan));
            memcpy((*lndata)[index++], &tmp, sizeof(Loan));
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return count;
}

int loan_read_empl(const char *eun, Loan***lndata) {
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int count = 0;
    Loan tmp;
    SKIP_ID;
    while (read(fd, &tmp, sizeof(Loan)) > 0) {
        if (strcmp(tmp.assignee_emp, eun) == 0 && tmp.status == LOAN_PENDING) count++;
    }
    // printf("found %d records!\n", count);
    *lndata = (Loan**)calloc(count + 1, sizeof(Loan*));
    (*lndata)[count] = NULL;
    SKIP_ID;
    for (int index = 0; read(fd, &tmp, sizeof(Loan)) > 0; ) {
        if (strcmp(tmp.assignee_emp, eun) == 0 && tmp.status == LOAN_PENDING) {
            (*lndata)[index] = (Loan*)malloc(sizeof(Loan));
            memcpy((*lndata)[index++], &tmp, sizeof(Loan));
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return count;
}

int loan_read_man(Loan ***lndata) {
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int count = 0;
    Loan tmp;
    SKIP_ID;
    while (read(fd, &tmp, sizeof(Loan)) > 0) {
        if (tmp.assignee_emp[0] == 0) count++;
    }
    // printf("found %d records!\n", count);
    *lndata = (Loan**)calloc(count + 1, sizeof(Loan*));
    (*lndata)[count] = NULL;
    SKIP_ID;
    for (int index = 0; read(fd, &tmp, sizeof(Loan)) > 0; ) {
        if (tmp.assignee_emp[0] == 0) {
            (*lndata)[index] = (Loan*)malloc(sizeof(Loan));
            memcpy((*lndata)[index++], &tmp, sizeof(Loan));
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return count;
}

void loan_free(Loan ***lndata) {
    for (int i = 0; (*lndata)[i] != NULL; i++) free((*lndata)[i]);
    free(*lndata);
}

/// @brief create new loan application
/// @param lndata loan application info
/// @return true if entry successfully created, false otherwise
bool loan_apply(Loan *lndata) {
    if (lndata->type < 0 || lndata->type > LOAN_TYPE_MAX) return false;
    if (lndata->emp_status < 0 || lndata->emp_status > EMP_STAT_MAX) return false;
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    long max_id = 0;
    read(fd, &max_id, sizeof(long));
    printf("read max_id: %ld\n", max_id++);
    // update max_id
    lseek(fd, 0, SEEK_SET);
    write(fd, &max_id, sizeof(long));
    // write loan data
    lseek(fd, 0, SEEK_END);
    lndata->loan_id = max_id;
    lndata->apply_timestp = time(NULL);
    lndata->status = LOAN_PENDING;
    memset(lndata->assignee_emp, 0, UN_LEN);
    write(fd, lndata, sizeof(Loan));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

typedef enum e_ln_assgn_result {
    LNASGN_SUCCESS,
    LNASGN_ALREADYASSGND,
    LNASGN_NOTFOUND
} ln_assgn_result;

ln_assgn_result loan_assign(long loan_id, char *eun) {
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Loan temp;
    ln_assgn_result result = LNASGN_NOTFOUND;
    SKIP_ID;
    while (read(fd, &temp, sizeof(Loan)) > 0) {
        if (loan_id == temp.loan_id) {
            if (temp.assignee_emp[0] != 0) {
                result = LNASGN_ALREADYASSGND;
                strcpy(eun, temp.assignee_emp);
                break;
            }
            lck.l_type = F_WRLCK;
            lck.l_whence = SEEK_CUR,
            lck.l_start = lseek(fd, -1 * sizeof(Loan), SEEK_CUR),
            lck.l_len = sizeof(Loan);
            fcntl(fd, F_SETLKW, &lck);
            strcpy(temp.assignee_emp, eun);
            write(fd, &temp, sizeof(Loan));
            result = LNASGN_SUCCESS;
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return result;
}

typedef enum e_ln_rv_result {
    LNRV_SUCCESS,
    LNRV_ALREADYRVD,
    LNRV_NOTFOUND
} ln_rv_result;

ln_rv_result loan_approve(long loan_id, float amt, float rate) {
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Loan temp;
    ln_rv_result result = LNRV_NOTFOUND;
    SKIP_ID;
    while (read(fd, &temp, sizeof(Loan)) > 0) {
        if (loan_id == temp.loan_id) {
            if (temp.status != LOAN_PENDING) {
                result = LNRV_ALREADYRVD;
                break;
            }
            lck.l_type = F_WRLCK;
            lck.l_whence = SEEK_CUR,
            lck.l_start = lseek(fd, -1 * sizeof(Loan), SEEK_CUR),
            lck.l_len = sizeof(Loan);
            fcntl(fd, F_SETLKW, &lck);
            temp.status = LOAN_APPROVED;
            temp.review_timestp = time(NULL);
            temp.acpt_amount = amt;
            temp.interest_rate = rate;
            write(fd, &temp, sizeof(Loan));
            // creadit amount to customer
            tran_deposit(temp.applicant_cust, temp.acpt_amount, LOAN);
            result = LNRV_SUCCESS;
            break;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return result;
}

ln_rv_result loan_reject(long loan_id, const char *reason) {
    int fd = open(LOAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Loan temp;
    ln_rv_result result = LNRV_NOTFOUND;
    SKIP_ID;
    while (read(fd, &temp, sizeof(Loan)) > 0) {
        if (loan_id == temp.loan_id) {
            if (temp.status != LOAN_PENDING) {
                result = LNRV_ALREADYRVD;
                break;
            }
            lck.l_type = F_WRLCK;
            lck.l_whence = SEEK_CUR,
            lck.l_start = lseek(fd, -1 * sizeof(Loan), SEEK_CUR),
            lck.l_len = sizeof(Loan);
            fcntl(fd, F_SETLKW, &lck);
            temp.status = LOAN_REJECTED;
            temp.review_timestp = time(NULL);
            memcpy(temp.rejection_reason, reason, 128);
            write(fd, &temp, sizeof(Loan));
            result = LNRV_SUCCESS;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return result;
}

#endif // LOAN_CONTROLLER