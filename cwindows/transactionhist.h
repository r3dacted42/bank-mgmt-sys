#ifndef TRANSACTION_HISTORY_CWINDOW
#define TRANSACTION_HISTORY_CWINDOW

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

void trh_update_message(WINDOW *trhwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(trhwin);
    draw_rounded_border(trhwin, h, w);
    mvwprintw(trhwin, 0, w / 2 - 11, " TRANSACTION HISTORY ");
    mvwprintw(trhwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(trhwin);
}

void trh_draw_table(WINDOW *trhwin, int sfd, const char *un, Response res) {
    int h = LINES - 4, w = COLS - 4;
    int page_num = 0;
    keypad(trhwin, TRUE);
    while (1) {
        wclear(trhwin);
        draw_rounded_border(trhwin, h, w);
        mvwprintw(trhwin, 0, w / 2 - 11, " TRANSACTION HISTORY ");
        mvwprintw(trhwin, 0, w - 10, "[X] CLOSE");
        int ty = h / 2 - 1, tx = w / 2, dty = 2;
        int num_fields = 5;
        char *table_headers[] = {"DATE/TIME", "OPERATION", "USER", "AMOUNT", "TYPE"};
        int field_width[] = {24, 16, 16, 16, 6};
        int txstart = 0;
        for (int i = 0; i < num_fields; i++) txstart += field_width[i];
        txstart = tx - txstart / 2;
        int field_start[] = {txstart, 0, 0, 0, 0};
        for (int i = 1; i < num_fields; i++) field_start[i] = field_width[i - 1] + field_start[i - 1];
        for (int i = 0; i < num_fields; i++)
            mvwprintw(trhwin, ty - 3 * dty, field_start[i], "%s", table_headers[i]);
        for (int i = 0; i < res.data.viewtran.page_len; i++) {
            int field_idx = 0;
            struct tm *timeinfo = localtime(&res.data.viewtran.lst[i].timestp);
            char timestr[32];
            strftime(timestr, sizeof(timestr), "%H:%M:%S %Y-%m-%d", timeinfo);
            mvwprintw(trhwin, ty - (2 - i) * dty, field_start[field_idx++], "%s", timestr);
            char opstr[20] = {};
            if (res.data.viewtran.lst[i].op == DEPOSIT) sprintf(opstr, "DEPOSIT");
            else if (res.data.viewtran.lst[i].op == WITHDRAW) sprintf(opstr, "WITHDRAW");
            else if (res.data.viewtran.lst[i].op == TRANSFER) sprintf(opstr, "TRANSFER");
            else if (res.data.viewtran.lst[i].op == LOAN) sprintf(opstr, "LOAN");
            mvwprintw(trhwin, ty - (2 - i) * dty, field_start[field_idx++], "%s", opstr);
            mvwprintw(trhwin, ty - (2 - i) * dty, field_start[field_idx++], "%s", res.data.viewtran.lst[i].other_username);
            mvwprintw(trhwin, ty - (2 - i) * dty, field_start[field_idx++], "₹ %.2f", res.data.viewtran.lst[i].amount);
            mvwprintw(trhwin, ty - (2 - i) * dty, field_start[field_idx++], "%s", (res.data.viewtran.lst[i].type == CREDIT ? "CREDIT" : "DEBIT"));
        }
        mvwprintw(trhwin, ty + 4 * dty, tx / 2 - 9, "[%s] PREVIOUS PAGE", ARROW_LEFT);
        mvwprintw(trhwin, ty + 4 * dty, tx - 4, "%d / %d", page_num + 1, res.data.viewtran.total_pages);
        mvwprintw(trhwin, ty + 4 * dty, (tx * 3) / 2 - 8, "[%s] NEXT PAGE", ARROW_RIGHT);
        wrefresh(trhwin);
        int choice = wgetch(trhwin);
        bool refresh_page = false;
        if (choice == KEY_LEFT && page_num > 0) { refresh_page = true; page_num--; }
        if (choice == KEY_RIGHT && page_num < res.data.viewtran.total_pages - 1) { refresh_page = true; page_num++; }
        if (choice == 'x' || choice == 'X' || choice == '\n' || choice == '\r') break;
        if (refresh_page) {
            Request req = { .type = REQVIEWTRAN };
            req.data.viewtran.page_num = page_num;
            strcpy(req.data.viewtran.un, un);
            write(sfd, &req, sizeof(Request));
            read(sfd, &res, sizeof(Response));
        }
    }
    keypad(trhwin, FALSE);
}

WINDOW* transaction_history_window(int sfd, const char *uname) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *trhwin = newwin(h, w, 2, 2);
    int page_num = 0;
    Request req = { .type = REQVIEWTRAN };
    req.data.viewtran.page_num = page_num;
    strcpy(req.data.viewtran.un, uname);
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    if (res.type == RESBADREQ) {
        trh_update_message(trhwin, "Bad request!!! >:(((");
        wgetch(trhwin);
    } else if (res.type == RESEMPTY) {
        trh_update_message(trhwin, "No records found");
        wgetch(trhwin);
    } else {
        trh_draw_table(trhwin, sfd, uname, res);
    }
    removewin(trhwin);
}

#endif // TRANSACTION_HISTORY_CWINDOW