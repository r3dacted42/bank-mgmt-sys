#ifndef FEEDBACK_MODEL
#define FEEDBACK_MODEL

#define FDBK_CATEGORY_MAX 3
typedef enum e_feedback_category {
    FDBK_GENERAL,
    FDBK_TRANSFERS,
    FDBK_EMPLOYEES,
    FDBK_LOANS
} feedback_category;

#define FDBK_LIFESPAN_DAYS 15
#define FDBK_TEXT_LEN 256
typedef struct s_feedback {
    long submit_timestp;
    feedback_category category;
    char feedback_text[FDBK_TEXT_LEN];
} Feedback;

#endif // FEEDBACK_MODEL