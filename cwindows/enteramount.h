#ifndef ENTER_AMOUNT_CWINDOW
#define ENTER_AMOUNT_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"

#define ENTER_AMOUNT_H 7
#define ENTER_AMOUNT_W 42

enum eamt_type {
    EAMT_DEPOSIT,
    EAMT_WITHDRAW,
    EAMT_TRANSFER
};

void eamt_update_message(WINDOW *eamt, const char *msg) {
    int h = ENTER_AMOUNT_H, w = ENTER_AMOUNT_W;
    wclear(eamt);
    draw_rounded_border(eamt, h, w);
    mvwprintw(eamt, 0, w / 2 - 8, " ENTER AMOUNT ");
    mvwprintw(eamt, h / 2, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(eamt);
}

WINDOW* enter_amount_window(float *amt, enum eamt_type type) {
    int h = ENTER_AMOUNT_H, w = ENTER_AMOUNT_W;
    WINDOW *eamt = newwin(h, w, LINES / 2 - h / 2 - 1, COLS / 2 - w / 2 - 1);
    draw_rounded_border(eamt, h, w);
    mvwprintw(eamt, 0, w / 2 - 8, " ENTER AMOUNT ");
    mvwprintw(eamt, h / 2, 4, "Amount: ");
    wrectangle(eamt, h / 2 - 1, 18, h / 2 + 1, w - 5);
    mvwprintw(eamt, h / 2, 19, "₹");
    wrefresh(eamt);
    mvwscanw(eamt, h / 2, 21, "%f", amt);
    char msg[32];
    if (type == EAMT_DEPOSIT) sprintf(msg, "Depositing...");
    if (type == EAMT_WITHDRAW) sprintf(msg, "Withdrawing...");
    if (type == EAMT_TRANSFER) sprintf(msg, "Transferring...");
    eamt_update_message(eamt, msg);
    return eamt;
}

#endif // ENTER_AMOUNT_CWINDOW