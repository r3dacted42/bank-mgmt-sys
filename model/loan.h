#ifndef LOAN_NODEL
#define LOAN_MODEL

#include "user.h"

typedef enum e_loan_type {
    LOAN_PERSONAL,
    LOAN_HOME,
    LOAN_VEHICLE,
    LOAN_EDUCATION,
    LOAN_MORTGAGE,
    LAON_GOLD
} loan_type;

typedef enum e_employment_status {
    UNEMPLOYED,
    SELF_EMPLOYED,
    EMPLOYED
} employment_status;

typedef enum e_loan_status {
    LOAN_PENDING,
    LOAN_ACCEPTED,
    LOAN_REJECTED
} loan_status;

typedef struct s_loan {
    char applicant_cust[UN_LEN];
    char assignee_emp[UN_LEN];
    
    // some params to help decide loan eligibilty
    long annual_income;
    float credit_score;
    employment_status emp_status;
    int years_of_emp;
    
    // loan params
    loan_type type;
    float amount;
    float interest_rate;
    
    bool approved;
    long apply_timestp;
    long decision_timestp;
    char rejection_reason[128];
} Loan;

#endif // LOAN_MODEL