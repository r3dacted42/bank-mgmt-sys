#ifndef REVIEW_FEEDBACK_CWINDOW
#define REVIEW_FEEDBACK_CWINDOW

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
#include "addfeedback.h"
#include "../model/feedback.h"

void rfb_update_message(WINDOW *rfbwin, const char *msg) {
    int h = LINES - 4, w = COLS - 4;
    wclear(rfbwin);
    draw_rounded_border(rfbwin, h, w);
    mvwprintw(rfbwin, 0, w / 2 - 9, " REVIEW FEEDBACK ");
    mvwprintw(rfbwin, h / 2 - 1, w / 2 - strlen(msg) / 2, "%s", msg);
    wrefresh(rfbwin);
}

void rfb_display_table(WINDOW *rfbwin, int sfd, Response res) {
    int n_fdbk = res.data.bufcount;
    Feedback *fdbks = (Feedback*)calloc(n_fdbk, sizeof(Feedback));
    for (int i = 0; i < n_fdbk; i++) {
        int rd = read(sfd, &fdbks[i], sizeof(Feedback));
        if (rd <= 0) {
            mvwprintw(rfbwin, 1, 1, "could only read %d fdbks", i + 1);
            n_fdbk = i + 1;
            break;
        }
        mvwprintw(rfbwin, 1, 1, "read fdbk idx %d", i);
        wrefresh(rfbwin);
    }
    int h = LINES - 4, w = COLS - 4;
    int fdbk_idx = 0;
    int ty = h / 2 - 1, tx = w / 2, dty = 1;
    int page_idx = 0, page_sz = 7;
    int n_pages = n_fdbk / page_sz + (n_fdbk % page_sz != 0);
    keypad(rfbwin, TRUE);
    while (1) {
        wclear(rfbwin);
        draw_rounded_border(rfbwin, h, w);
        mvwprintw(rfbwin, 0, w / 2 - 9, " REVIEW FEEDBACK ");
        mvwprintw(rfbwin, 0, w - 10, "[X] CLOSE");
        int n_fields = 3;
        char *fields[] = {"DATE/TIME", "CATEGORY", "FEEDBACK"};
        int field_widths[] = {24, 16, 32};
        int txstart = 0;
        for (int i = 0; i < n_fields; i++) txstart += field_widths[i];
        txstart = tx - txstart / 2;
        int field_start[] = {txstart, 0, 0};
        for (int i = 1; i < n_fields; i++) field_start[i] = field_start[i - 1] + field_widths[i - 1];
        for (int i = 0; i < n_fields; i++) {
            mvwprintw(rfbwin, ty - dty * (page_sz / 2 + 1) - 1, field_start[i], "%s", fields[i]);
        }
        for (int i = 0; i < page_sz; i++) {
            int idx = page_idx * page_sz + i;
            if (idx > n_fdbk - 1) break;
            int y = ty - dty * (page_sz / 2 - i);
            struct tm *timeinfo = localtime(&fdbks[idx].submit_timestp);
            char timestr[24];
            strftime(timestr, sizeof(timestr), "%H:%M:%S %Y-%m-%d", timeinfo);
            char trnctxt[33] = {0};
            memcpy(trnctxt, fdbks[idx].feedback_text, field_widths[2]);
            if (strlen(fdbks[idx].feedback_text) > 31) trnctxt[29] = '.', trnctxt[30] = '.', trnctxt[31] = '.';
            mvwprintw(rfbwin, y, field_start[0], "%s", timestr);
            mvwprintw(rfbwin, y, field_start[1], "%s", fdbkcattostr(fdbks[idx].category));
            mvwprintw(rfbwin, y, field_start[2], "%s", trnctxt);
            if (idx == fdbk_idx) mvwchgat(rfbwin, y, field_start[0], field_start[2] + field_widths[2] - field_start[0], A_REVERSE, 0, NULL);
        }
        mvwprintw(rfbwin, ty + 5 * dty + 1, tx / 2 - 9, "[%s] PREVIOUS PAGE", ARROW_LEFT);
        mvwprintw(rfbwin, ty + 5 * dty + 1, tx - 4, "%d / %d", page_idx + 1, n_pages);
        mvwprintw(rfbwin, ty + 5 * dty + 1, (tx * 3) / 2 - 7, "[%s] NEXT PAGE", ARROW_RIGHT);
        wrefresh(rfbwin);
        int choice = wgetch(rfbwin);
        if (choice == KEY_LEFT && page_idx > 0) { 
            page_idx--; 
            fdbk_idx = page_idx * page_sz;
        }
        if (choice == KEY_RIGHT && page_idx < n_pages - 1) { 
            page_idx++; 
            fdbk_idx = page_idx * page_sz;
        }
        if (choice == 'x' || choice == 'X') break;
        if (choice == KEY_DOWN && fdbk_idx < n_fdbk - 1) {
            if (fdbk_idx == (page_idx + 1) * page_sz - 1) page_idx++;
            fdbk_idx++;
        }
        if (choice == KEY_UP && fdbk_idx > 0) {
            if (fdbk_idx == page_idx * page_sz) page_idx--;
            fdbk_idx--;
        }
        if (choice == '\n' || choice == '\r' || choice == KEY_ENTER) {
            int mh = h/2, mw = w / 2;
            WINDOW *msgwin = newwin(mh, mw, h / 2 - mh / 2, w / 2 - mw / 2);
            draw_heavy_border(msgwin, mh, mw);
            mvwprintw(msgwin, 0, mw / 2 - 5, " FEEDBACK ");
            struct tm *timeinfo = localtime(&fdbks[fdbk_idx].submit_timestp);
            char timestr[24];
            strftime(timestr, sizeof(timestr), "%H:%M:%S %Y-%m-%d", timeinfo);
            mvwprintw(msgwin, 2, 4, "DATE/TIME: %s", timestr);
            mvwprintw(msgwin, 4, 4, "CATEGORY:  %s", fdbkcattostr(fdbks[fdbk_idx].category));
            mvwprintw(msgwin, 6, 4, "FEEDBACK:");
            wrefresh(msgwin);
            WINDOW *textwin = newwin(mh - 3 - 6 + 1, mw - 4 - 4 - 11, h / 2 - mh / 2 + 6, w / 2 - mw / 2 + 4 + 11);
            mvwprintw(textwin, 0, 0, "%s", fdbks[fdbk_idx].feedback_text);
            wgetch(textwin);
            removewin(textwin);
            removewin(msgwin);
        }
    }
    keypad(rfbwin, FALSE);
    free(fdbks);
}

void review_feedback_window(int sfd) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *rfbwin = newwin(h, w, 2, 2);
    int page_num = 0;
    Request req = { .type = REQGETFDBK };
    write(sfd, &req, sizeof(Request));
    Response res;
    read(sfd, &res, sizeof(Response));
    if (res.type == RESBADREQ || res.type == RESUNAUTH) {
        rfb_update_message(rfbwin, "How did you get here?! >:(((");
        wgetch(rfbwin);
    } else if (res.type == RESEMPTY) {
        rfb_update_message(rfbwin, "No feedbacks available");
        wgetch(rfbwin);
    } else {
        rfb_display_table(rfbwin, sfd, res);
    }
    removewin(rfbwin);
}

#endif // REVIEW_FEEDBACK_CWINDOW