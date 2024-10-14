#ifndef REMOVE_WIN_UTIL
#define REMOVE_WIN_UTIL

#include <ncursesw/curses.h>

void removewin(WINDOW *win) {
    wclear(win);
    wrefresh(win);
    delwin(win);
}

#endif // REMOVE_WIN_UTIL