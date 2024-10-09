#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/select.h>

#include "controller/user.h"
#include "model/request.h"
#include "model/response.h"
#include "controller/customer.h"
#include "controller/employee.h"

#define PORT 5003
#define MAX_ACTIVE_USERS 500

void close_sock(int);
int sfd;
int num_active_users = 0;
typedef struct s_active_user {
    pthread_t th;
    User *udata;
} active_user;
active_user *active_users[MAX_ACTIVE_USERS] = {0};

void mark_user(pthread_t th, User *user) {
    for (int i = 0; i < MAX_ACTIVE_USERS; i++) {
        if (active_users[i] == NULL) {
            active_users[i] = malloc(sizeof(active_user));
            active_users[i]->th = th;
            active_users[i]->udata = user;
            return;
        }
    }
}

void unmark_user(User *user) {
    for (int i = 0; i < MAX_ACTIVE_USERS; i++) {
        if (active_users[i] != NULL && strcmp(active_users[i]->udata->uname, user->uname) == 0) {
            free(active_users[i]);
            active_users[i] = NULL;
        }
    }
}

void close_user(User *user) {
    for (int i = 0; i < MAX_ACTIVE_USERS; i++) {
        if (active_users[i] != NULL && strcmp(active_users[i]->udata->uname, user->uname) == 0) {
            printf("closing existing session for user %s...\n", active_users[i]->udata->uname);
            pthread_cancel(active_users[i]->th);
            free(active_users[i]);
            active_users[i] = NULL;
        }
    }
}

struct thread_args {
    pthread_t *th;
    int cfd;
    int num_requests;
};

struct cleanup_args {
    int cfd;
    int tnum;
};

void thread_cleanup(void *arg) {
    struct cleanup_args args = *(struct cleanup_args*)(arg);
    printf("cancelling thread %d...\n", args.tnum);
    close(args.cfd);
}

void* service(void *arg) {
    struct thread_args args;
    memcpy(&args, (struct thread_args*)(arg), sizeof(struct thread_args));
    free(arg);
    printf("spawned thread for request #%d\n", args.num_requests);
    int cfd = args.cfd;
    struct cleanup_args cargs = {
        .cfd = cfd,
        .tnum = args.num_requests
    };
    pthread_cleanup_push(thread_cleanup, (void*)&cargs);
    User current_user;
    Request *req = malloc(sizeof(Request));
    Response *res;
    read(cfd, req, sizeof(Request));
    if (!(req->type == REQLOGIN)) {
        res = malloc(sizeof(Response));
        res->type = RESBADREQ;
        write(cfd, res, sizeof(Response));
        free(req);
        free(res);
        printf("exiting thread for request #%d\n", args.num_requests);
        return NULL;
    }
    login_res lres = user_login(req->data.login.uname, req->data.login.pw, &current_user);
    free(req);
    if (lres == LGNSUCCESS) {
        close_user(&current_user);
        mark_user(*args.th, &current_user);
        res = malloc(sizeof(Response));
        res->type = RESSUCCESS;
        res->data.login = current_user.role;
        printf("[%d] role of user: %d\n", args.num_requests, res->data.login);
        write(cfd, res, sizeof(Response));
        free(res);
        printf("[%d] user %s logged in\n", args.num_requests, current_user.uname);
    } else {
        res = malloc(sizeof(Response));
        res->type = RESUNAUTH;
        write(cfd, res, sizeof(Response));
        free(res);
        printf("exiting thread for request #%d\n", args.num_requests);
        return NULL;
    }
    while (1) {
        req = malloc(sizeof(Request));
        read(cfd, req, sizeof(Request));
        if (req->type == REQLOGOUT) {
            printf("[%d] user %s logged out\n", args.num_requests, current_user.uname);
            unmark_user(&current_user);
            break;
        }
        if (req->type == REQCHPW) {
            printf("[%d] user %s trying to change passwd\n", args.num_requests, current_user.uname);
            User tmp;
            res = malloc(sizeof(Response));
            if (user_login(current_user.uname, req->data.chpw.oldpw, &tmp) == LGNSUCCESS
                && user_update(UPDT_PASSWD, current_user.uname, NULL, req->data.chpw.newpw, current_user.role)) {
                res->type = RESSUCCESS;
            } else res->type = RESBADREQ;
            write(cfd, res, sizeof(Response));
            free(res);
        }
        if (req->type == REQREGISTER) {
            printf("[%d] user %s trying to create user\n", args.num_requests, current_user.uname);
            res = malloc(sizeof(Response));
            if (current_user.role == ADMIN || req->data.uregupdt.role < current_user.role) {
                if (user_register(req->data.uregupdt.uname, req->data.uregupdt.pw, req->data.uregupdt.role) == REGSUCCESS) {
                    if (req->data.uregupdt.role != ADMIN) {
                        if (req->data.uregupdt.role == CUSTOMER) {
                            Customer cust = {
                                .balance = 0,
                                .state = INACTIVE,
                                .pers_info = req->data.uregupdt.info
                            };
                            strcpy(cust.uname, req->data.uregupdt.uname);
                            cust_create(&cust);
                        } else {
                            Employee emp = {
                                .pers_info = req->data.uregupdt.info
                            };
                            strcpy(emp.uname, req->data.uregupdt.uname);
                            emp_create(&emp);
                        }
                    }
                    res->type = RESSUCCESS;
                } else res->type = RESBADREQ;
            } else res->type = RESUNAUTH;
            write(cfd, res, sizeof(Response));
            free(res);
        }
        free(req);
    }
    printf("exiting thread for request #%d\n", args.num_requests);
    close(cfd);
    pthread_cleanup_pop(0);
    return NULL;
}

int main() {
    int num_users = get_user_count();
    if (num_users <= 0) {
        printf("please create an admin account to continue:\nenter the username: ");
        char uname[128], pw[128];
        scanf("%s", uname);
        printf("enter the password: ");
        scanf("%s", pw);
        if (user_register(uname, pw, ADMIN)) {
            printf("admin register error!\n");
            return -1;
        }
    } else printf("%d user(s) registered\n", num_users);
    User test;
    if (user_login("admin", "admin", &test) == LGNSUCCESS) {
        printf("admin works\n");
    }

    struct sockaddr_in serv, cli;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("failed while opening socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("failed during setsockopt");
        close_sock(0);
    }

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(PORT);
    if (bind(sfd, (void*)&serv, sizeof(serv)) < 0) { 
        perror("failed at bind"); 
        close_sock(0);
    }
    if (listen(sfd, 5) < 0) { 
        perror("failed at listen"); 
        close_sock(0); 
    }
    printf("listening at port %d...\n", PORT);
    signal(SIGINT, close_sock);
    int num_requests = 0;
    socklen_t clisize = sizeof(cli);

    while (1) {
        int cfd = accept(sfd, (void*)&cli, &clisize);
        if (cfd < 0) {
            perror("failed at accept");
            close_sock(0);
        }
        num_requests++;
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->cfd = cfd;
        args->num_requests = num_requests;
        args->th = malloc(sizeof(pthread_t));
        pthread_create(args->th, NULL, service, (void *)(args));
    }

    return 0;
}

void close_sock(int s) {
    printf("\nshutting down server...\n");
    close(sfd);
    exit(errno);
}