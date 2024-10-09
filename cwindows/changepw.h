#ifndef CHANGE_PW_CWINDOW
#define CHANGE_PW_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"

#define CHANGE_PW_H 10
#define CHANGE_PW_W 42

void chpw_update_message(WINDOW *chpwin, const char *msg) {
    int h = CHANGE_PW_H, w = CHANGE_PW_W;
    wclear(chpwin);
    draw_rounded_border(chpwin, h, w);
    mvwprintw(chpwin, 0, w / 2 - 9, " CHANGE PASSWORD ");
    mvwprintw(chpwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(chpwin);
}

WINDOW* change_passwd_window(char *oldpw, char *newpw) {
    int h = CHANGE_PW_H, w = CHANGE_PW_W;
    WINDOW *chpwin = newwin(h, w, LINES / 2 - h / 2, COLS / 2 - w / 2 - 1);
    draw_rounded_border(chpwin, h, w);
    mvwprintw(chpwin, 0, w / 2 - 9, " CHANGE PASSWORD ");
    mvwprintw(chpwin, 3, 4, "Old Passwd: ");
    mvwprintw(chpwin, 6, 4, "New Passwd: ");
    wrectangle(chpwin, 2, 18, 4, w - 5);
    wrectangle(chpwin, 5, 18, 7, w - 5);
    wrefresh(chpwin);
    mvwscanw(chpwin, 3, 19, "%s", oldpw);
    mvwscanw(chpwin, 6, 19, "%s", newpw);
    chpw_update_message(chpwin, "Updating password...");
    return chpwin;
}

#endif // CHANGE_PW_CWINDOW