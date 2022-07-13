#include "ui_sender.h"

#include <curses.h>
#include <stdint.h>

WINDOW *uiwindow = NULL;

//variables to be rendered:
uint8_t fallback = 0;
uint8_t highest_prob = 0;
uint8_t second_best = 0;
uint8_t best = 0;
uint8_t current = 0;
uint8_t probe = 0;

uint32_t last_pkt_id = 0;
uint32_t pkt_count = 0;
float ewma = 0; // Success probability
float throughput = 0;
uint32_t total_send = 0;
uint32_t total_recv = 0;
uint32_t bytes_send = 0;
uint32_t avg_duration = 0;

static void set_colors() {

}

bool ui_init() {
    uiwindow = initscr();
    if (uiwindow == NULL) return false;
    start_color();
    if (!has_colors() || !can_change_color() || COLOR_PAIRS < 6) {
        printf("Warning, UI doesn't have colors\n");
        return false;
    }

    set_colors();

    raw();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();	
    curs_set(0); //no blinking cursor

    return true;
}
bool ui_cleanup() {
    delwin(uiwindow);
    endwin();
    refresh();
    return true;
}
bool ui_show() {
    color_set(0, NULL);
    mvaddnstr(0, 0, "Hello world", 11);
    mvaddch(1, 0, 'U');
    refresh();
}
bool ui_update(Minstrel* minstrel) {
    
}