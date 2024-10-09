#ifndef BORDERS_UTIL
#define BORDERS_UTIL

#include <wchar.h>
#include <ncursesw/curses.h>
#include <ncursesw/ncurses.h>

int mvwaddwstr(WINDOW *win, int y, int x, const wchar_t *wstr);

void draw_double_border(WINDOW *win, int wy, int wx) {
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, 0, L"║");
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, wx-1, L"║");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, 0, i, L"═");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, wy - 1, i, L"═");
    mvwaddwstr(win, 0, 0, L"╔");
    mvwaddwstr(win, 0, wx - 1, L"╗");
    mvwaddwstr(win, wy - 1, 0, L"╚");
    mvwaddwstr(win, wy - 1, wx - 1, L"╝");
}

void draw_rounded_border(WINDOW *win, int wy, int wx) {
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, 0, L"│");
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, wx-1, L"│");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, 0, i, L"─");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, wy - 1, i, L"─");
    mvwaddwstr(win, 0, 0, L"╭");
    mvwaddwstr(win, 0, wx - 1, L"╮");
    mvwaddwstr(win, wy - 1, 0, L"╰");
    mvwaddwstr(win, wy - 1, wx - 1, L"╯");
}

void draw_heavy_border(WINDOW *win, int wy, int wx) {
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, 0, L"┃");
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, wx-1, L"┃");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, 0, i, L"━");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, wy - 1, i, L"━");
    mvwaddwstr(win, 0, 0, L"┏");
    mvwaddwstr(win, 0, wx - 1, L"┓");
    mvwaddwstr(win, wy - 1, 0, L"┗");
    mvwaddwstr(win, wy - 1, wx - 1, L"┛");
}

void draw_thick_border(WINDOW *win, int wy, int wx) {
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, 0, L"▌");
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, wx-1, L"▐");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, 0, i, L"▀");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, wy - 1, i, L"▄");
    mvwaddwstr(win, 0, 0, L"▛");
    mvwaddwstr(win, 0, wx - 1, L"▜");
    mvwaddwstr(win, wy - 1, 0, L"▙");
    mvwaddwstr(win, wy - 1, wx - 1, L"▟");
}

void draw_full_border(WINDOW *win, int wy, int wx) {
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, 0, L"█");
    for(int i = 1; i < wy - 1; i++) mvwaddwstr(win, i, wx-1, L"█");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, 0, i, L"█");
    for(int i = 1; i < wx - 1; i++) mvwaddwstr(win, wy - 1, i, L"█");
    mvwaddwstr(win, 0, 0, L"█");
    mvwaddwstr(win, 0, wx - 1, L"█");
    mvwaddwstr(win, wy - 1, 0, L"█");
    mvwaddwstr(win, wy - 1, wx - 1, L"█");
}

#endif // BORDERS_UTIL