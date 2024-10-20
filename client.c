#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <wchar.h>
#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <stdbool.h>
#include <signal.h>

#include "model/user.h"
#include "utilities/borders.h"
#include "cwindows/login.h"
#include "model/request.h"
#include "model/response.h"
#include "cwindows/adminmenu.h"
#include "cwindows/managermenu.h"
#include "cwindows/employeemenu.h"
#include "cwindows/customermenu.h"

#define PORT 5003

#define debug_text(FORMAT...) mvwprintw(win, 1, 1, FORMAT)

WINDOW *win;
int sfd;

void graceful_exit(int sig) {
    Request req = { .type = REQLOGOUT };
    write(sfd, &req, sizeof(Request));
    delwin(win);
	endwin();
    shutdown(sfd, SHUT_RDWR);
    close(sfd);
    exit(0);
}

int main() {
    struct sockaddr_in serv;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("failed while opening socket");
        return -1;
    }
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr) <= 0) {
        perror("failed at inet_pton");
        exit(EXIT_FAILURE);
    }
    if (connect(sfd, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("invalid connection ");
        return -1;
    }
    
    setlocale(LC_ALL, "");
    initscr();
	win = newwin(LINES, COLS, 0, 0);
    signal(SIGINT, graceful_exit);
    draw_double_border(win, LINES, COLS);
    mvwprintw(win, 0, COLS / 2 - 13, " BANK MANAGEMENT SYSTEM ");
    wrefresh(win);
    char uname[UN_LEN], passwd[PW_LEN];
    WINDOW *lwin = login_window(uname, passwd);
    
    Request req = { .type = REQLOGIN };
    strcpy(req.data.login.uname, uname);
    strcpy(req.data.login.pw, passwd);
    bool logged_in = false;
    user_role crole;
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    logged_in = res.type == RESSUCCESS;
    if (logged_in) {
        crole = res.data.login;
    } else {
        char msg[128];
        sprintf(msg, "%s Exiting...", res.data.msg);
        lw_update_message(lwin, msg);
        wgetch(lwin);
        removewin(lwin);
        delwin(win);
        endwin();
        close(sfd);
        return -1;
    }
    removewin(lwin);
    if (logged_in) {
        switch (crole) {
            case ADMIN: admin_menu_window(sfd, uname); break;
            case MANAGER: manager_menu_window(sfd, uname); break;
            case EMPLOYEE: employee_menu_window(sfd, uname); break;
            case CUSTOMER: customer_menu_window(sfd, uname); break;
        }
        req.type = REQLOGOUT;
        write(sfd, &req, sizeof(Request));
        mvwprintw(win, LINES / 2 - 1, COLS / 2 - 37 / 2, "Logged out... Press any key to exit.");
    }
    wgetch(win);
    delwin(win);
	endwin();
    shutdown(sfd, SHUT_RDWR);
    close(sfd);
    return 0;
}