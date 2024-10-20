#ifndef LOGIN_CWINDOW
#define LOGIN_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"

#define LOGIN_WIN_H 16
#define LOGIN_WIN_W 60

void lw_update_message(WINDOW *lwin, const char *msg) {
    int h = LOGIN_WIN_H, w = LOGIN_WIN_W;
    wclear(lwin);
    draw_rounded_border(lwin, h, w);
    mvwprintw(lwin, 0, w / 2 - 4, " LOGIN ");
    mvwprintw(lwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(lwin);
}

WINDOW* login_window(char *uname, char *passwd) {
    int h = LOGIN_WIN_H, w = LOGIN_WIN_W;
    WINDOW *lwin = newwin(h, w, LINES / 2 - h / 2, COLS / 2 - w / 2 - 1);
    draw_rounded_border(lwin, h, w);
    int yu = h / 2 - 2, yp = h / 2 + 2;
    mvwprintw(lwin, 0, w / 2 - 4, " LOGIN ");
    mvwprintw(lwin, yu, 6, "Username: ");
    mvwprintw(lwin, yp, 6, "Password: ");
    wrectangle(lwin, yu - 1, 16, yu + 1, w - 7);
    wrectangle(lwin, yp - 1, 16, yp + 1, w - 7);
    wrefresh(lwin);
    mvwscanw(lwin, yu, 17, "%s", uname);
    mvwscanw(lwin, yp, 17, "%s", passwd);
    lw_update_message(lwin, "Logging in...");
    return lwin;
}

#endif // LOGIN_CWINDOW