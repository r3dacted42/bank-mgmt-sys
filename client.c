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

#include "model/user.h"
#include "utilities/borders.h"
#include "cwindows/login.h"
#include "model/request.h"
#include "model/response.h"
#include "cwindows/adminmenu.h"

#define PORT 5003

void remove_window(WINDOW *win) {
    wclear(win);
    wrefresh(win);
    delwin(win);
}

#define debug_text(FORMAT...) mvwprintw(win, 1, 1, FORMAT)

int main() {
    struct sockaddr_in serv;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
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
    cbreak();
	WINDOW *win = newwin(LINES, COLS, 0, 0);
    draw_double_border(win, LINES, COLS);
    mvwprintw(win, 0, COLS / 2 - 13, " BANK MANAGEMENT SYSTEM ");
    wrefresh(win);
    char uname[128], passwd[128];
    WINDOW *lwin = login_window(uname, passwd);
    
    Request *req = malloc(sizeof(Request));
    req->type = REQLOGIN;
    strcpy(req->data.login.uname, uname);
    strcpy(req->data.login.pw, passwd);
    Response *res = malloc(sizeof(Response));
    bool logged_in = false;
    user_role crole;
    write(sfd, req, sizeof(Request));
    free(req);
    read(sfd, res, sizeof(Response));
    logged_in = res->type == RESSUCCESS;
    if (logged_in) {
        crole = res->data.login;
    } else {
        lw_update_message(lwin, "Login failed! Exiting...");
        wgetch(lwin);
        delwin(win);
        endwin();
        close(sfd);
        return -1;
    }
    free(res);
    remove_window(lwin);
    if (logged_in) {
        switch (crole) {
        case ADMIN:
            admin_menu_window(sfd);
            break;
        }
        req = malloc(sizeof(Request));
        req->type = REQLOGOUT;
        write(sfd, req, sizeof(Request));
        free(req);
        mvwprintw(win, LINES / 2 - 1, COLS / 2 - 37 / 2, "Logged out... Press any key to exit.");
    }
    
    wgetch(win);
    delwin(win);
	endwin();
    close(sfd);
    return 0;
}

// https://linux.die.net/man/3/mvwaddwstr