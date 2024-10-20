#ifndef ADD_FEEDBACK_CWINDOW
#define ADD_FEEDBACK_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../utilities/removewin.h"
#include "../model/feedback.h"

void afdbk_update_message(WINDOW *afdbkwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(afdbkwin);
    draw_rounded_border(afdbkwin, h, w);
    mvwprintw(afdbkwin, 0, w / 2 - 7, " ADD FEEDBACK ");
    mvwprintw(afdbkwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(afdbkwin);
}

const char *fdbkcattostr(feedback_category c) {
    switch (c) {
        case FDBK_GENERAL: return "General";
        case FDBK_TRANSFERS: return "Transfers";
        case FDBK_EMPLOYEES: return "Employees";
        case FDBK_LOANS: return "Loans";
    }
    return "";
}

WINDOW* add_feedback_window(feedback_category *cat, char *text) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *afdbkwin = newwin(h, w, 2, 2);
    draw_rounded_border(afdbkwin, h, w);
    mvwprintw(afdbkwin, 0, w / 2 - 7, " ADD FEEDBACK ");
    int fh = 2, fw = 35, fx = COLS / 2 - 10, fy = LINES / 2 - 3, dfy = 3;
    mvwprintw(afdbkwin, fy - dfy + 1, fx - 11, "Category: ");
    mvwprintw(afdbkwin, fy + 1, fx - 6, "Text:");
    wrectangle(afdbkwin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(afdbkwin, fy, fx, fy + 2 * dfy, fx + fw);
    mvwprintw(afdbkwin, fy - dfy + 1, fx + 1, ARROW_LEFT);
    mvwprintw(afdbkwin, fy - dfy + 1, fx + fw - 1, ARROW_RIGHT);
    mvwprintw(afdbkwin, fy - dfy + 1, fx + fw / 2 - 3, "General");
    wrefresh(afdbkwin);
    keypad(afdbkwin, TRUE);
    int catidx = 0;
    while (1) {
        int opt = wgetch(afdbkwin);
        if (opt == KEY_LEFT) catidx = (catidx > 0 ? (catidx - 1) : FDBK_CATEGORY_MAX);
        if (opt == KEY_RIGHT) catidx = (catidx < FDBK_CATEGORY_MAX ? (catidx + 1) : 0);
        *cat = (feedback_category)catidx;
        mvwprintw(afdbkwin, fy - dfy + 1, fx + fw / 2 - 7, "             ");
        const char *fdbkstr = fdbkcattostr(*cat);
        mvwprintw(afdbkwin, fy - dfy + 1, fx + fw / 2 - strlen(fdbkstr) / 2, "%s", fdbkstr);
        if (opt == '\n' || opt == '\r' || opt == KEY_ENTER) break;
    }
    keypad(afdbkwin, FALSE);
    wrefresh(afdbkwin);
    int th = 2 * dfy - 1, tw = fw - 1;
    WINDOW *txtwin = newwin(th, tw, 2 + fy + 1, 2 + fx + 1);
    mvwscanw(txtwin, 0, 0, "%[^\n]", text);
    removewin(txtwin);
    return afdbkwin;
}

#endif // ADD_FEEDBACK_CWINDOW