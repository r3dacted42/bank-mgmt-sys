#ifndef VIEW_EDIT_USER_CWINDOW
#define VIEW_EDIT_USER_CWINDOW

#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>
#include <string.h>
#include "../utilities/borders.h"
#include "../utilities/shapes.h"
#include "../model/common.h"
#include "../model/request.h"

#define hilt(win, idx) if (active_idx == idx) wattron(win, A_REVERSE)
#define unlt(win, idx) if (active_idx == idx) wattroff(win, A_REVERSE)

void roletostr(user_role role, char *str) {
    memset(str, 0, 10);
    switch (role) {
        case ADMIN: strcpy(str, "Admin"); break;
        case MANAGER: strcpy(str, "Manager"); break;
        case EMPLOYEE: strcpy(str, "Employee"); break;
        case CUSTOMER: strcpy(str, "Customer"); break;
    }
}

int gendertoi(char g) {
    switch (g) {
        case 'F': return 0;
        case 'M': return 1;
        case 'O': return 2;
    }
}

void itogtxt(int i, char *str) {
    memset(str, 0, 10);
    switch (i) {
        case 0: strcpy(str, "Female"); break;
        case 1: strcpy(str, "Male"); break;
        case 2: strcpy(str, "Other"); break;
    }
}

char itogender(int i) {
    switch (i) {
        case 0: return 'F';
        case 1: return 'M';
        case 2: return 'O';
    }
}

