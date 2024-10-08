#include <ncurses.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#include <locale.h>
 
int main()
{
    setlocale(LC_ALL, "");
	// initscr();
	// WINDOW *mywin = newwin(LINES, COLS, 0, 0); // Create a 10x20 window at (0, 0)
    // box(mywin, 0, 0);
    // mvwprintw(mywin, LINES / 2 - 1, COLS / 2, "Hello, the timestamp is %ld", time(NULL));
    // wrefresh(mywin);
    // int ch = wgetch(mywin);
    // delwin(mywin);
	// endwin();
    putchar(186);
 
	return 0;
}