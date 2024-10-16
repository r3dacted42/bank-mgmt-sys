#ifndef TRANSACTION_MODEL
#define TRANSACTION_MODEL

#include "user.h"

typedef enum e_tr_type {
    CREDIT,
    DEBIT
} tr_type;

typedef enum e_tr_operation {
    DEPOSIT,
    WITHDRAW,
    TRANSFER,
    LOAN
} tr_operation;

typedef struct s_tran_list_item {
    char other_username[UN_LEN];
    tr_operation op;
    tr_type type;
    float amount;
    long timestp;
} tran_list_item;

typedef struct s_transaction {
    char uname[UN_LEN];
    char other_uname[UN_LEN];
    tr_operation op;
    tr_type type;
    float amount;
    long timestp;
} Transaction;

#endif // TRANSACTION_MODEL