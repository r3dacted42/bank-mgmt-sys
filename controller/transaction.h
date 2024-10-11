#ifndef TRANSACTION_CONTROLLER
#define TRANSACTION_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "../model/transaction.h"
#include "../controller/customer.h"

#define TRAN_DB_PATH "database/transactions.db"

int tran_read_usr(const char *uname, Transaction *trdata[]) {
    int fd = open(TRAN_DB_PATH, O_CREAT | O_RDONLY, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int count = 0;
    Transaction tmp;
    while (read(fd, &tmp, sizeof(Transaction)) > 0) {
        if (strcmp(tmp.uname, uname) == 0) count++;
    }
    trdata = (Transaction**)calloc(count + 1, sizeof(Transaction*));
    trdata[count] = NULL;
    lseek(fd, 0, SEEK_SET);
    int index = 0;
    while (read(fd, &tmp, sizeof(Transaction)) > 0) {
        if (strcmp(tmp.uname, uname) == 0) {
            trdata[index] = (Transaction*)malloc(sizeof(Transaction));
            memcpy(trdata[index], &tmp, sizeof(Transaction));
            index++;
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return count;
}

void tran_free(Transaction *trdata[]) {
    int i = 0;
    for ( ; trdata[i] != NULL; i++) {
        free(trdata[i]);
        free(&trdata[i]);
    }
    free(&trdata[i]); // trailing NULL
}

// time_t rawtime;
// time(&rawtime);
// struct tm *timeinfo = localtime(&rawtime);

bool tran_transfer(const char *un, const char *oun, long amt) {
    int fd = open(CUST_DB_PATH, O_RDWR);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Customer tmp, ctmp = {0}, octmp = {0};
    int idx = 0, cidx = -1, ocidx = -1;
    while (read(fd, &tmp, sizeof(Customer)) > 0) {
        if (strcmp(tmp.uname, un) == 0) { ctmp = tmp; cidx = idx; }
        if (strcmp(tmp.uname, oun) == 0) { octmp = tmp; ocidx = idx; }
        idx++;
    }
    if (ctmp.balance < amt) {
        lck.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lck);
        close(fd);
        return false;
    }
    ctmp.balance -= amt;
    octmp.balance += amt;
    lck.l_type = F_WRLCK;
    lck.l_start = cidx * sizeof(Customer);
    lck.l_len = sizeof(Customer);
    struct flock olck = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = ocidx * sizeof(Customer),
        .l_len = sizeof(Customer),
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    fcntl(fd, F_SETLKW, &olck);
    lseek(fd, cidx * sizeof(Customer), SEEK_SET);
    write(fd, &ctmp, sizeof(Customer));
    lseek(fd, ocidx * sizeof(Customer), SEEK_SET);
    write(fd, &octmp, sizeof(Customer));
    lck.l_type = F_UNLCK;
    olck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    fcntl(fd, F_SETLK, &olck);
    // add transaction records
    Transaction trdata = {
        .type = DEBIT,
        .amount = amt,
        .timestp = time(NULL)
    };
    strcpy(trdata.uname, un);
    strcpy(trdata.other_uname, oun);
    int fd = open(TRAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, 0, SEEK_END);
    // un -> oun
    write(fd, &trdata, sizeof(Transaction));
    trdata.type = CREDIT;
    strcpy(trdata.uname, oun);
    strcpy(trdata.other_uname, un);
    // oun <- un
    write(fd, &trdata, sizeof(Transaction));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
}

bool tran_deposit(const char *un, long amt) {
    int fd = open(CUST_DB_PATH, O_RDWR);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Customer ctmp;
    int cidx = 0;
    while (read(fd, &ctmp, sizeof(Customer)) > 0) {
        if (strcmp(ctmp.uname, un) == 0) break;
        cidx++;
    }
    if (strcmp(ctmp.uname, un)) {
        lck.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lck);
        close(fd);
        return false;
    }
    ctmp.balance += amt;
    lck.l_type = F_WRLCK;
    lck.l_start = cidx * sizeof(Customer);
    lck.l_len = sizeof(Customer);
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, cidx * sizeof(Customer), SEEK_SET);
    write(fd, &ctmp, sizeof(Customer));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    // add transaction record
    Transaction trdata = {
        .type = CREDIT,
        .amount = amt,
        .timestp = time(NULL)
    };
    strcpy(trdata.uname, un);
    memset(trdata.other_uname, 0, UN_LEN);
    fd = open(TRAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, 0, SEEK_END);
    write(fd, &trdata, sizeof(Transaction));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

bool tran_withdraw(const char *un, long amt) {
    int fd = open(CUST_DB_PATH, O_RDWR);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    Customer ctmp;
    int cidx = 0;
    while (read(fd, &ctmp, sizeof(Customer)) > 0) {
        if (strcmp(ctmp.uname, un) == 0) break;
        cidx++;
    }
    if (strcmp(ctmp.uname, un) || ctmp.balance < amt) {
        lck.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lck);
        close(fd);
        return false;
    }
    ctmp.balance -= amt;
    lck.l_type = F_WRLCK;
    lck.l_start = cidx * sizeof(Customer);
    lck.l_len = sizeof(Customer);
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, cidx * sizeof(Customer), SEEK_SET);
    write(fd, &ctmp, sizeof(Customer));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    // add transaction record
    Transaction trdata = {
        .type = DEBIT,
        .amount = amt,
        .timestp = time(NULL)
    };
    strcpy(trdata.uname, un);
    memset(trdata.other_uname, 0, UN_LEN);
    fd = open(TRAN_DB_PATH, O_CREAT | O_RDWR, 0644);
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    lseek(fd, 0, SEEK_END);
    write(fd, &trdata, sizeof(Transaction));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

#endif // TRANSACTION_CONTROLLER