#ifndef CREATE_CUSTOMER_CWINDOW
#define CREATE_CUSTOMER_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../model/common.h"

void ccust_update_message(WINDOW *ccustwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(ccustwin);
    draw_rounded_border(ccustwin, h, w);
    mvwprintw(ccustwin, 0, w / 2 - 7, " ADD CUSTOMER ");
    mvwprintw(ccustwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(ccustwin);
}

WINDOW* create_customer_window(char *uname, char *passwd) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *ccustwin = newwin(h, w, 2, 2);
    draw_rounded_border(ccustwin, h, w);
    mvwprintw(ccustwin, 0, w / 2 - 7, " ADD CUSTOMER ");
    int fh = 2, fw = 35, fx = COLS / 2 - 10, fy = LINES / 2 - 3, dfy = 3;
    mvwprintw(ccustwin, fy - dfy + 1, fx - 11, "Username: ");
    mvwprintw(ccustwin, fy + 1, fx - 11, "Password: ");
    mvwprintw(ccustwin, fy + dfy + 1, fx - 7, "Role: ");
    wrectangle(ccustwin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(ccustwin, fy, fx, fy + fh, fx + fw);
    wrectangle(ccustwin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    mvwprintw(ccustwin, fy + dfy + 1, fx + fw / 2 - 4, "Customer");
    wrefresh(ccustwin);
    mvwscanw(ccustwin, fy - dfy + 1, fx + 1, "%s", uname);
    mvwscanw(ccustwin, fy + 1, fx + 1, "%s", passwd);
    return ccustwin;
}

void customer_info_window(WINDOW *win, PersonalInfo *pinfo) {
    wclear(win);
    wrefresh(win);
    int h = LINES - 4, w = COLS - 4;
    draw_rounded_border(win, h, w);
    mvwprintw(win, 0, w / 2 - 8, " CUSTOMER INFO ");
    int fh = 2, fw = 35, fx = COLS / 2 - 10, fy = LINES / 2 - 3, dfy = 3;
    mvwprintw(win, fy - 2 * dfy + 1, fx - 13, "First Name: ");
    mvwprintw(win, fy - dfy + 1, fx - 12, "Last Name: ");
    mvwprintw(win, fy + 1, fx - 8, "Email: ");
    mvwprintw(win, fy + dfy + 1, fx - 8, "Phone: ");
    mvwprintw(win, fy + 2 * dfy + 1, fx - 19, "DOB (YYYY/MM/DD): ");
    wrectangle(win, fy - 2 * dfy, fx, fy + fh - 2 * dfy, fx + fw);
    wrectangle(win, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(win, fy, fx, fy + fh, fx + fw);
    wrectangle(win, fy + dfy, fx, fy + fh + dfy, fx + fw);
    wrectangle(win, fy + 2 * dfy, fx, fy + fh + 2 * dfy, fx + fw);
    wrefresh(win);
    mvwscanw(win, fy - 2 * dfy + 1, fx + 1, "%s", pinfo->first_name);
    mvwscanw(win, fy - dfy + 1, fx + 1, "%s", pinfo->last_name);
    mvwscanw(win, fy + 1, fx + 1, "%s", pinfo->email);
    mvwscanw(win, fy + dfy + 1, fx + 1, "%s", pinfo->phone);
    mvwscanw(win, fy + 2 * dfy + 1, fx + 1, "%s", pinfo->dob);
    wclear(win);
    wrefresh(win);
    draw_rounded_border(win, h, w);
    mvwprintw(win, 0, w / 2 - 8, " PERSONAL INFO ");
    mvwprintw(win, fy - dfy + 1, fx - 9, "Gender: ");
    mvwprintw(win, fy + 1, fx - 7, "City: ");
    mvwprintw(win, fy + dfy + 1, fx - 11, "Zip Code: ");
    wrectangle(win, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(win, fy, fx, fy + fh, fx + fw);
    wrectangle(win, fy + dfy, fx, fy + fh + dfy, fx + fw);
    mvwprintw(win, fy - dfy + 1, fx + 1, ARROW_LEFT);
    mvwprintw(win, fy - dfy + 1, fx + fw - 1, ARROW_RIGHT);
    mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 4, "Female");
    pinfo->gender = 'F';
    keypad(win, TRUE);
    while (1) {
        int opt = wgetch(win);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') break;
        mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 5, "         ");
        if (opt == KEY_LEFT) {
            switch (pinfo->gender) {
                case 'F': mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 3, "Other"); pinfo->gender = 'O'; break;
                case 'M': mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 4, "Female"); pinfo->gender = 'F'; break;
                case 'O': mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 3, "Male"); pinfo->gender = 'M'; break;
            }
        }
        if (opt == KEY_RIGHT) {
            switch (pinfo->gender) {
                case 'M': mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 3, "Other"); pinfo->gender = 'O'; break;
                case 'O': mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 4, "Female"); pinfo->gender = 'F'; break;
                case 'F': mvwprintw(win, fy - dfy + 1, fx + fw / 2 - 3, "Male"); pinfo->gender = 'M'; break;
            }
        }
    }
    keypad(win, FALSE);
    mvwscanw(win, fy + 1, fx + 1, "%s", pinfo->city);
    mvwscanw(win, fy + dfy + 1, fx + 1, "%s", pinfo->zip_code);
    ccust_update_message(win, "Creating customer...");
}

#endif // CREATE_CUSTOMER_CWINDOW