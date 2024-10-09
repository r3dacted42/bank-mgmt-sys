#ifndef ADMIN_MENU_CWINDOW
#define ADMIN_MENU_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../model/request.h"
#include "../model/response.h"
#include "../cwindows/changepw.h"
#include "../cwindows/createuser.h"

void admin_menu_window(int sfd) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *awin = newwin(h, w, 1, 1);
    draw_heavy_border(awin, h, w);
    mvwprintw(awin, 0, w / 2 - 7, " ADMIN MENU ");
    wrefresh(awin);
    char *options[] = {"Add New User", "Modify User Details", "Change Password", "Logout"};
    int highlight_idx = 0;
    keypad(awin, TRUE);
    while (1) {
        for (int i = 0; i < 4; i++) {
            if (highlight_idx == i) wattron(awin, A_REVERSE);
            mvwprintw(awin, h / 2 - (2 - i) * 2, w / 2 - strlen(options[i]) / 2, "%s", options[i]);
            if (highlight_idx == i) wattroff(awin, A_REVERSE);
        }
        wrefresh(awin);
        int opt = wgetch(awin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') {
            // mvwprintw(awin, 1, 1, "%d", highlight_idx);
            if (highlight_idx == 3) break;
            if (highlight_idx == 2) {
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
            }
            if (highlight_idx == 0) {
                Request req = { .type = REQREGISTER };
                WINDOW *cusrwin = create_user_window(req.data.uregupdt.uname, req.data.uregupdt.pw, &req.data.uregupdt.role);
                // mvwprintw(awin, 1, 1, "%s, %s, %d", req.data.uregupdt.uname, req.data.uregupdt.pw, req.data.uregupdt.role);
                if (req.data.uregupdt.role != ADMIN) {
                    personal_info_window(cusrwin, &req.data.uregupdt.info);
                } else {
                    usw_update_message(cusrwin, "Creating user...");
                }
                mvwprintw(cusrwin, 1, 1, "sending user data...");
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    usw_update_message(cusrwin, "User created successfuly!");
                } else {
                    usw_update_message(cusrwin, "User creation failed!");
                }
                wgetch(cusrwin);
                wclear(cusrwin);
                wrefresh(cusrwin);
                delwin(cusrwin);
            }
        } 
        else if (opt == KEY_UP) highlight_idx = ((highlight_idx - 1 >= 0) ? (highlight_idx - 1) : 3);
        else if (opt == KEY_DOWN) highlight_idx = (highlight_idx + 1) % 4;
    }
    keypad(awin, FALSE);
    wclear(awin);
    wrefresh(awin);
    delwin(awin);
}

#endif // ADMIN_MENU_CWINDOW