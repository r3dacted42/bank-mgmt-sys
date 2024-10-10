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

void sigpipe_handler(int sig) {
    // printf("got SIGPIPE, client exited without closing pipe :(\n");
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
    Request req;
    Response res;
    read(cfd, &req, sizeof(Request));
    if (!(req.type == REQLOGIN)) {
        res.type = RESBADREQ;
        write(cfd, &res, sizeof(Response));
        printf("exiting thread for request #%d\n", args.num_requests);
        return NULL;
    }
    login_res lres = user_login(req.data.login.uname, req.data.login.pw, &current_user);
    if (lres == LGNSUCCESS) {
        close_user(&current_user);
        mark_user(*args.th, &current_user);
        res.type = RESSUCCESS;
        res.data.login = current_user.role;
        printf("[%d] role of user: %d\n", args.num_requests, res.data.login);
        write(cfd, &res, sizeof(Response));
        printf("[%d] user %s logged in\n", args.num_requests, current_user.uname);
    } else {
        res.type = RESUNAUTH;
        write(cfd, &res, sizeof(Response));
        printf("exiting thread for request #%d\n", args.num_requests);
        return NULL;
    }
    while (1) {
        read(cfd, &req, sizeof(Request));
        if (req.type == REQLOGOUT) {
            printf("[%d] user %s logged out\n", args.num_requests, current_user.uname);
            unmark_user(&current_user);
            break;
        }
        if (req.type == REQCHPW) {
            printf("[%d] user %s trying to change passwd\n", args.num_requests, current_user.uname);
            User tmp;
            if (user_login(current_user.uname, req.data.chpw.oldpw, &tmp) == LGNSUCCESS
                && user_update(UPDT_PASSWD, current_user.uname, NULL, req.data.chpw.newpw, current_user.role)) {
                res.type = RESSUCCESS;
            } else res.type = RESBADREQ;
        }
        if (req.type == REQREGISTER) {
            printf("[%d] user %s trying to create user\n", args.num_requests, current_user.uname);
            if (current_user.role == ADMIN || req.data.ureg.role > current_user.role) {
                if (user_register(req.data.ureg.uname, req.data.ureg.pw, req.data.ureg.role) == REGSUCCESS) {
                    if (req.data.ureg.role != ADMIN) {
                        if (req.data.ureg.role == CUSTOMER) {
                            Customer cust = {
                                .balance = 0,
                                .state = INACTIVE,
                                .pers_info = req.data.ureg.info
                            };
                            strcpy(cust.uname, req.data.ureg.uname);
                            cust_create(&cust);
                        } else {
                            Employee emp = {
                                .pers_info = req.data.ureg.info
                            };
                            strcpy(emp.uname, req.data.ureg.uname);
                            emp_create(&emp);
                        }
                    }
                    res.type = RESSUCCESS;
                } else res.type = RESBADREQ;
            } else res.type = RESUNAUTH;
        }
        if (req.type == REQGETUSR) {
            printf("[%d] user %s trying to get data of (%s)\n", args.num_requests, current_user.uname, req.data.getusr);
            User temp;
            if (user_read(req.data.getusr, &temp)) {
                res.data.getusr.role = temp.role;
                res.type = RESSUCCESS;
                if (temp.role == CUSTOMER) {
                    Customer ctemp;
                    if (cust_read(req.data.getusr, &ctemp)) {
                        printf("found customer data\n");
                    }
                    memcpy(res.data.getusr.info.first_name, ctemp.pers_info.first_name, sizeof(PersonalInfo));
                    printf("read %s, copied %s\n", ctemp.pers_info.last_name, res.data.getusr.info.last_name);
                    res.data.getusr.cust_state = ctemp.state;
                    res.data.getusr.cust_balance = ctemp.balance;
                } else if (temp.role == EMPLOYEE || temp.role == MANAGER) {
                    Employee etemp;
                    emp_read(req.data.getusr, &etemp);
                    memcpy(&res.data.getusr.info, &etemp.pers_info, sizeof(PersonalInfo));
                } else if (temp.role == ADMIN) {
                    res.type = RESUNAUTH;
                }
            } else res.type = RESBADREQ;
        }
        if (req.type == REQUPDTUSR) {
            printf("[%d] user %s trying to update (%s)\n", args.num_requests, current_user.uname, req.data.uupdt.uname);
            User utemp;
            if (!user_read(req.data.uupdt.uname, &utemp)) res.type = RESBADREQ;
            else if (current_user.role < utemp.role) {
                int uopt = 0;
                if (strcmp(req.data.uupdt.nuname, utemp.uname)) uopt = UPDT_UNAME;
                if (req.data.uupdt.pw[0] != 0) uopt |= UPDT_PASSWD;
                if (req.data.uupdt.role != utemp.role) uopt |= UPDT_ROLE;
                printf("update opt: %d\n", uopt);
                user_update(uopt, utemp.uname, req.data.uupdt.nuname, req.data.uupdt.pw, req.data.uupdt.role);
                if (uopt & UPDT_ROLE) {
                    if (utemp.role == EMPLOYEE || utemp.role == MANAGER) 
                        emp_delete(utemp.uname);
                    if (utemp.role == CUSTOMER)
                        cust_delete(utemp.uname);
                    if (req.data.uupdt.role == EMPLOYEE || req.data.uupdt.role == MANAGER) {
                        Employee em = {
                            .pers_info = req.data.uupdt.info
                        };
                        if (uopt & UPDT_UNAME) strcpy(em.uname, req.data.uupdt.nuname);
                        else strcpy(em.uname, utemp.uname);
                        emp_create(&em);
                    }
                    if (req.data.uupdt.role == CUSTOMER) {
                        Customer cu = {
                            .balance = 0,
                            .pers_info = req.data.uupdt.info,
                            .state = INACTIVE
                        };
                        if (uopt & UPDT_UNAME) strcpy(cu.uname, req.data.uupdt.nuname);
                        else strcpy(cu.uname, utemp.uname);
                        cust_create(&cu);
                    }
                } else {
                    if (utemp.role == MANAGER || utemp.role == EMPLOYEE) {
                        Employee em = {
                            .pers_info = req.data.uupdt.info
                        };
                        if (uopt & UPDT_UNAME) strcpy(em.uname, req.data.uupdt.nuname);
                        else strcpy(em.uname, utemp.uname);
                        emp_update(utemp.uname, &em);
                    }
                    if (utemp.role == CUSTOMER) {
                        Customer cu = {
                            .balance = req.data.uupdt.cust_balance,
                            .state = req.data.uupdt.cust_state,
                            .pers_info = req.data.uupdt.info
                        };
                        if (uopt & UPDT_UNAME) strcpy(cu.uname, req.data.uupdt.nuname);
                        else strcpy(cu.uname, utemp.uname);
                        cust_update(utemp.uname, &cu);
                    }
                }
                res.type = RESSUCCESS;
            } else res.type = RESUNAUTH;
        }
        if (req.type == REQDLTUSR) {
            printf("[%d] user %s is trying to delete (%s)\n", args.num_requests, current_user.uname, req.data.udlt);
            User utmp;
            if (!user_read(req.data.udlt, &utmp)) {
                res.type = RESBADREQ;
            } else if (current_user.role < utmp.role) {
                user_delete(utmp.uname);
                if (utmp.role == EMPLOYEE || utmp.role == MANAGER) emp_delete(utmp.uname);
                else cust_delete(utmp.uname);
                res.type = RESSUCCESS;
            } else res.type = RESUNAUTH;
        }
        write(cfd, &res, sizeof(Response));
        memset(&res, 0, sizeof(Response));
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
        char uname[UN_LEN], pw[PW_LEN];
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
    signal(SIGPIPE, sigpipe_handler);
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

    printf("control shouldn't reach here!!!!\n");
    return -1;
}

void close_sock(int s) {
    printf("\nshutting down server...\n");
    close(sfd);
    exit(errno);
}