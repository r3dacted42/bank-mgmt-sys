#ifndef LOAN_NODEL
#define LOAN_MODEL

#include "user.h"

#define LOAN_TYPE_MAX 5
typedef enum e_loan_type {
    LOAN_PERSONAL,
    LOAN_HOME,
    LOAN_VEHICLE,
    LOAN_EDUCATION,
    LOAN_MORTGAGE,
    LOAN_GOLD
} loan_type;

#define EMP_STAT_MAX 2
typedef enum e_employment_status {
    UNEMPLOYED,
    SELF_EMPLOYED,
    EMPLOYED
} employment_status;

typedef enum e_loan_status {
    LOAN_PENDING,
    LOAN_APPROVED,
    LOAN_REJECTED
} loan_status;

typedef struct s_loan {
    long loan_id; // auto inc
    char applicant_cust[UN_LEN];
    // set by manager
    char assignee_emp[UN_LEN];
    
    // set by customer
    loan_type type;
    float req_amount;
    float annual_income;
    float credit_score;
    employment_status emp_status;
    int years_of_emp;
    
    // set by employee
    float acpt_amount;
    float interest_rate;
    loan_status status;

    long apply_timestp;
    long review_timestp;
    char rejection_reason[128];
} Loan;

#endif // LOAN_MODEL