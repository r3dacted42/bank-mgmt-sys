#ifndef ENTER_UNAME_CWINDOW
#define ENTER_UNAME_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"

#define ENTER_UNAME_H 7
#define ENTER_UNAME_W 55

void eun_update_message(WINDOW *eunwin, const char *msg) {
    int h = ENTER_UNAME_H, w = ENTER_UNAME_W;
    wclear(eunwin);
    draw_rounded_border(eunwin, h, w);
    mvwprintw(eunwin, 0, w / 2 - 8, " ENTER USERNAME ");
    mvwprintw(eunwin, h / 2, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(eunwin);
}

WINDOW* enter_uname_window(char *uname) {
    int h = ENTER_UNAME_H, w = ENTER_UNAME_W;
    WINDOW *eunwin = newwin(h, w, LINES / 2 - h / 2 - 1, COLS / 2 - w / 2 - 1);
    draw_rounded_border(eunwin, h, w);
    mvwprintw(eunwin, 0, w / 2 - 8, " ENTER USERNAME ");
    mvwprintw(eunwin, h / 2, 4, "Username: ");
    wrectangle(eunwin, h / 2 - 1, 18, h / 2 + 1, w - 5);
    wrefresh(eunwin);
    mvwscanw(eunwin, h / 2, 19, "%s", uname);
    eun_update_message(eunwin, "Searching username...");
    return eunwin;
}

#endif // ENTER_UNAME_CWINDOW