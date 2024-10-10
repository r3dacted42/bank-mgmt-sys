#ifndef TRANSACTION_MODEL
#define TRANSACTION_MODEL

#include "user.h"

typedef enum e_tr_type {
    CREDIT,
    DEBIT
} tr_type;

typedef struct s_transaction {
    char uname[UN_LEN];
    tr_type type;
    char other_uname[UN_LEN];
    long amount;
    int tr_time[3]; // HH, MM, SS
    int tr_date[3]; // YYYY, MM, DD
    long tr_timestp;
} Transaction;

#endif // TRANSACTION_MODEL