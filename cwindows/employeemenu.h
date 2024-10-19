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
#include "../cwindows/createcustomer.h"
#include "../cwindows/enteruname.h"
#include "../cwindows/editcustomer.h"
#include "../cwindows/transactionhist.h"
#include "../cwindows/reviewloans.h"

void employee_menu_window(int sfd, const char *uname) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *ewin = newwin(h, w, 1, 1);
    draw_heavy_border(ewin, h, w);
    mvwprintw(ewin, 0, w / 2 - 8, " EMPLOYEE MENU ");
    mvwprintw(ewin, h - 1, 1, " LOGGED IN AS %s (EMPLOYEE) ", uname);
    wrefresh(ewin);
    int num_options = 6;
    char *options[] = {"Add New Customer", "View / Modify Customer [BROKEN]", "Review Loan Applications", "View Customer Transactions", "Change Password", "Logout"};
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
                Request req = { .type = REQREGISTER };
                WINDOW *ccustwin = create_customer_window(req.data.ureg.uname, req.data.ureg.pw);
                customer_info_window(ccustwin, &req.data.ureg.info);
                req.data.ureg.role = CUSTOMER;
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    usw_update_message(ccustwin, "Customer created successfuly!");
                } else {
                    usw_update_message(ccustwin, "Customer creation failed!");
                    mvwprintw(ccustwin, 1, 1, "%s", (res.type == RESBADREQ ? "BAD REQUEST" : "UNAUTHORIZED"));
                }
                wgetch(ccustwin);
                removewin(ccustwin);
            } else if (highlight_idx == 1) {
                // DISABLED UNTIL FIX
                continue;
                
                char uname[UN_LEN];
                Request req = { .type = REQGETUSR };
                WINDOW *eunwin = enter_uname_window(req.data.getusr);
                memcpy(uname, req.data.getusr, UN_LEN);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS && res.data.getusr.role == CUSTOMER) {
                    eun_update_message(eunwin, "Customer found!");
                    wgetch(eunwin);
                    memset(&req, 0, sizeof(Request));
                    req.type = REQUPDTUSR;
                    strcpy(req.data.uupdt.uname, uname);
                    strcpy(req.data.uupdt.nuname, uname);
                    memcpy(&req.data.uupdt.role, &res.data.getusr.role, sizeof(get_usr_data));
                    if (edit_customer_window(&req.data.uupdt)) {
                        write(sfd, &req, sizeof(Request));
                        read(sfd, &res, sizeof(Response));
                        if (res.type == RESSUCCESS) {
                            eun_update_message(eunwin, "Customer updated successfully!");
                        } else {
                            eun_update_message(eunwin, "Customer update failed!");
                            mvwprintw(eunwin, 1, 1, "%s", (res.type == RESBADREQ ? "BAD REQUEST" : "UNAUTHORIZED"));
                        }
                        wgetch(eunwin);
                    }
                } else if (res.type == RESBADREQ) {
                    eun_update_message(eunwin, "Username not found!");
                    wgetch(eunwin);
                } else if (res.type == RESUNAUTH) {
                    eun_update_message(eunwin, "User is not a customer, cannot proceed.");
                    wgetch(eunwin);
                }
                removewin(eunwin);
            } else if (highlight_idx == 2) {
                review_loans_window(sfd);
            } else if (highlight_idx == 3) {
                char uname[UN_LEN];
                Request req = { .type = REQGETUSR };
                WINDOW *eunwin = enter_uname_window(req.data.getusr);
                memcpy(uname, req.data.getusr, UN_LEN);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS && res.data.getusr.role == CUSTOMER) {
                    eun_update_message(eunwin, "Customer found!");
                    wgetch(eunwin);
                    transaction_history_window(sfd, uname);
                } else if (res.type == RESBADREQ) {
                    eun_update_message(eunwin, "Username not found!");
                    wgetch(eunwin);
                } else if (res.type == RESUNAUTH) {
                    eun_update_message(eunwin, "User is not a customer, cannot proceed.");
                    wgetch(eunwin);
                }
                removewin(eunwin);
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