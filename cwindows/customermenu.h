#ifndef CUSTOMER_MENU_CWINDOW
#define CUSTOMER_MENU_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../model/request.h"
#include "../model/response.h"
#include "../cwindows/changepw.h"
#include "../cwindows/enteramount.h"

void view_balance(int sfd) {
    Request req = { .type = REQGETBAL };
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    int h = 5, w = 32;
    WINDOW *balwin = newwin(h, w, LINES / 2 - h / 2, COLS / 2 - w / 2);
    draw_rounded_border(balwin, h, w);
    mvwprintw(balwin, 0, w / 2 - 5, " BALANCE ");
    char balstr[w - 4];
    sprintf(balstr, "₹ %.2f", res.data.getbal);
    mvwprintw(balwin, h / 2, w / 2 - strlen(balstr) / 2, "%s", balstr);
    wgetch(balwin);
    wclear(balwin);
    wrefresh(balwin);
    delwin(balwin);
}

void customer_menu_window(int sfd) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *cwin = newwin(h, w, 1, 1);
    draw_heavy_border(cwin, h, w);
    mvwprintw(cwin, 0, w / 2 - 8, " CUSTOMER MENU ");
    wrefresh(cwin);
    int num_options = 10;
    char *options[] = {"View Account Balance", "Deposit Money", "Withdraw Money", "Transfer Funds", "Apply for Loan", 
        "View Loan Applications", "View Transaction History", "Add Feedback", "Change Password", "Logout"};
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
                view_balance(sfd);
            } else if (highlight_idx == 1) {
                Request req = { .type = REQDEPOSIT };
                WINDOW *eamtwin = enter_amount_window(&req.data.baldelta, true);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) eamt_update_message(eamtwin, "Deposit successful!");
                else eamt_update_message(eamtwin, "Deposit failed!");
                wgetch(eamtwin);
                wclear(eamtwin);
                wrefresh(eamtwin);
                delwin(eamtwin);
            } else if (highlight_idx == 2) {
                Request req = { .type = REQWITHDRAW };
                WINDOW *eamtwin = enter_amount_window(&req.data.baldelta, false);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) eamt_update_message(eamtwin, "Withdraw successful!");
                else eamt_update_message(eamtwin, "Withdraw failed!");
                wgetch(eamtwin);
                wclear(eamtwin);
                wrefresh(eamtwin);
                delwin(eamtwin);
            } else if (highlight_idx == 3) {
                // transfer money
            } else if (highlight_idx == 4) {
                // apply for loan
            } else if (highlight_idx == 5) {
                // view applications
            } else if (highlight_idx == 6) {
                // transaction history
            } else if (highlight_idx == 7) {
                // feedback
            } else if (highlight_idx == 8) {
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
            } else if (highlight_idx == 9) break; // LOGOUT
        } 
        else if (opt == KEY_UP) highlight_idx = ((highlight_idx - 1 >= 0) ? (highlight_idx - 1) : num_options - 1);
        else if (opt == KEY_DOWN) highlight_idx = (highlight_idx + 1) % num_options;
    }
    keypad(cwin, FALSE);
    wclear(cwin);
    wrefresh(cwin);
    delwin(cwin);
}

#endif // CUSTOMER_MENU_CWINDOW