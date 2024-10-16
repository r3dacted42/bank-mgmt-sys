#ifndef VIEW_EDIT_CUSTOMER_CWINDOW
#define VIEW_EDIT_CUSTOMER_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../utilities/removewin.h"
#include "../model/common.h"
#include "../model/request.h"
#include "viewedituser.h"

bool edit_customer_window(req_uupdt_data *udata) {
    memset(udata->pw, 0, PW_LEN);
    int h = LINES - 4, w = COLS - 4;
    WINDOW *ecustwin = newwin(h, w, 2, 2);
    draw_rounded_border(ecustwin, h, w);
    mvwprintw(ecustwin, 0, w / 2 - 12, " VIEW / MODIFY CUSTOMER ");
    int fh = 2, fw = 35, fx = COLS / 2 - 10, fy = LINES / 2 - 5, dfy = 3;
    mvwprintw(ecustwin, fy - 2 * dfy + 1, fx - 13, "First Name: ");
    mvwprintw(ecustwin, fy - dfy + 1, fx - 12, "Last Name: ");
    mvwprintw(ecustwin, fy + 1, fx - 8, "Email: ");
    mvwprintw(ecustwin, fy + dfy + 1, fx - 8, "Phone: ");
    mvwprintw(ecustwin, fy + 2 * dfy + 1, fx - 19, "DOB (YYYY/MM/DD): ");
    wrectangle(ecustwin, fy - 2 * dfy, fx, fy + fh - 2 * dfy, fx + fw);
    wrectangle(ecustwin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(ecustwin, fy, fx, fy + fh, fx + fw);
    wrectangle(ecustwin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    wrectangle(ecustwin, fy + 2 * dfy, fx, fy + fh + 2 * dfy, fx + fw);
    short active_idx = 0;
    char eraser[fw - 1];
    memset(eraser, ' ', fw - 2);
    eraser[fw - 2] = '\0';
    keypad(ecustwin, TRUE);
    while (1) {
        hilt(ecustwin, 0); mvwprintw(ecustwin, fy - 2 * dfy + 1, fx + 1, "%s", udata->info.first_name); unlt(ecustwin, 0);
        hilt(ecustwin, 1); mvwprintw(ecustwin, fy - dfy + 1, fx + 1, "%s", udata->info.last_name); unlt(ecustwin, 1);
        hilt(ecustwin, 2); mvwprintw(ecustwin, fy + 1, fx + 1, "%s", udata->info.email); unlt(ecustwin, 2);
        hilt(ecustwin, 3); mvwprintw(ecustwin, fy + dfy + 1, fx + 1, "%s", udata->info.phone); unlt(ecustwin, 3);
        hilt(ecustwin, 4); mvwprintw(ecustwin, fy + 2 * dfy + 1, fx + 1, "%s", udata->info.dob); unlt(ecustwin, 4);
        hilt(ecustwin, 5); mvwprintw(ecustwin, fy + 3 * dfy + 1, COLS * 1 / 3 - 11 / 2, "[X] CANCEL"); unlt(ecustwin, 5);
        hilt(ecustwin, 6); mvwprintw(ecustwin, fy + 3 * dfy + 1, COLS * 2 / 3 - 9 / 2, "[N] NEXT"); unlt(ecustwin, 6);
        wrefresh(ecustwin);
        int opt = wgetch(ecustwin);
        if (opt == 'x' || opt == 'X') {
            removewin(ecustwin);
            return false;
        }
        if (opt == 'N' || opt == 'n') break;
        if (opt == KEY_ENTER || opt == '\r' || opt == '\n') {
            if (active_idx == 0) {
                mvwprintw(ecustwin, fy - 2 * dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy - 2 * dfy + 1, fx + 1, "%s", udata->info.first_name);
            }
            if (active_idx == 1) {
                mvwprintw(ecustwin, fy - dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy - dfy + 1, fx + 1, "%s", udata->info.last_name);
            }
            if (active_idx == 2) {
                mvwprintw(ecustwin, fy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy + 1, fx + 1, "%s", udata->info.email);
            }
            if (active_idx == 3) {
                mvwprintw(ecustwin, fy + dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy + dfy + 1, fx + 1, "%s", udata->info.phone);
            }
            if (active_idx == 4) {
                mvwprintw(ecustwin, fy + 2 * dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy + 2 * dfy + 1, fx + 1, "%s", udata->info.dob);
            }
            if (active_idx == 5) {
                removewin(ecustwin);
                return false;
            }
            if (active_idx == 6) break;
        }
        if (opt == KEY_DOWN && active_idx < 5) active_idx++;
        if (opt == KEY_UP && active_idx > 0) active_idx = (active_idx == 6 ? 4 : active_idx - 1);
        if (opt == KEY_LEFT && active_idx == 6) active_idx--;
        if (opt == KEY_RIGHT && active_idx == 5) active_idx++;
    }
    wclear(ecustwin);
    wrefresh(ecustwin);
    draw_rounded_border(ecustwin, h, w);
    mvwprintw(ecustwin, 0, w / 2 - 12, " VIEW / MODIFY CUSTOMER ");
    mvwprintw(ecustwin, fy - dfy + 1, fx - 9, "Gender: ");
    mvwprintw(ecustwin, fy + 1, fx - 7, "City: ");
    mvwprintw(ecustwin, fy + dfy + 1, fx - 11, "Zip Code: ");
    wrectangle(ecustwin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(ecustwin, fy, fx, fy + fh, fx + fw);
    wrectangle(ecustwin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    active_idx = 0;
    int gidx = gendertoi(udata->info.gender);
    char genderstr[7];
    while (1) {
        itogtxt(gidx, genderstr);
        mvwprintw(ecustwin, fy - dfy + 1, fx + 1, "%s", eraser);
        mvwprintw(ecustwin, fy - dfy + 1, fx + 1, ARROW_LEFT);
        mvwprintw(ecustwin, fy - dfy + 1, fx + fw - 1, ARROW_RIGHT);
        hilt(ecustwin, 0); mvwprintw(ecustwin, fy - dfy + 1, fx + fw / 2 - strlen(genderstr) / 2, "%s", genderstr); unlt(ecustwin, 0);
        hilt(ecustwin, 1); mvwprintw(ecustwin, fy + 1, fx + 1, "%s", udata->info.city); unlt(ecustwin, 1);
        hilt(ecustwin, 2); mvwprintw(ecustwin, fy + dfy + 1, fx + 1, "%s", udata->info.zip_code); unlt(ecustwin, 2);
        hilt(ecustwin, 3); mvwprintw(ecustwin, fy + 2 * dfy + 1, COLS * 1 / 3 - 11 / 2, "[X] CANCEL"); unlt(ecustwin, 3);
        hilt(ecustwin, 4); mvwprintw(ecustwin, fy + 2 * dfy + 1, COLS * 2 / 3 - 9 / 2, "[S] SAVE"); unlt(ecustwin, 4);
        wrefresh(ecustwin);
        int opt = wgetch(ecustwin);
        if (opt == 'x' || opt == 'X') {
            mvwprintw(ecustwin, 1, 1, "closing");
            wgetch(ecustwin);
            removewin(ecustwin);
            return false;
        }
        if (opt == 's' || opt == 'S') break;
        if (opt == KEY_ENTER || opt == '\r' || opt == '\n') {
            if (active_idx == 1) {
                mvwprintw(ecustwin, fy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy + 1, fx + 1, "%s", udata->info.city);
            }
            if (active_idx == 2) {
                mvwprintw(ecustwin, fy + dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(ecustwin, fy + dfy + 1, fx + 1, "%s", udata->info.zip_code);
            }
            if (active_idx == 3) {
                removewin(ecustwin);
                return false;
            }
            if (active_idx == 4) break;
        }
        if (opt == KEY_DOWN && active_idx < 3) active_idx++;
        if (opt == KEY_UP && active_idx > 0) active_idx = (active_idx == 4 ? 2 : active_idx - 1);
        if (opt == KEY_LEFT) {
            if (active_idx == 4) active_idx--;
            else if (active_idx == 0) {
                gidx = (gidx > 0 ? gidx - 1 : 2);
                udata->info.gender = itogender(gidx);
            }
        }
        if (opt == KEY_RIGHT) {
            if (active_idx == 3) active_idx++;
            else if (active_idx == 0) {
                gidx = (gidx + 1) % 3;
                udata->info.gender = itogender(gidx);
            }
        }
    }
    removewin(ecustwin);
    return true;
}

#endif // VIEW_EDIT_CUSTOMER_CWINDOW