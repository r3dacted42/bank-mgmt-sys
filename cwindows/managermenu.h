#ifndef MANAGER_MENU_CWINDOW
#define MANAGER_MENU_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../model/request.h"
#include "../model/response.h"
#include "../cwindows/changepw.h"
#include "../cwindows/createuser.h"
#include "../cwindows/enteruname.h"
#include "../cwindows/viewedituser.h"

void manager_menu_window(int sfd) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *cwin = newwin(h, w, 1, 1);
    draw_heavy_border(cwin, h, w);
    mvwprintw(cwin, 0, w / 2 - 7, " MANAGER MENU ");
    wrefresh(cwin);
    int num_options = 5;
    char *options[] = {"Activate / Deactivate Customer Accts", "Assign Loan Applications", "Review Customer Feedback", "Change Password", "Logout"};
    int highlight_idx = 0;
    keypad(cwin, TRUE);
    while (1) {
        for (int i = 0; i < num_options; i++) {
            if (highlight_idx == i) wattron(cwin, A_REVERSE);
            mvwprintw(cwin, h / 2 - (num_options / 2 - i) * 2, w / 2 - strlen(options[i]) / 2, "%s", options[i]);
            if (highlight_idx == i) wattroff(cwin, A_REVERSE);
        }
        wrefresh(cwin);
        int opt = wgetch(cwin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') {
            // mvwprintw(cwin, 1, 1, "%d", highlight_idx);
            if (highlight_idx == 0) {
                // act/deact cust
            } else if (highlight_idx == 1) {
                // assign loans
            } else if (highlight_idx == 2) {
                // review feedback
            } else if (highlight_idx == 3) {
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
                wclear(chpwin);
                wrefresh(chpwin);
                delwin(chpwin);
            } else if (highlight_idx == 4) break; // LOGOUT
        } 
        else if (opt == KEY_UP) highlight_idx = ((highlight_idx - 1 >= 0) ? (highlight_idx - 1) : num_options - 1);
        else if (opt == KEY_DOWN) highlight_idx = (highlight_idx + 1) % num_options;
    }
    keypad(cwin, FALSE);
    wclear(cwin);
    wrefresh(cwin);
    delwin(cwin);
}

#endif // MANAGER_MENU_CWINDOW