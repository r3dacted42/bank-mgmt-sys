#ifndef TRANSACTION_MODEL
#define TRANSACTION_MODEL

typedef enum e_tr_type {
    CREDIT,
    DEBIT
} tr_type;

typedef struct s_transaction {
    char uname[128];
    tr_type type;
    char other_uname[128];
    long amount;
    int tr_time[3]; // HH, MM, SS
    int tr_date[3]; // YYYY, MM, DD
    long tr_timestp;
} Transaction;

#endif // TRANSACTION_MODEL