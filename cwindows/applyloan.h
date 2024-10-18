#ifndef APPLY_LOAN_CWINDOW
#define APPLY_LOAN_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../model/common.h"
#include "../controller/loan.h"

const char *lntypetostr(loan_type t) {
    switch (t) {
        case LOAN_PERSONAL: return "Personal";
        case LOAN_HOME: return "Home";
        case LOAN_VEHICLE: return "Vehicle";
        case LOAN_EDUCATION: return "Education";
        case LOAN_MORTGAGE: return "Mortgage";
        case LOAN_GOLD: return "Gold";
    }
    return "";
}

const char *empstattostr(employment_status s) {
    switch (s) {
        case UNEMPLOYED: return "Unemployed";
        case SELF_EMPLOYED: return "Self Employed";
        case EMPLOYED: return "Employed";
    }
    return "";
}

void aln_update_message(WINDOW *alnwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(alnwin);
    draw_rounded_border(alnwin, h, w);
    mvwprintw(alnwin, 0, w / 2 - 8, " APPLY FOR LOAN ");
    mvwprintw(alnwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(alnwin);
}

WINDOW* apply_loan_window(Loan *lndata) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *alnwin = newwin(h, w, 2, 2);
    draw_rounded_border(alnwin, h, w);
    mvwprintw(alnwin, 0, w / 2 - 8, " APPLY FOR LOAN ");
    int fh = 2, fw = 35, fx = COLS / 2 - 5, fy = LINES / 2 - 3, dfy = 3;
    mvwprintw(alnwin, fy - dfy + 1, fx - 12, "Loan Type: ");
    mvwprintw(alnwin, fy + 1, fx - 19, "Requested Amount: ");
    mvwprintw(alnwin, fy + dfy + 1, fx - 16, "Annual Income: ");
    wrectangle(alnwin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    mvwprintw(alnwin, fy + 1, fx + 1, "₹");
    wrectangle(alnwin, fy, fx, fy + fh, fx + fw);
    mvwprintw(alnwin, fy + dfy + 1, fx + 1, "₹");
    wrectangle(alnwin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    lndata->type = LOAN_PERSONAL;
    mvwprintw(alnwin, fy - dfy + 1, fx + 1, ARROW_LEFT);
    mvwprintw(alnwin, fy - dfy + 1, fx + fw - 1, ARROW_RIGHT);
    wrefresh(alnwin);
    keypad(alnwin, TRUE);
    while (1) {
        const char *lnstr = lntypetostr(lndata->type);
        mvwprintw(alnwin, fy - dfy + 1, fx + fw / 2 - strlen(lnstr) / 2, "%s", lnstr);
        int opt = wgetch(alnwin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') break;
        mvwprintw(alnwin, fy - dfy + 1, fx + fw / 2 - 5, "          ");
        if (opt == KEY_LEFT) lndata->type = (loan_type)(lndata->type > 0 ? lndata->type - 1 : LOAN_TYPE_MAX);
        if (opt == KEY_RIGHT) lndata->type = (loan_type)(lndata->type < LOAN_TYPE_MAX ? lndata->type + 1 : 0);
    }
    keypad(alnwin, FALSE);
    mvwscanw(alnwin, fy + 1, fx + 3, "%f", &lndata->req_amount);
    mvwscanw(alnwin, fy + dfy + 1, fx + 3, "%f", &lndata->annual_income);
    wclear(alnwin);
    wrefresh(alnwin);
    draw_rounded_border(alnwin, h, w);
    mvwprintw(alnwin, 0, w / 2 - 8, " APPLY FOR LOAN ");
    mvwprintw(alnwin, fy - dfy + 1, fx - 15, "Credit Score: ");
    mvwprintw(alnwin, fy + 1, fx - 20, "Employment Status: ");
    mvwprintw(alnwin, fy + dfy + 1, fx - 22, "Years of Employment: ");
    wrectangle(alnwin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(alnwin, fy, fx, fy + fh, fx + fw);
    wrectangle(alnwin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    mvwprintw(alnwin, fy + 1, fx + 1, ARROW_LEFT);
    mvwprintw(alnwin, fy + 1, fx + fw - 1, ARROW_RIGHT);
    mvwprintw(alnwin, fy + 1, fx + fw / 2 - 5, "Unemployed");
    wrefresh(alnwin);
    mvwscanw(alnwin, fy - dfy + 1, fx + 1, "%f", &lndata->credit_score);
    lndata->emp_status = UNEMPLOYED;
    keypad(alnwin, TRUE);
    while (1) {
        const char *estr = empstattostr(lndata->emp_status);
        mvwprintw(alnwin, fy + 1, fx + fw / 2 - strlen(estr) / 2, "%s", estr);
        int opt = wgetch(alnwin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') break;
        mvwprintw(alnwin, fy + 1, fx + fw / 2 - 8, "               ");
        if (opt == KEY_LEFT) lndata->emp_status = (employment_status)(lndata->emp_status > 0 ? lndata->emp_status - 1 : EMP_STAT_MAX);
        if (opt == KEY_RIGHT) lndata->emp_status = (employment_status)(lndata->emp_status < EMP_STAT_MAX ? lndata->emp_status + 1 : 0);
    }
    keypad(alnwin, FALSE);
    mvwscanw(alnwin, fy + dfy + 1, fx + 1, "%d", &lndata->years_of_emp);
    aln_update_message(alnwin, "Submitting application...");
    return alnwin;
}

#endif // APPLY_LOAN_CWINDOW