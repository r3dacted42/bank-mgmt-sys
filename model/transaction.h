#ifndef TRANSACTION_MODEL
#define TRANSACTION_MODEL

typedef enum e_tr_type {
    CREDIT,
    DEBIT
} tr_type;

typedef struct s_transaction {
    long cuid;
    long trid;
    tr_type type;
    long other_cuid;
    long amount;
    long tr_time[3]; // HH, MM, SS
    int tr_date[3]; // YYYY, MM, DD
    long tr_timestp;
} Transaction;

#endif // TRANSACTION_MODEL