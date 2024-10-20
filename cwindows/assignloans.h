#ifndef ASSIGN_LOANS_CWINDOW
#define ASSIGN_LOANS_CWINDOW

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

void asln_update_message(WINDOW *aslnwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(aslnwin);
    draw_rounded_border(aslnwin, h, w);
    mvwprintw(aslnwin, 0, w / 2 - 13, " ASSIGN LOAN APPLICATIONS ");
    mvwprintw(aslnwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(aslnwin);
}

void asln_display_loans(WINDOW *aslnwin, int sfd, Response res) {
    int n_appls = res.data.bufcount;
    Loan *loanappls = (Loan*)calloc(n_appls, sizeof(Loan));
    bool *assigned = (bool*)calloc(n_appls, sizeof(bool));
    for (int i = 0; i < n_appls; i++) {
        int rd = read(sfd, &loanappls[i], sizeof(Loan));
        if (rd <= 0) {
            mvwprintw(aslnwin, 1, 1, "could only read %d appls", i + 1);
            n_appls = i + 1;
            break;
        }
        mvwprintw(aslnwin, 1, 1, "read appl idx %d", i);
        wrefresh(aslnwin);
    }
    for (int i = 0; i < n_appls; i++) assigned[i] = false;
    Request req = { .type = REQGETEMPS };
    write(sfd, &req, sizeof(Request));
    read(sfd, &res, sizeof(Response));
    if (res.type == RESEMPTY) {
        // no employees available to assign, clean-up and display message
        free(loanappls);
        free(assigned);
        asln_update_message(aslnwin, "No employees to assign!");
        wgetch(aslnwin);
        return;
    }
    int n_emps = res.data.bufcount;
    Employee *allemps = (Employee*)calloc(n_emps, sizeof(Employee));
    for (int i = 0; i < n_emps; i++) read(sfd, &allemps[i], sizeof(Employee));
    int h = LINES - 4, w = COLS - 4;
    int appl_idx = 0, emp_idx = 0;
    int ty = h / 2 - 1, tx = w / 2, dty = 2;
    int emp_page_idx = 0, emp_page_sz = 6 * dty - 3;
    keypad(aslnwin, TRUE);
    while (1) {
        wclear(aslnwin);
        draw_rounded_border(aslnwin, h, w);
        mvwprintw(aslnwin, 0, w / 2 - 13, " ASSIGN LOAN APPLICATIONS ");
        mvwprintw(aslnwin, 0, w - 10, "[X] CLOSE");
        char statusstr[50];
        sprintf(statusstr, "APPLICATION STATUS: %s", (assigned[appl_idx] ? "ASSIGNED" : "NOT ASSIGNED"));
        mvwprintw(aslnwin, ty - 3 * dty, tx / 5, "%s", statusstr);
        mvwprintw(aslnwin, ty - 2 * dty, tx / 5, "LOAN TYPE: %s", lntypetostr(loanappls[appl_idx].type));
        mvwprintw(aslnwin, ty - dty, tx / 5, "REQ AMOUNT: %.2f", loanappls[appl_idx].req_amount);
        mvwprintw(aslnwin, ty, tx / 5, "ANNUAL INCOME: %.2f", loanappls[appl_idx].annual_income);
        mvwprintw(aslnwin, ty + dty, tx / 5, "CREDIT SCORE: %.2f", loanappls[appl_idx].credit_score);
        mvwprintw(aslnwin, ty + 2 * dty, tx / 5, "EMPLOYMENT STATUS: %s", empstattostr(loanappls[appl_idx].emp_status));
        mvwprintw(aslnwin, ty + 3 * dty, tx / 5, "YEARS OF EMPLOYMENT: %d", loanappls[appl_idx].years_of_emp);
        mvwprintw(aslnwin, ty - 3 * dty, tx + 5, "SELECT EMPLOYEE:");
        wrectangle(aslnwin, ty - 3 * dty + 1, tx + 5, ty + 3 * dty, w - tx / 5);
        for (int i = 0; i < emp_page_sz; i++) {
            int eidx = emp_page_idx * emp_page_sz + i;
            if (eidx >= n_emps) break;
            if (eidx == emp_idx) wattron(aslnwin, A_REVERSE);
            mvwprintw(aslnwin, ty - 3 * dty + 2 + i, tx + 6, "%s %s", 
                allemps[eidx].pers_info.first_name, allemps[eidx].pers_info.last_name);
            if (eidx == emp_idx) wattroff(aslnwin, A_REVERSE);
        }
        mvwprintw(aslnwin, ty + 4 * dty + 1, tx / 2 - 9, "[%s] PREVIOUS APPL", ARROW_LEFT);
        mvwprintw(aslnwin, ty + 4 * dty + 1, tx - 4, "%d / %d", appl_idx + 1, n_appls);
        mvwprintw(aslnwin, ty + 4 * dty + 1, (tx * 3) / 2 - 8, "[%s] NEXT APPL", ARROW_RIGHT);
        wrefresh(aslnwin);
        int choice = wgetch(aslnwin);
        if (choice == KEY_LEFT && appl_idx > 0) { appl_idx--; }
        if (choice == KEY_RIGHT && appl_idx < n_appls - 1) { appl_idx++; }
        if (choice == 'x' || choice == 'X') break;
        if (choice == KEY_DOWN && emp_idx < n_emps - 1) {
            if (emp_idx == (emp_page_idx + 1) * emp_page_sz - 1) emp_page_idx++;
            emp_idx++;
        }
        if (choice == KEY_UP && emp_idx > 0) {
            if (emp_idx == emp_page_idx * emp_page_sz) emp_page_idx--;
            emp_idx--;
        }
        if (choice == '\n' || choice == '\r' || choice == KEY_ENTER) {
            int mh = 7, mw = 64;
            WINDOW *msgwin = newwin(mh, mw, h / 2 - mh / 2, w / 2 - mw / 2);
            draw_thick_border(msgwin, mh, mw);
            if (!assigned[appl_idx]) {
                mvwprintw(msgwin, mh / 2, mw / 2 - 11, "Waiting for server...");
                memset(&req, 0, sizeof(Request));
                req.type = REQLNASSGNPOST;
                req.data.lnassgn.loan_id = loanappls[appl_idx].loan_id;
                strcpy(req.data.lnassgn.eun, allemps[emp_idx].uname);
                write(sfd, &req, sizeof(Request));
                read(sfd, &res, sizeof(Response));
                wclear(msgwin);
                wrefresh(msgwin);
                draw_thick_border(msgwin, mh, mw);
                if (res.type == RESSUCCESS) {
                    mvwprintw(msgwin, mh / 2, mw / 2 - 10, "Assignment success!");
                    assigned[appl_idx] = true;
                } else if (res.type == RESBADREQ)
                    mvwprintw(msgwin, mh / 2, mw / 2 - strlen(res.data.msg) / 2, "%s", res.data.msg);
            } else {
                mvwprintw(msgwin, mh / 2, mw / 2 - 9, "Already assigned!");
            }
            wgetch(msgwin);
            removewin(msgwin);
        }
    }
    keypad(aslnwin, FALSE);
    free(loanappls);
    free(assigned);
    free(allemps);
}

void assign_loans_window(int sfd) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *aslnwin = newwin(h, w, 2, 2);
    int page_num = 0;
    Request req = { .type = REQLNASSGNGET };
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    if (res.type == RESBADREQ || res.type == RESUNAUTH) {
        asln_update_message(aslnwin, "How did you get here?! >:(((");
        wgetch(aslnwin);
    } else if (res.type == RESEMPTY) {
        asln_update_message(aslnwin, "No loan applications needing assignment!");
        wgetch(aslnwin);
    } else {
        asln_display_loans(aslnwin, sfd, res);
    }
    removewin(aslnwin);
}

#endif // ASSIGN_LOANS_CWINDOW