bool view_edit_user_window(req_uupdt_data *udata) {
    int h = LINES - 4, w = COLS - 4;
    WINDOW *uvewin = newwin(h, w, 2, 2);
    draw_rounded_border(uvewin, h, w);
    mvwprintw(uvewin, 0, w / 2 - 11, " VIEW / MODIFY USER ");
    int fh = 2, fw = 35, fx = COLS / 2 - 10, fy = LINES / 2 - 5, dfy = 3;
    mvwprintw(uvewin, fy - dfy + 1, fx - 11, "Username: ");
    mvwprintw(uvewin, fy + 1, fx - 11, "Password: ");
    mvwprintw(uvewin, fy + dfy + 1, fx - 7, "Role: ");
    wrectangle(uvewin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(uvewin, fy, fx, fy + fh, fx + fw);
    wrectangle(uvewin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    short active_idx = 0;
    char rolestr[10], eraser[fw - 1];
    memset(eraser, ' ', fw - 2);
    eraser[fw - 2] = '\0';
    keypad(uvewin, TRUE);
    bool pw_modified = false;
    while (1) {
        hilt(uvewin, 0); mvwprintw(uvewin, fy - dfy + 1, fx + 1, "%s", udata->nuname); unlt(uvewin, 0);
        hilt(uvewin, 1); mvwprintw(uvewin, fy + 1, fx + 1, "%s", udata->pw); unlt(uvewin, 1);
        roletostr(udata->role, rolestr);
        mvwprintw(uvewin, fy + dfy + 1, fx + 1, "%s", eraser);
        mvwprintw(uvewin, fy + dfy + 1, fx + 1, ARROW_LEFT);
        mvwprintw(uvewin, fy + dfy + 1, fx + fw - 1, ARROW_RIGHT);
        hilt(uvewin, 2); mvwprintw(uvewin, fy + dfy + 1, fx + fw / 2 - strlen(rolestr) / 2, "%s", rolestr); unlt(uvewin, 2);
        hilt(uvewin, 3); mvwprintw(uvewin, fy + 2 * dfy + 1, COLS * 1 / 3 - 11 / 2, "[X] CANCEL"); unlt(uvewin, 3);
        hilt(uvewin, 4); mvwprintw(uvewin, fy + 2 * dfy + 1, COLS * 2 / 3 - 9 / 2, "[N] NEXT"); unlt(uvewin, 4);
        wrefresh(uvewin);
        int opt = wgetch(uvewin);
        if (opt == 'x' || opt == 'X') {
            wclear(uvewin);
            wrefresh(uvewin);
            delwin(uvewin);
            return false;
        }
        if (opt == 'N' || opt == 'n') break;
        if (opt == KEY_ENTER || opt == '\r' || opt == '\n') {
            if (active_idx == 0) {
                mvwprintw(uvewin, fy - dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy - dfy + 1, fx + 1, "%s", udata->nuname);
            }
            if (active_idx == 1) {
                pw_modified = true;
                mvwprintw(uvewin, fy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy + 1, fx + 1, "%s", udata->pw);
            }
            if (active_idx == 3) {
                wclear(uvewin);
                wrefresh(uvewin);
                delwin(uvewin);
                return false;
            }
            if (active_idx == 4) break;
        }
        if (opt == KEY_DOWN && active_idx <= 2) active_idx++;
        if (opt == KEY_UP && active_idx > 0) active_idx = (active_idx == 4 ? 2 : active_idx - 1);
        if (opt == KEY_LEFT) {
            if (active_idx == 4) active_idx--;
            else if (active_idx == 2) 
                udata->role = (user_role)((udata->role > 0) ? udata->role - 1 : CUSTOMER);
        }
        if (opt == KEY_RIGHT) {
            if (active_idx == 3) active_idx++;
            else if (active_idx == 2)
                udata->role = (user_role)(udata->role < CUSTOMER ? udata->role + 1 : ADMIN);
        }
    }
    wclear(uvewin);
    wrefresh(uvewin);
    draw_rounded_border(uvewin, h, w);
    mvwprintw(uvewin, 0, w / 2 - 11, " VIEW / MODIFY USER ");
    mvwprintw(uvewin, fy - 2 * dfy + 1, fx - 13, "First Name: ");
    mvwprintw(uvewin, fy - dfy + 1, fx - 12, "Last Name: ");
    mvwprintw(uvewin, fy + 1, fx - 8, "Email: ");
    mvwprintw(uvewin, fy + dfy + 1, fx - 8, "Phone: ");
    mvwprintw(uvewin, fy + 2 * dfy + 1, fx - 19, "DOB (YYYY/MM/DD): ");
    wrectangle(uvewin, fy - 2 * dfy, fx, fy + fh - 2 * dfy, fx + fw);
    wrectangle(uvewin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(uvewin, fy, fx, fy + fh, fx + fw);
    wrectangle(uvewin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    wrectangle(uvewin, fy + 2 * dfy, fx, fy + fh + 2 * dfy, fx + fw);
    active_idx = 0;
    while (1) {
        hilt(uvewin, 0); mvwprintw(uvewin, fy - 2 * dfy + 1, fx + 1, "%s", udata->info.first_name); unlt(uvewin, 0);
        hilt(uvewin, 1); mvwprintw(uvewin, fy - dfy + 1, fx + 1, "%s", udata->info.last_name); unlt(uvewin, 1);
        hilt(uvewin, 2); mvwprintw(uvewin, fy + 1, fx + 1, "%s", udata->info.email); unlt(uvewin, 2);
        hilt(uvewin, 3); mvwprintw(uvewin, fy + dfy + 1, fx + 1, "%s", udata->info.phone); unlt(uvewin, 3);
        hilt(uvewin, 4); mvwprintw(uvewin, fy + 2 * dfy + 1, fx + 1, "%s", udata->info.dob); unlt(uvewin, 4);
        hilt(uvewin, 5); mvwprintw(uvewin, fy + 3 * dfy + 1, COLS * 1 / 3 - 11 / 2, "[X] CANCEL"); unlt(uvewin, 5);
        hilt(uvewin, 6); mvwprintw(uvewin, fy + 3 * dfy + 1, COLS * 2 / 3 - 9 / 2, "[N] NEXT"); unlt(uvewin, 6);
        wrefresh(uvewin);
        int opt = wgetch(uvewin);
        if (opt == 'x' || opt == 'X') {
            wclear(uvewin);
            wrefresh(uvewin);
            delwin(uvewin);
            return false;
        }
        if (opt == 'N' || opt == 'n') break;
        if (opt == KEY_ENTER || opt == '\r' || opt == '\n') {
            if (active_idx == 0) {
                mvwprintw(uvewin, fy - 2 * dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy - 2 * dfy + 1, fx + 1, "%s", udata->info.first_name);
            }
            if (active_idx == 1) {
                mvwprintw(uvewin, fy - dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy - dfy + 1, fx + 1, "%s", udata->info.last_name);
            }
            if (active_idx == 2) {
                mvwprintw(uvewin, fy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy + 1, fx + 1, "%s", udata->info.email);
            }
            if (active_idx == 3) {
                mvwprintw(uvewin, fy + dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy + dfy + 1, fx + 1, "%s", udata->info.phone);
            }
            if (active_idx == 4) {
                mvwprintw(uvewin, fy + 2 * dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy + 2 * dfy + 1, fx + 1, "%s", udata->info.dob);
            }
            if (active_idx == 5) {
                wclear(uvewin);
                wrefresh(uvewin);
                delwin(uvewin);
                return false;
            }
            if (active_idx == 6) break;
        }
        if (opt == KEY_DOWN && active_idx < 5) active_idx++;
        if (opt == KEY_UP && active_idx > 0) active_idx = (active_idx == 6 ? 4 : active_idx - 1);
        if (opt == KEY_LEFT && active_idx == 6) active_idx--;
        if (opt == KEY_RIGHT && active_idx == 5) active_idx++;
    }
    wclear(uvewin);
    wrefresh(uvewin);
    draw_rounded_border(uvewin, h, w);
    mvwprintw(uvewin, 0, w / 2 - 11, " VIEW / MODIFY USER ");
    mvwprintw(uvewin, fy - dfy + 1, fx - 9, "Gender: ");
    mvwprintw(uvewin, fy + 1, fx - 7, "City: ");
    mvwprintw(uvewin, fy + dfy + 1, fx - 11, "Zip Code: ");
    wrectangle(uvewin, fy - dfy, fx, fy + fh - dfy, fx + fw);
    wrectangle(uvewin, fy, fx, fy + fh, fx + fw);
    wrectangle(uvewin, fy + dfy, fx, fy + fh + dfy, fx + fw);
    active_idx = 0;
    int gidx = gendertoi(udata->info.gender);
    char genderstr[7];
    while (1) {
        itogtxt(gidx, genderstr);
        mvwprintw(uvewin, fy - dfy + 1, fx + 1, "%s", eraser);
        mvwprintw(uvewin, fy - dfy + 1, fx + 1, ARROW_LEFT);
        mvwprintw(uvewin, fy - dfy + 1, fx + fw - 1, ARROW_RIGHT);
        hilt(uvewin, 0); mvwprintw(uvewin, fy - dfy + 1, fx + fw / 2 - strlen(genderstr) / 2, "%s", genderstr); unlt(uvewin, 0);
        hilt(uvewin, 1); mvwprintw(uvewin, fy + 1, fx + 1, "%s", udata->info.city); unlt(uvewin, 1);
        hilt(uvewin, 2); mvwprintw(uvewin, fy + dfy + 1, fx + 1, "%s", udata->info.zip_code); unlt(uvewin, 2);
        hilt(uvewin, 3); mvwprintw(uvewin, fy + 2 * dfy + 1, COLS * 1 / 3 - 11 / 2, "[X] CANCEL"); unlt(uvewin, 3);
        hilt(uvewin, 4); mvwprintw(uvewin, fy + 2 * dfy + 1, COLS * 2 / 3 - 9 / 2, "[S] SAVE"); unlt(uvewin, 4);
        wrefresh(uvewin);
        int opt = wgetch(uvewin);
        if (opt == 'x' || opt == 'X') {
            wclear(uvewin);
            wrefresh(uvewin);
            delwin(uvewin);
            return false;
        }
        if (opt == 's' || opt == 'S') break;
        if (opt == KEY_ENTER || opt == '\r' || opt == '\n') {
            if (active_idx == 1) {
                mvwprintw(uvewin, fy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy + 1, fx + 1, "%s", udata->info.city);
            }
            if (active_idx == 2) {
                mvwprintw(uvewin, fy + dfy + 1, fx + 1, "%s", eraser);
                mvwscanw(uvewin, fy + dfy + 1, fx + 1, "%s", udata->info.zip_code);
            }
            if (active_idx == 3) {
                wclear(uvewin);
                wrefresh(uvewin);
                delwin(uvewin);
                return false;
            }
            if (active_idx == 4) break;
        }
        if (opt == KEY_DOWN && active_idx < 3) active_idx++;
        if (opt == KEY_UP && active_idx > 0) active_idx = (active_idx == 4 ? 2 : active_idx - 1);
        if (opt == KEY_LEFT) {
            if (active_idx == 4) active_idx--;
            else if (active_idx == 0) {
                gidx = (gidx > 0 ? gidx - 1 : 2);
                udata->info.gender = itogender(gidx);
            }
        }
        if (opt == KEY_RIGHT) {
            if (active_idx == 3) active_idx++;
            else if (active_idx == 0) {
                gidx = (gidx + 1) % 3;
                udata->info.gender = itogender(gidx);
            }
        }
    }

    wclear(uvewin);
    wrefresh(uvewin);
    delwin(uvewin);
    if (!pw_modified) memset(udata->pw, 0, PW_LEN);
    return true;
}

#endif // VIEW_EDIT_USER_CWINDOW