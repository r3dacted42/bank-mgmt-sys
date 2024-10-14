#ifndef LOGIN_CWINDOW
#define LOGIN_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"

#define LOGIN_WIN_H 10
#define LOGIN_WIN_W 42

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
    mvwprintw(lwin, 0, w / 2 - 4, " LOGIN ");
    mvwprintw(lwin, 3, 6, "Username: ");
    mvwprintw(lwin, 6, 6, "Password: ");
    wrectangle(lwin, 2, 16, 4, w - 7);
    wrectangle(lwin, 5, 16, 7, w - 7);
    wrefresh(lwin);
    mvwscanw(lwin, 3, 17, "%s", uname);
    mvwscanw(lwin, 6, 17, "%s", passwd);
    lw_update_message(lwin, "Logging in...");
    return lwin;
}

#endif // LOGIN_CWINDOW