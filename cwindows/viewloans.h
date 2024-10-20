#ifndef VIEW_LOANS_CWINDOW
#define VIEW_LOANS_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include <time.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../utilities/removewin.h"
#include "../model/common.h"
#include "../model/request.h"
#include "../model/response.h"
#include "applyloan.h"
#include "../model/employee.h"

void vln_update_message(WINDOW *vlnwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(vlnwin);
    draw_rounded_border(vlnwin, h, w);
    mvwprintw(vlnwin, 0, w / 2 - 12, " VIEW LOAN APPLICATIONS ");
    mvwprintw(vlnwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(vlnwin);
}

const char *lnstattostr(loan_status s) {
    switch (s) {
        case LOAN_PENDING: return "Pending";
        case LOAN_APPROVED: return "Approved";
        case LOAN_REJECTED: return "Rejected :(";
    }
    return "";
}

void vln_display_loans(WINDOW *vlnwin, int sfd, Response res) {
    int n_appls = res.data.bufcount;
    Loan *loanappls = (Loan*)calloc(n_appls, sizeof(Loan));
    for (int i = 0; i < n_appls; i++) {
        int rd = read(sfd, &loanappls[i], sizeof(Loan));
        if (rd <= 0) {
            mvwprintw(vlnwin, 1, 1, "could only read %d appls", i + 1);
            n_appls = i + 1;
            break;
        }
        mvwprintw(vlnwin, 1, 1, "read appl idx %d", i);
        wrefresh(vlnwin);
    }
    int h = LINES - 4, w = COLS - 4;
    int appl_idx = 0;
    int ty = h / 2 - 1, tx = w / 2, dty = 2;
    keypad(vlnwin, TRUE);
    while (1) {
        wclear(vlnwin);
        wrefresh(vlnwin);
        draw_rounded_border(vlnwin, h, w);
        mvwprintw(vlnwin, 0, w / 2 - 12, " VIEW LOAN APPLICATIONS ");
        mvwprintw(vlnwin, 0, w - 10, "[X] CLOSE");
        char statusstr[50];
        sprintf(statusstr, "APPLICATION STATUS: %s", lnstattostr(loanappls[appl_idx].status));
        mvwprintw(vlnwin, ty - 3 * dty, tx / 5, "%s", statusstr);
        mvwprintw(vlnwin, ty - 2 * dty, tx / 5, "LOAN TYPE: %s", lntypetostr(loanappls[appl_idx].type));
        mvwprintw(vlnwin, ty - dty, tx / 5, "REQ AMOUNT: %.2f", loanappls[appl_idx].req_amount);
        mvwprintw(vlnwin, ty, tx / 5, "ANNUAL INCOME: %.2f", loanappls[appl_idx].annual_income);
        mvwprintw(vlnwin, ty + dty, tx / 5, "CREDIT SCORE: %.2f", loanappls[appl_idx].credit_score);
        mvwprintw(vlnwin, ty + 2 * dty, tx / 5, "EMPLOYMENT STATUS: %s", empstattostr(loanappls[appl_idx].emp_status));
        mvwprintw(vlnwin, ty + 3 * dty, tx / 5, "YEARS OF EMPLOYMENT: %d", loanappls[appl_idx].years_of_emp);
        if (loanappls[appl_idx].status == LOAN_APPROVED) {
            mvwprintw(vlnwin, ty - 3 * dty, tx + 5, "APPROVAL DETAILS:");
            struct tm *timeinfo = localtime(&loanappls[appl_idx].review_timestp);
            char timestr[32];
            strftime(timestr, sizeof(timestr), "%H:%M:%S %Y-%m-%d", timeinfo);
            mvwprintw(vlnwin, ty - 2 * dty, tx + 5, "REVIEW TIME: %s", timestr);
            mvwprintw(vlnwin, ty - dty, tx + 5, "AMOUNT: ₹ %.2f", loanappls[appl_idx].acpt_amount);
            mvwprintw(vlnwin, ty, tx + 5, "INTEREST RATE: %.2f %c P.A.", loanappls[appl_idx].interest_rate, '%');
        }
        WINDOW *rsnwin = NULL;
        if (loanappls[appl_idx].status == LOAN_REJECTED) {
            mvwprintw(vlnwin, ty - 3 * dty, tx + 5, "REJECTION DETAILS:");
            struct tm *timeinfo = localtime(&loanappls[appl_idx].review_timestp);
            char timestr[32];
            strftime(timestr, sizeof(timestr), "%H:%M:%S %Y-%m-%d", timeinfo);
            mvwprintw(vlnwin, ty - 2 * dty, tx + 5, "REVIEW TIME: %s", timestr);
            mvwprintw(vlnwin, ty - dty, tx + 5, "REASON:");
            wrectangle(vlnwin, ty - dty + 1, tx + 5, ty + dty + 2, tx + 37);
            wrefresh(vlnwin);
            rsnwin = newwin(2 * dty, 31, 2 + ty - dty + 2, 2 + tx + 6);
            mvwprintw(rsnwin, 0, 0, "%s", loanappls[appl_idx].rejection_reason);
        }
        mvwprintw(vlnwin, ty + 4 * dty + 1, tx / 2 - 9, "[%s] PREVIOUS APPL", ARROW_LEFT);
        mvwprintw(vlnwin, ty + 4 * dty + 1, tx - 4, "%d / %d", appl_idx + 1, n_appls);
        mvwprintw(vlnwin, ty + 4 * dty + 1, (tx * 3) / 2 - 8, "[%s] NEXT APPL", ARROW_RIGHT);
        if (rsnwin != NULL) {
            redrawwin(rsnwin);
            wrefresh(rsnwin);
        }
        int choice = wgetch(vlnwin);
        if (rsnwin != NULL) delwin(rsnwin);
        if (choice == KEY_LEFT && appl_idx > 0) { appl_idx--; }
        if (choice == KEY_RIGHT && appl_idx < n_appls - 1) { appl_idx++; }
        if (choice == 'x' || choice == 'X') break;
    }
    keypad(vlnwin, FALSE);
    free(loanappls);
}

void view_loans_window(int sfd) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *vlnwin = newwin(h, w, 2, 2);
    int page_num = 0;
    Request req = { .type = REQLNCUSTGET };
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    if (res.type == RESBADREQ || res.type == RESUNAUTH) {
        vln_update_message(vlnwin, "How did you get here?! >:(((");
        wgetch(vlnwin);
    } else if (res.type == RESEMPTY) {
        vln_update_message(vlnwin, "No loan applications yet!");
        wgetch(vlnwin);
    } else {
        vln_display_loans(vlnwin, sfd, res);
    }
    removewin(vlnwin);
}

#endif // VIEW_LOANS_CWINDOW