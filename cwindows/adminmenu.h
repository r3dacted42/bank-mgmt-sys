#ifndef ADMIN_MENU_CWINDOW
#define ADMIN_MENU_CWINDOW

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

void admin_menu_window(int sfd, const char *uname) {
    int h = LINES - 2, w = COLS - 2;
    WINDOW *awin = newwin(h, w, 1, 1);
    draw_heavy_border(awin, h, w);
    mvwprintw(awin, 0, w / 2 - 7, " ADMIN MENU ");
    mvwprintw(awin, h - 1, 1, " LOGGED IN AS %s (ADMIN) ", uname);
    wrefresh(awin);
    int num_options = 5;
    char *options[] = {"Add New User", "View/Modify User", "Delete User", "Change Password", "Logout"};
    int highlight_idx = 0;
    keypad(awin, TRUE);
    while (1) {
        for (int i = 0; i < num_options; i++) {
            if (highlight_idx == i) wattron(awin, A_REVERSE);
            mvwprintw(awin, h / 2 - (num_options / 2 - i) * 2, w / 2 - strlen(options[i]) / 2, "%s", options[i]);
            if (highlight_idx == i) wattroff(awin, A_REVERSE);
        }
        wrefresh(awin);
        int opt = wgetch(awin);
        if (opt == KEY_ENTER || opt == '\n' || opt == '\r') {
            // mvwprintw(awin, 1, 1, "%d", highlight_idx);
            if (highlight_idx == 4) break;
            else if (highlight_idx == 3) {
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
            } else if (highlight_idx == 0) {
                Request req = { .type = REQREGISTER };
                WINDOW *cusrwin = create_user_window(req.data.ureg.uname, req.data.ureg.pw, &req.data.ureg.role);
                // mvwprintw(awin, 1, 1, "%s, %s, %d", req.data.ureg.uname, req.data.ureg.pw, req.data.ureg.role);
                if (req.data.ureg.role != ADMIN) {
                    personal_info_window(cusrwin, &req.data.ureg.info);
                } else {
                    usw_update_message(cusrwin, "Creating user...");
                }
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    usw_update_message(cusrwin, "User created successfuly!");
                } else {
                    usw_update_message(cusrwin, "User creation failed!");
                }
                wgetch(cusrwin);
                removewin(cusrwin);
            } else if (highlight_idx == 1) {
                char uname[UN_LEN];
                Request req = { .type = REQGETUSR };
                WINDOW *eunwin = enter_uname_window(req.data.getusr);
                memcpy(uname, req.data.getusr, UN_LEN);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    eun_update_message(eunwin, "User found!");
                    wgetch(eunwin);
                    memset(&req, 0, sizeof(Request));
                    req.type = REQUPDTUSR;
                    memcpy(req.data.uupdt.uname, uname, UN_LEN);
                    memcpy(req.data.uupdt.nuname, uname, UN_LEN);
                    strcpy(req.data.uupdt.pw, "*******");
                    memcpy(&req.data.uupdt.role, &res.data.getusr.role, sizeof(get_usr_data));
                    if (view_edit_user_window(&req.data.uupdt)) {
                        write(sfd, &req, sizeof(Request));
                        read(sfd, &res, sizeof(Response));
                        if (res.type == RESSUCCESS) {
                            eun_update_message(eunwin, "User updated successfully!");
                        } else {
                            eun_update_message(eunwin, "User update failed!");
                        }
                        wgetch(eunwin);
                    }
                } else if (res.type == RESBADREQ) {
                    eun_update_message(eunwin, "Username not found!");
                    wgetch(eunwin);
                } else if (res.type == RESUNAUTH) {
                    eun_update_message(eunwin, "User is an Admin, cannot proceed.");
                    wgetch(eunwin);
                }
                removewin(eunwin);
            } else if (highlight_idx == 2) {
                Request req = { .type = REQDLTUSR };
                WINDOW *eunwin = enter_uname_window(req.data.udlt);
                write(sfd, &req, sizeof(Request));
                Response res;
                read(sfd, &res, sizeof(Response));
                if (res.type == RESSUCCESS) {
                    eun_update_message(eunwin, "User deleted!");
                } else if (res.type == RESBADREQ) {
                    eun_update_message(eunwin, "Username not found!");
                } else if (res.type == RESUNAUTH) {
                    eun_update_message(eunwin, "User is an Admin, cannot proceed.");
                }
                wgetch(eunwin);
                removewin(eunwin);
            }
        } 
        else if (opt == KEY_UP) highlight_idx = ((highlight_idx - 1 >= 0) ? (highlight_idx - 1) : num_options - 1);
        else if (opt == KEY_DOWN) highlight_idx = (highlight_idx + 1) % num_options;
    }
    keypad(awin, FALSE);
    removewin(awin);
}

#endif // ADMIN_MENU_CWINDOW