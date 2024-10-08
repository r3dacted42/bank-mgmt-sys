#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <wchar.h>
#include <locale.h>

#include "model/user.h"

#define PORT 5003

int mvwaddwstr(WINDOW *win, int y, int x, const wchar_t *wstr);
void draw_border(WINDOW *win, int wy, int wx) {
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, 0, L"║");
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, wx-1, L"║");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, 0, i, L"═");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, wy - 1, i, L"═");
    mvwaddwstr(win, 0, 0, L"╔");
    mvwaddwstr(win, 0, wx - 1, L"╗");
    mvwaddwstr(win, wy - 1, 0, L"╚");
    mvwaddwstr(win, wy - 1, wx - 1, L"╝");
}

int main() {
    // struct sockaddr_in serv;
    // int sfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sfd < 0) {
    //     perror("failed while opening socket");
    //     return -1;
    // }
    // serv.sin_family = AF_INET;
    // serv.sin_port = htons(PORT);
    // if (inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr) <= 0) {
    //     perror("failed at inet_pton");
    //     exit(EXIT_FAILURE);
    // }
    // if (connect(sfd, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
    //     perror("invalid connection ");
    //     return -1;
    // }
    
    setlocale(LC_ALL, "");
    initscr();
	WINDOW *win = newwin(LINES, COLS, 0, 0);
    draw_border(win, LINES, COLS);
    // box(win, 0, 0);
    mvwprintw(win, 0, COLS / 2 - 13, " BANK MANAGEMENT SYSTEM ");
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        attron(COLOR_PAIR(1));
        mvwprintw(win, 1, 1, "has colors, just hates you");
        attroff(COLOR_PAIR(1));
    }
    wrefresh(win);
    wgetch(win);
    delwin(win);
	endwin();

    // close(sfd);
    return 0;
}

// https://linux.die.net/man/3/mvwaddwstr