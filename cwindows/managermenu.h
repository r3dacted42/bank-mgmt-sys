#ifndef MANAGER_MENU_CWINDOW
#define MANAGER_MENU_CWINDOW

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
#include "../cwindows/assignloans.h"
#include "../cwindows/reviewfeedback.h"

void manager_menu_window(int sfd, const char *uname) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *mwin = newwin(h, w, 1, 1);
    draw_heavy_border(mwin, h, w);
    mvwprintw(mwin, 0, w / 2 - 7, " MANAGER MENU ");
    mvwprintw(mwin, h - 1, 1, " LOGGED IN AS %s (MANAGER) ", uname);
    wrefresh(mwin);
    int num_options = 5;
    char *options[] = {"Activate / Deactivate Customer Accts", "Assign Loan Applications", "Review Customer Feedback", "Change Password", "Logout"};
    int highlight_idx = 0;
    keypad(mwin, TRUE);
    while (1) {
        for (int i = 0; i < num_options; i++) {
            if (highlight_idx == i) wattron(mwin, A_REVERSE);
            mvwprintw(mwin, h / 2 - (num_options / 2 - i) * 2, w / 2 - strlen(options[i]) / 2, "%s", options[i]);
            if (highlight_idx == i) wattroff(mwin, A_REVERSE);
        }
        wrefresh(mwin);
        int opt = wgetch(mwin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') {
            if (highlight_idx == 0) {
                Request req = { .type = REQGETUSR };
                WINDOW *eunwin = enter_uname_window(req.data.getusr);
                write(sfd, &req, sizeof(Request));
                Response res; 
                read(sfd, &res, sizeof(Response));
                if (res.type != RESSUCCESS) {
                    eun_update_message(eunwin, "User not found!");
                    wgetch(eunwin);
                } else if (res.data.getusr.role != CUSTOMER) {
                    eun_update_message(eunwin, "User is not a customer!");
                    wgetch(eunwin);
                } else {
                    if (res.data.getusr.cust_state == CACC_INACTIVE) {
                        eun_update_message(eunwin, "Customer account is inactive, activate it? [y/n]");
                    } else {
                        eun_update_message(eunwin, "Customer account is active, deactivate it? [y/n]");
                    }
                    int opt = wgetch(eunwin);
                    if (opt == 'y' || opt == 'Y') {
                        eun_update_message(eunwin, "Updating customer account state...");
                        char cuname[UN_LEN];
                        strcpy(cuname, req.data.getusr);
                        memset(&req, 0, sizeof(Request));
                        req.type = REQUPDTUSR;
                        strcpy(req.data.uupdt.uname, cuname);
                        strcpy(req.data.uupdt.nuname, cuname);
                        memcpy(&req.data.uupdt.role, &res.data.getusr.role, 
                        sizeof(user_role) + sizeof(PersonalInfo) + sizeof(acc_state) + sizeof(long));
                        if (req.data.uupdt.cust_state == CACC_ACTIVE) {
                            req.data.uupdt.cust_state = CACC_INACTIVE;
                        } else {
                            req.data.uupdt.cust_state = CACC_ACTIVE;
                        }
                        write(sfd, &req, sizeof(Request));
                        read(sfd, &res, sizeof(Response));
                        if (res.type == RESSUCCESS) {
                            eun_update_message(eunwin, "Successfully updated account state!");
                        } else {
                            eun_update_message(eunwin, "Failed to update account state!");
                        }
                        wgetch(eunwin);
                    }
                }
                removewin(eunwin);
                echo();
            } else if (highlight_idx == 1) {
                assign_loans_window(sfd);
            } else if (highlight_idx == 2) {
                review_feedback_window(sfd);
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
                removewin(chpwin);
            } else if (highlight_idx == 4) break; // LOGOUT
        } 
        else if (opt == KEY_UP) highlight_idx = ((highlight_idx - 1 >= 0) ? (highlight_idx - 1) : num_options - 1);
        else if (opt == KEY_DOWN) highlight_idx = (highlight_idx + 1) % num_options;
    }
    keypad(mwin, FALSE);
    removewin(mwin);
}

#endif // MANAGER_MENU_CWINDOW