#ifndef TRANSACTION_CONTROLLER
#define TRANSACTION_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
// #include <stdio.h>
#include <time.h>
#include "../model/transaction.h"
#include "../controller/customer.h"

#define TRAN_DB_PATH "database/transactions.db"

int tran_read_usr(const char *uname, Transaction ***trdata) {
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
    // printf("found %d records!\n", count);
    *trdata = (Transaction**)calloc(count + 1, sizeof(Transaction*));
    (*trdata)[count] = NULL;
    lseek(fd, 0, SEEK_SET);
    for (int index = count - 1; read(fd, &tmp, sizeof(Transaction)) > 0; ) {
        if (strcmp(tmp.uname, uname) == 0) {
            (*trdata)[index] = (Transaction*)malloc(sizeof(Transaction));
            memcpy((*trdata)[index--], &tmp, sizeof(Transaction));
        }
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return count;
}

void tran_free(Transaction ***trdata) {
    for (int i = 0; (*trdata)[i] != NULL; i++) free((*trdata)[i]);
    free(*trdata);
}

bool tran_transfer(const char *un, const char *oun, float amt) {
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
    bool foundc = false, foundo = false;
    int idx = 0, cidx = -1, ocidx = -1;
    while (read(fd, &tmp, sizeof(Customer)) > 0) {
        if (strcmp(tmp.uname, un) == 0) { foundc = true; ctmp = tmp; cidx = idx; }
        if (strcmp(tmp.uname, oun) == 0) { foundo = true; octmp = tmp; ocidx = idx; }
        idx++;
    }
    if (!foundc || !foundo || ctmp.balance < amt) {
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
        .op = TRANSFER,
        .type = DEBIT,
        .amount = amt,
        .timestp = time(NULL)
    };
    Transaction trdatao = {
        .op = TRANSFER,
        .type = CREDIT,
        .amount = amt,
        .timestp = time(NULL)
    };
    strcpy(trdata.uname, un);
    strcpy(trdata.other_uname, oun);
    strcpy(trdatao.uname, oun);
    strcpy(trdatao.other_uname, un);
    fd = open(TRAN_DB_PATH, O_CREAT | O_RDWR | O_APPEND, 0644);
    lck.l_type = F_WRLCK,
    lck.l_whence = SEEK_SET,
    lck.l_start = 0,
    lck.l_len = 0,
    lck.l_pid = getpid();
    fcntl(fd, F_SETLKW, &lck);
    // lseek(fd, 0, SEEK_END);
    // un -> oun
    write(fd, &trdata, sizeof(Transaction));
    // oun <- un
    write(fd, &trdatao, sizeof(Transaction));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    return true;
}

// op must be DEPOSIT or LOAN
bool tran_deposit(const char *un, float amt, tr_operation op) {
    if (op != DEPOSIT && op != LOAN) return false;
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
    bool found = false;
    int cidx = 0;
    while (read(fd, &ctmp, sizeof(Customer)) > 0) {
        if (strcmp(ctmp.uname, un) == 0) { found = true; break; }
        cidx++;
    }
    if (!found) {
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
        .op = op,
        .type = CREDIT,
        .amount = amt,
        .timestp = time(NULL)
    };
    strcpy(trdata.uname, un);
    fd = open(TRAN_DB_PATH, O_CREAT | O_RDWR | O_APPEND, 0644);
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    // lseek(fd, 0, SEEK_END);
    write(fd, &trdata, sizeof(Transaction));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

bool tran_withdraw(const char *un, float amt) {
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
    bool found = false;
    int cidx = 0;
    while (read(fd, &ctmp, sizeof(Customer)) > 0) {
        if (strcmp(ctmp.uname, un) == 0) { found = true; break; }
        cidx++;
    }
    if (!found || ctmp.balance < amt) {
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
    trdata.op = WITHDRAW;
    fd = open(TRAN_DB_PATH, O_CREAT | O_RDWR | O_APPEND, 0644);
    lck.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &lck);
    // lseek(fd, 0, SEEK_END);
    write(fd, &trdata, sizeof(Transaction));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return true;
}

#endif // TRANSACTION_CONTROLLER