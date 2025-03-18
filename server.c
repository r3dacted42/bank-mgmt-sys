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
#include "controller/transaction.h"
#include "controller/loan.h"

#define PORT 5003
#define MAX_ACTIVE_USERS 512

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
        sprintf(res.data.msg, "must login before doing anything else!");
        write(cfd, &res, sizeof(Response));
        printf("exiting thread for request #%d\n", args.num_requests);
        close(cfd);
        return NULL;
    }
    login_res lres = user_login(req.data.login.uname, req.data.login.pw, &current_user);
    if (lres == LGNSUCCESS) {
        if (current_user.role == CUSTOMER) {
            Customer temp;
            cust_read(current_user.uname, &temp);
            if (temp.state == CACC_INACTIVE) {
                res.type = RESUNAUTH;
                sprintf(res.data.msg, "Account inactive, please contact a manager.");
                write(cfd, &res, sizeof(Response));
                printf("exiting thread for request #%d\n", args.num_requests);
                close(cfd);
                return NULL;
            }
        }
        close_user(&current_user);
        mark_user(*args.th, &current_user);
        res.type = RESSUCCESS;
        res.data.login = current_user.role;
        printf("[%d] role of user: %d\n", args.num_requests, res.data.login);
        write(cfd, &res, sizeof(Response));
        printf("[%d] user %s logged in\n", args.num_requests, current_user.uname);
    } else {
        res.type = RESUNAUTH;
        if (lres == LGNWRONGPW) sprintf(res.data.msg, "Wrong password entered!");
        if (lres == LGNUNAMENOTFOUND) sprintf(res.data.msg, "Username not found!");
        write(cfd, &res, sizeof(Response));
        printf("exiting thread for request #%d\n", args.num_requests);
        close(cfd);
        return NULL;
    }
    while (1) {
        int bytesread = read(cfd, &req, sizeof(Request));
        bool bufdata = false;
        if (bytesread <= 0 || req.type == REQLOGOUT) {
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
            printf("[%d] user %s trying to create user (%s, %d)\n", args.num_requests, current_user.uname,
            req.data.ureg.uname, req.data.ureg.role);
            if (current_user.role == ADMIN || req.data.ureg.role > current_user.role) {
                if (user_register(req.data.ureg.uname, req.data.ureg.pw, req.data.ureg.role) == REGSUCCESS) {
                    if (req.data.ureg.role != ADMIN) {
                        if (req.data.ureg.role == CUSTOMER) {
                            Customer cust = {
                                .balance = 0,
                                .state = CACC_INACTIVE,
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
                    cust_read(req.data.getusr, &ctemp);
                    memcpy(&res.data.getusr.info, &ctemp.pers_info, sizeof(PersonalInfo));
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
        if (req.type == REQGETUSRROLE) {
            printf("[%d] user %s trying to get role of (%s)\n", args.num_requests, current_user.uname, req.data.getusr);
            User temp;
            if (user_read(req.data.getusr, &temp)) {
                res.data.getusrrole = temp.role;
                res.type = RESSUCCESS;
            } else res.type = RESBADREQ;
        }
        if (req.type == REQUPDTUSR) {
            printf("[%d] user %s trying to update (%s)\n", args.num_requests, current_user.uname, req.data.uupdt.uname);
            printf("un:%s nun:%s pw:%s rl:%d | fn:%s ln:%s\n", req.data.uupdt.uname, req.data.uupdt.nuname, req.data.uupdt.pw, 
            req.data.uupdt.role, req.data.uupdt.info.first_name, req.data.uupdt.info.last_name);
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
                            .state = CACC_INACTIVE
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
        if (req.type == REQGETBAL) {
            printf("[%d] user %s trying to get balance\n", args.num_requests, current_user.uname);
            Customer ctmp;
            res.type = cust_read(current_user.uname, &ctmp) ? RESSUCCESS : RESBADREQ;
            if (res.type == RESSUCCESS) res.data.getbal = ctmp.balance;
        }
        if (req.type == REQDEPOSIT) {
            printf("[%d] user %s trying to deposit moni\n", args.num_requests, current_user.uname);
            res.type = tran_deposit(current_user.uname, req.data.baldelta, DEPOSIT) ? RESSUCCESS : RESBADREQ;
        }
        if (req.type == REQWITHDRAW) {
            printf("[%d] user %s trying to withdraw moni\n", args.num_requests, current_user.uname);
            res.type = tran_withdraw(current_user.uname, req.data.baldelta) ? RESSUCCESS : RESBADREQ;
        }
        if (req.type == REQTRANSFER) {
            printf("[%d] user %s trying to transfer moni to (%s)\n", args.num_requests, current_user.uname, req.data.transfer.oun);
            res.type = tran_transfer(current_user.uname, req.data.transfer.oun, req.data.transfer.amt) ? RESSUCCESS : RESBADREQ;
        }
        if (req.type == REQVIEWTRAN) {
            printf("[%d] user %s trying to view transaction history (page %d)\n", args.num_requests, current_user.uname, req.data.viewtran.page_num);
            Transaction **trlist;
            int len = 0;
            if (req.data.viewtran.un[0]) len = tran_read_usr(req.data.viewtran.un, &trlist);
            else len = tran_read_usr(current_user.uname, &trlist);
            if (len > 0) {
                res.type = RESSUCCESS;
                res.data.viewtran.total_pages = len / 5 + (len % 5 != 0); // one extra page if remainder exists
                if (req.data.viewtran.page_num < res.data.viewtran.total_pages) {
                    res.data.viewtran.page_len = len - req.data.viewtran.page_num * 5;
                    if (res.data.viewtran.page_len > 5) res.data.viewtran.page_len = 5;
                    for (int i = 0; i < res.data.viewtran.page_len && trlist[req.data.viewtran.page_num * 5 + i] != NULL; i++) {
                        int lidx = req.data.viewtran.page_num * 5 + i;
                        memcpy(&(res.data.viewtran.lst[i].other_username), &(trlist[lidx]->other_uname), sizeof(tran_list_item));
                    }
                } else res.type = RESBADREQ;
            } else res.type = RESEMPTY;
            tran_free(&trlist);
        }
        if (req.type == REQLNAPPL) {
            printf("[%d] user %s trying to apply for loan\n", args.num_requests, current_user.uname);
            res.type = (loan_apply(&req.data.loanappl) ? RESSUCCESS : RESBADREQ);
        }
        if (req.type == REQLNASSGNGET) {
            printf("[%d] user %s trying to get unassigned loans\n", args.num_requests, current_user.uname);
            if (current_user.role < MANAGER) res.type = RESUNAUTH;
            else {
                Loan **lnlist;
                int count = loan_read_man(&lnlist);
                if (count > 0) {
                    res.type = RESSUCCESS;
                    res.data.bufcount = count;
                    bufdata = true;
                    write(cfd, &res, sizeof(Response));
                    for (int i = 0; i < count; i++) {
                        write(cfd, lnlist[i], sizeof(Loan));
                    }
                } else res.type = RESEMPTY;
                loan_free(&lnlist);
            }
        }
        if (req.type == REQGETEMPS) {
            printf("[%d] user %s trying to get employees\n", args.num_requests, current_user.uname);
            if (current_user.role < MANAGER) res.type = RESUNAUTH;
            else {
                Employee **emlist;
                int count = emp_read_all_no_man(&emlist);
                if (count > 0) {
                    res.type = RESSUCCESS;
                    res.data.bufcount = count;
                    bufdata = true;
                    write(cfd, &res, sizeof(Response));
                    for (int i = 0; i < count; i++) write(cfd, emlist[i], sizeof(Employee));
                } else res.type = RESEMPTY;
                emp_free(&emlist);
            }
        }
        if (req.type == REQLNASSGNPOST) {
            printf("[%d] user %s trying to assign loan id (%ld)\n", args.num_requests, current_user.uname, req.data.lnassgn.loan_id);
            if (current_user.role < MANAGER) res.type = RESUNAUTH;
            else {
                ln_assgn_result result = loan_assign(req.data.lnassgn.loan_id, req.data.lnassgn.eun);
                if (result == LNASGN_SUCCESS) res.type = RESSUCCESS;
                else {
                    res.type = RESBADREQ;
                    if (result == LNASGN_ALREADYASSGND) sprintf(res.data.msg, "Loan application already assigned to %s", req.data.lnassgn.eun);
                    else sprintf(res.data.msg, "Loan ID not found");
                }
            }
        }
        if (req.type == REQLNRVGET) {
            printf("[%d] user %s trying to get loans for review\n", args.num_requests, current_user.uname);
            if (current_user.role < EMPLOYEE) res.type = RESUNAUTH;
            else {
                Loan **lnlist;
                int count = loan_read_empl(current_user.uname, &lnlist);
                if (count > 0) {
                    res.type = RESSUCCESS;
                    res.data.bufcount = count;
                    bufdata = true;
                    write(cfd, &res, sizeof(Response));
                    for (int i = 0; i < count; i++) {
                        write(cfd, lnlist[i], sizeof(Loan));
                    }
                } else res.type = RESEMPTY;
                loan_free(&lnlist);
            }
        }
        if (req.type == REQLNRVPOST) {
            printf("[%d] user %s trying to review loan id (%ld)\n", args.num_requests, current_user.uname, req.data.lnrv.loan_id);
            if (current_user.role < EMPLOYEE) res.type = RESUNAUTH;
            else {
                ln_rv_result result;
                if (req.data.lnrv.status == LOAN_APPROVED) {
                    result = loan_approve(req.data.lnrv.loan_id, req.data.lnrv.acpt_amt, req.data.lnrv.rate);
                } else {
                    result = loan_reject(req.data.lnrv.loan_id, req.data.lnrv.reason);
                }
                if (result == LNRV_SUCCESS) res.type = RESSUCCESS;
                else {
                    res.type = RESBADREQ;
                    if (result == LNRV_ALREADYRVD) sprintf(res.data.msg, "Loan application already reviewed");
                    else sprintf(res.data.msg, "Loan ID not found");
                }
            }
        }
        if (req.type == REQLNCUSTGET) {
            printf("[%d] user %s trying to get applied loans\n", args.num_requests, current_user.uname);
            Loan **lnlist;
            int count = loan_read_cust(current_user.uname, &lnlist);
            if (count > 0) {
                res.type = RESSUCCESS;
                res.data.bufcount = count;
                bufdata = true;
                write(cfd, &res, sizeof(Response));
                for (int i = 0; i < count; i++) {
                    write(cfd, lnlist[i], sizeof(Loan));
                }
            } else res.type = RESEMPTY;
            loan_free(&lnlist);
        }
        if (req.type == REQADDFDBK) {
            printf("[%d] user %s trying to submite feedback\n", args.num_requests, current_user.uname);
            feedback_create(req.data.fdbk.cat, req.data.fdbk.text);
            res.type = RESSUCCESS;
        }
        if (req.type == REQGETFDBK) {
            printf("[%d] user %s trying to get feedbacks\n", args.num_requests, current_user.uname);
            Feedback **fdbklist;
            int count = feedback_read(&fdbklist);
            if (count > 0) {
                res.type = RESSUCCESS;
                res.data.bufcount = count;
                bufdata = true;
                write(cfd, &res, sizeof(Response));
                for (int i = 0; i < count; i++) {
                    write(cfd, fdbklist[i], sizeof(Feedback));
                }
            } else res.type = RESEMPTY;
            feedback_free(&fdbklist);
        }
        if (!bufdata) write(cfd, &res, sizeof(Response));
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