#ifndef FEEDBACK_CONTROLLER
#define FEEDBACK_CONTROLLER

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "../model/feedback.h"

#define FEEDBACK_DB_PATH "database/feedbacks.db"
#define _FEEDBACK_DB_CLN "database/feedbacks_.db"

void feedback_create(feedback_category category, const char *text) {
    Feedback temp = {
        .submit_timestp = time(NULL),
        .category = category
    };
    strcpy(temp.feedback_text, text);
    int fd = open(FEEDBACK_DB_PATH, O_CREAT | O_RDWR, 0644);
    lseek(fd, 0, SEEK_END);
    struct flock lck = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    write(fd, &temp, sizeof(Feedback));
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
}

void _feedback_clean() {
    int fd = open(FEEDBACK_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int fdc = open(_FEEDBACK_DB_CLN, O_CREAT | O_RDWR, 0644);
    Feedback temp;
    while (read(fd, &temp, sizeof(Feedback)) > 0) {
        long long timediff = time(NULL) - temp.submit_timestp;
        if (timediff < FDBK_LIFESPAN_DAYS * 24 * 60 * 60) {
            write(fdc, &temp, sizeof(Feedback));
        }
    }
    ftruncate(fd, 0);
    lseek(fd, 0, SEEK_SET);
    lseek(fdc, 0, SEEK_SET);
    while (read(fdc, &temp, sizeof(Feedback)) > 0) {
        write(fd, &temp, sizeof(Feedback));
    }
    unlink(_FEEDBACK_DB_CLN);
    close(fdc);
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
}

int feedback_read(Feedback ***fbkdata) {
    _feedback_clean();
    int fd = open(FEEDBACK_DB_PATH, O_CREAT | O_RDWR, 0644);
    struct flock lck = {
        .l_type = F_RDLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
        .l_pid = getpid()
    };
    fcntl(fd, F_SETLKW, &lck);
    int count = lseek(fd, 0, SEEK_END) / sizeof(Feedback);
    // printf("found %d records\n", count);
    *fbkdata = (Feedback**)calloc(count + 1, sizeof(Feedback*));
    (*fbkdata)[count] = NULL;
    lseek(fd, 0, SEEK_SET);
    Feedback temp;
    int index = count;
    while (read(fd, &temp, sizeof(Feedback)) > 0) {
        (*fbkdata)[--index] = (Feedback*)malloc(sizeof(Feedback));
        memcpy((*fbkdata)[index], &temp, sizeof(Feedback));
    }
    lck.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lck);
    close(fd);
    return count;
}

void feedback_free(Feedback ***fbkdata) {
    for (int i = 0; (*fbkdata)[i] != NULL; i++) free((*fbkdata)[i]);
    free(*fbkdata);
}

#endif // FEEDBACK_CONTROLLER