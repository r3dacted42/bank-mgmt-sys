#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ncurses.h>

#include "model/user.h"

#define PORT 5003

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
    
    initscr();
    start_color();
    attron(COLOR_PAIR(1));
	WINDOW *win = newwin(LINES, COLS, 0, 0);
    // wborder(win, 186, 186, 205, 205, 201, 187, 200, 188);
    box(win, 0, 0);
    init_pair(1, COLOR_RED, COLOR_WHITE);
    mvwprintw(win, 0, COLS / 2 - 13, " BANK MANAGEMENT SYSTEM ");
    wrefresh(win);
    int ch = wgetch(win);
    attroff(COLOR_PAIR(1));
    delwin(win);
	endwin();

    // close(sfd);
    return 0;
}