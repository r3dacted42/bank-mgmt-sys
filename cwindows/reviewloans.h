#ifndef REVIEW_LOANS_CWINDOW
#define REVIEW_LOANS_CWINDOW

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

void rvln_update_message(WINDOW *rvlnwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(rvlnwin);
    draw_rounded_border(rvlnwin, h, w);
    mvwprintw(rvlnwin, 0, w / 2 - 13, " REVIEW LOAN APPLICATIONS ");
    mvwprintw(rvlnwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(rvlnwin);
}

void rvln_display_loans(WINDOW *rvlnwin, int sfd, Response res) {
    int n_appls = res.data.bufcount;
    Loan *loanappls = (Loan*)calloc(n_appls, sizeof(Loan));
    bool *reviewed = (bool*)calloc(n_appls, sizeof(bool));
    for (int i = 0; i < n_appls; i++) {
        int rd = read(sfd, &loanappls[i], sizeof(Loan));
        if (rd <= 0) {
            mvwprintw(rvlnwin, 1, 1, "could only read %d appls", i + 1);
            n_appls = i + 1;
            break;
        }
        mvwprintw(rvlnwin, 1, 1, "read appl idx %d", i);
        wrefresh(rvlnwin);
    }
    for (int i = 0; i < n_appls; i++) reviewed[i] = false;
    PersonalInfo *applicants = (PersonalInfo*)calloc(n_appls, sizeof(PersonalInfo));
    Request req = { .type = REQGETUSR };
    for (int i = 0; i < n_appls; i++) {
        strcpy(req.data.getusr, loanappls[i].applicant_cust);
        write(sfd, &req, sizeof(Request));
        read(sfd, &res, sizeof(Response));
        memcpy(&applicants[i], &res.data.getusr.info, sizeof(PersonalInfo));
    }
    int h = LINES - 4, w = COLS - 4;
    int appl_idx = 0, decision_idx = 0;
    int ty = h / 2 - 1, tx = w / 2, dty = 2;
    keypad(rvlnwin, TRUE);
    while (1) {
        wclear(rvlnwin);
        draw_rounded_border(rvlnwin, h, w);
        mvwprintw(rvlnwin, 0, w / 2 - 13, " REVIEW LOAN APPLICATIONS ");
        mvwprintw(rvlnwin, 0, w - 10, "[X] CLOSE");
        char statusstr[50];
        sprintf(statusstr, "APPLICATION STATUS: %s", (reviewed[appl_idx] ? "REVIEWED" : "NOT REVIEWED"));
        mvwprintw(rvlnwin, ty - 3 * dty, tx / 5, "%s", statusstr);
        mvwprintw(rvlnwin, ty - 2 * dty, tx / 5, "LOAN TYPE: %s", lntypetostr(loanappls[appl_idx].type));
        mvwprintw(rvlnwin, ty - dty, tx / 5, "REQ AMOUNT: %.2f", loanappls[appl_idx].req_amount);
        mvwprintw(rvlnwin, ty, tx / 5, "ANNUAL INCOME: %.2f", loanappls[appl_idx].annual_income);
        mvwprintw(rvlnwin, ty + dty, tx / 5, "CREDIT SCORE: %.2f", loanappls[appl_idx].credit_score);
        mvwprintw(rvlnwin, ty + 2 * dty, tx / 5, "EMPLOYMENT STATUS: %s", empstattostr(loanappls[appl_idx].emp_status));
        mvwprintw(rvlnwin, ty + 3 * dty, tx / 5, "YEARS OF EMPLOYMENT: %d", loanappls[appl_idx].years_of_emp);
        mvwprintw(rvlnwin, ty - 3 * dty, tx, "APPLICANT DETAILS:");
        mvwprintw(rvlnwin, ty - 2 * dty, tx, "NAME: %s %s", applicants[appl_idx].first_name, applicants[appl_idx].last_name);
        mvwprintw(rvlnwin, ty - dty, tx, "GENDER: %c", applicants[appl_idx].gender);
        mvwprintw(rvlnwin, ty, tx, "DOB: %s", applicants[appl_idx].dob);
        mvwprintw(rvlnwin, ty + dty, tx, "PHONE: %s", applicants[appl_idx].phone);
        mvwprintw(rvlnwin, ty + 2 * dty, tx, "EMAIL: %s", applicants[appl_idx].email);
        int dw = 17;
        int dl = w - tx / 5 - dw + 1, dr = w - tx / 5;
        wrectangle(rvlnwin, ty - 2 * dty, dl, ty + dty, dr);
        mvwprintw(rvlnwin, ty - 2 * dty, dl + dw / 2 - 7, "REVIEW DECISION");
        if (decision_idx == 0) wattron(rvlnwin, A_REVERSE);
        mvwprintw(rvlnwin, ty - dty, dl + dw / 2 - 6, "APPROVE LOAN");
        if (decision_idx == 0) wattroff(rvlnwin, A_REVERSE);
        if (decision_idx == 1) wattron(rvlnwin, A_REVERSE);
        mvwprintw(rvlnwin, ty, dl + dw / 2 - 6, "REJECT LOAN");
        if (decision_idx == 1) wattroff(rvlnwin, A_REVERSE);
        mvwprintw(rvlnwin, ty + 4 * dty + 1, tx / 2 - 9, "[%s] PREVIOUS APPL", ARROW_LEFT);
        mvwprintw(rvlnwin, ty + 4 * dty + 1, tx - 4, "%d / %d", appl_idx + 1, n_appls);
        mvwprintw(rvlnwin, ty + 4 * dty + 1, (tx * 3) / 2 - 8, "[%s] NEXT APPL", ARROW_RIGHT);
        wrefresh(rvlnwin);
        int choice = wgetch(rvlnwin);
        if (choice == KEY_LEFT && appl_idx > 0) { appl_idx--; }
        if (choice == KEY_RIGHT && appl_idx < n_appls - 1) { appl_idx++; }
        if (choice == 'x' || choice == 'X') break;
        if (choice == KEY_DOWN && decision_idx == 0) decision_idx = 1;
        if (choice == KEY_UP && decision_idx == 1) decision_idx = 0;
        if (choice == '\n' || choice == '\r' || choice == KEY_ENTER) {
            int mh = 11, mw = 50;
            int fx = mw / 2 - 5, fw = 25;
            WINDOW *msgwin = newwin(mh, mw, h / 2 - mh / 2, w / 2 - mw / 2);
            draw_heavy_border(msgwin, mh, mw);
            if (!reviewed[appl_idx]) {
                memset(&req, 0, sizeof(Request));
                req.type = REQLNRVPOST;
                req.data.lnrv.loan_id = loanappls[appl_idx].loan_id;
                if (decision_idx == 0) {
                    req.data.lnrv.status = LOAN_APPROVED;
                    mvwprintw(msgwin, 3, fx - 17, "Accepted Amount:");
                    mvwprintw(msgwin, 7, fx - 15, "Interest Rate:");
                    wrectangle(msgwin, 2, fx, 4, fx + fw);
                    mvwprintw(msgwin, 3, fx + 1, "₹");
                    wrectangle(msgwin, 6, fx, 8, fx + fw);
                    mvwprintw(msgwin, 7, fx + fw - 6, "%c P.A.", '%');
                    mvwscanw(msgwin, 3, fx + 3, "%f", &req.data.lnrv.acpt_amt);
                    mvwscanw(msgwin, 7, fx + 1, "%f", &req.data.lnrv.rate);
                } else {
                    req.data.lnrv.status = LOAN_REJECTED;
                    int l = mw - (fx + fw);
                    int r = mw - l;
                    mvwprintw(msgwin, 2, l, "Rejection Reason:");
                    wrectangle(msgwin, 3, l, 8, r);
                    int rh =  4, rw = r - l - 1;
                    wrefresh(msgwin);
                    WINDOW *reasonwin = newwin(rh, rw, h / 2 - mh / 2 + 4, w / 2 - mw / 2 + l + 1);
                    mvwscanw(reasonwin, 0, 0, "%[^\n]", req.data.lnrv.reason);
                    removewin(reasonwin);
                }
                wclear(msgwin);
                wrefresh(msgwin);
                draw_heavy_border(msgwin, mh, mw);
                mvwprintw(msgwin, mh / 2, mw / 2 - 11, "Waiting for server...");
                write(sfd, &req, sizeof(Request));
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    mvwprintw(msgwin, mh / 2, mw / 2 - 14, "Loan reviewed successfully!");
                    reviewed[appl_idx] = true;
                } else {
                    mvwprintw(msgwin, mh / 2, mw / 2 - 11, "                     ");
                    mvwprintw(msgwin, mh / 2, mw / 2 - strlen(res.data.msg) / 2, "%s", res.data.msg);
                }
            } else {
                mvwprintw(msgwin, mh / 2, mw / 2 - 9, "Already reviewed!");
            }
            wgetch(msgwin);
            removewin(msgwin);
        }
    }
    keypad(rvlnwin, FALSE);
    free(loanappls);
    free(reviewed);
    free(applicants);
}

void review_loans_window(int sfd) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *rvlnwin = newwin(h, w, 2, 2);
    int page_num = 0;
    Request req = { .type = REQLNRVGET };
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    if (res.type == RESBADREQ || res.type == RESUNAUTH) {
        rvln_update_message(rvlnwin, "How did you get here?! >:(((");
        wgetch(rvlnwin);
    } else if (res.type == RESEMPTY) {
        rvln_update_message(rvlnwin, "No loan applications needing review!");
        wgetch(rvlnwin);
    } else {
        rvln_display_loans(rvlnwin, sfd, res);
    }
    removewin(rvlnwin);
}

#endif // REVIEW_LOANS_CWINDOW