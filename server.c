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

#include "controller/user.h"
#include "model/request.h"
#include "model/response.h"

#define PORT 5003

void close_sock(int);
int sfd;

struct thread_args {
    int cfd;
    int num_requests;
};

void* service(void *arg) {
    struct thread_args args = *(struct thread_args*)(arg);
    printf("spawned thread for request #%d\n", args.num_requests);
    int cfd = args.cfd;
    User current_user;
    Request *req = malloc(sizeof(Request));
    Response *res;
    read(cfd, req, sizeof(req));
    if (!(req->type == REQLOGIN)) {
        res = malloc(sizeof(Response));
        res->type = RESBADREQ;
        write(cfd, res, sizeof(Response));
        free(req);
        free(res);
        free(arg);
        close(cfd);
        return NULL;
    }
    login_res lres = user_login(req->data.login.uname, req->data.login.pw, &current_user);
    if (lres == LGNSUCCESS) {
        res = malloc(sizeof(Response));
        res->type = RESLOGIN;
        strcpy(res->data.login.uname, current_user.uname);
        res->data.login.role = current_user.role;
        write(cfd, res, sizeof(Response));
        free(res);
        free(req);
        printf("[%d] user %s logged in\n", args.num_requests, current_user.uname);
    } else {
        res = malloc(sizeof(Response));
        res->type = RESUNAUTH;
        write(cfd, res, sizeof(Response));
        free(res);
        free(req);
        free(arg);
        close(cfd);
        return NULL;
    }

    while (1) {
        req = malloc(sizeof(Request));
        read(cfd, req, sizeof(Request));
        if (req->type == REQLOGOUT) break;
    }

    free(arg);
    close(cfd);
    return NULL;
}

int main() {
    if (!check_db_exists()) {
        printf("please create an admin account to continue:\nenter the username: ");
        char uname[128], pw[128];
        scanf("%s", uname);
        printf("enter the password: ");
        scanf("%s", pw);
        user_register(uname, pw, ADMIN);
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
        pthread_t th;
        pthread_create(&th, NULL, service, (void *)(args));
    }

    return 0;
}

void close_sock(int s) {
    printf("\nshutting down server...\n");
    close(sfd);
    exit(errno);
}