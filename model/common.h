#ifndef COMMON_MODEL
#define COMMON_MODEL

typedef enum e_user_role {
    ADMIN,
    MANAGER,
    EMPLOYEE,
    CUSTOMER
} user_role;

typedef struct s_pinfo {
    char first_name[64];
    char last_name[64];
    char email[64];
    char phone[12];
    char dob[10]; // YYYY/MM/DD
    char gender;
    char city[32];
    long zip_code;
} PersonalInfo;

#endif // COMMON_MODEL