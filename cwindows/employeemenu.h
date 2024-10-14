#ifndef EMPLOYEE_MENU_CWINDOW
#define EMPLOYEE_MENU_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../utilities/removewin.h"
#include "../model/request.h"
#include "../model/response.h"
#include "../cwindows/changepw.h"
#include "../cwindows/createuser.h"
#include "../cwindows/enteruname.h"
#include "../cwindows/viewedituser.h"

void employee_menu_window(int sfd, const char *uname) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *ewin = newwin(h, w, 1, 1);
    draw_heavy_border(ewin, h, w);
    mvwprintw(ewin, 0, w / 2 - 8, " EMPLOYEE MENU ");
    mvwprintw(ewin, h - 1, 1, " LOGGED IN AS %s (EMPLOYEE) ", uname);
    wrefresh(ewin);
    int num_options = 6;
    char *options[] = {"Add New Customer", "Modify Customer", "Review Loan Applications", "View Customer Transactions", "Change Password", "Logout"};
    int highlight_idx = 0;
    keypad(ewin, TRUE);
    while (1) {
        for (int i = 0; i < num_options; i++) {
            if (highlight_idx == i) wattron(ewin, A_REVERSE);
            mvwprintw(ewin, h / 2 - (num_options / 2 - i) * 2, w / 2 - strlen(options[i]) / 2, "%s", options[i]);
            if (highlight_idx == i) wattroff(ewin, A_REVERSE);
        }
        wrefresh(ewin);
        int opt = wgetch(ewin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') {
            // mvwprintw(ewin, 1, 1, "%d", highlight_idx);
            if (highlight_idx == 0) {
                // add cust
            } else if (highlight_idx == 1) {
                // modify cust
            } else if (highlight_idx == 2) {
                // review loans
            } else if (highlight_idx == 3) {
                // view cust transactions
            } else if (highlight_idx == 4) {
                Request req = { .type = REQCHPW };
                WINDOW *chpwin = change_passwd_window(req.data.chpw.oldpw, req.data.chpw.newpw);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    chpw_update_message(chpwin, "Password changed successfuly!");
                } else {
                    chpw_update_message(chpwin, "Password change failed!");
                }
                wgetch(chpwin);
                removewin(chpwin);
            } else if (highlight_idx == 5) break; // LOGOUT
        } 
        else if (opt == KEY_UP) highlight_idx = ((highlight_idx - 1 >= 0) ? (highlight_idx - 1) : num_options - 1);
        else if (opt == KEY_DOWN) highlight_idx = (highlight_idx + 1) % num_options;
    }
    keypad(ewin, FALSE);
    removewin(ewin);
}

#endif // EMPLOYEE_MENU_CWINDOW