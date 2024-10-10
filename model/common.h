#ifndef COMMON_MODEL
#define COMMON_MODEL

typedef enum e_user_role {
    ADMIN,
    MANAGER,
    EMPLOYEE,
    CUSTOMER
} user_role;

typedef struct s_pinfo {
    char first_name[16];
    char last_name[16];
    char email[64];
    char phone[13];
    char dob[11]; // YYYY/MM/DD
    char gender;
    char city[32];
    char zip_code[7];
} PersonalInfo;

#endif // COMMON_MODEL