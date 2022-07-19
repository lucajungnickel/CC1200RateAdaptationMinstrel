#include "ui_sender.h"

#include <curses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "../log.c/src/log.h"
#include "packet.h"

#define MAX_CHANGES_PRINT 15

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

packet_status_t last_status = 0;
char status_strs[8][20] = {
    "NONE",
    "OK",
    "OK - ACK",
    "WARN - WRONG ACK",
    "ERR - TIMEOUT",
    "ERR - CHECKSUM",
    "ERR - TOKEN RCV",
    "ERR - TOKEN SEND"};
char* status_str = status_strs[0];

//array with 15 last rate changes
typedef struct rate_changes {
    int pkt;
    int rate;
} rate_changes;

rate_changes changes[MAX_CHANGES_PRINT];
int last_change = 0;

static void set_colors() {

}

bool ui_init() {
    //if (!IS_IN_GRAPHIC_MODE) return false;

    uiwindow = initscr();
    if (uiwindow == NULL) return false;
    start_color();
    if (!has_colors() || !can_change_color() || COLOR_PAIRS < 6) {
        printf("Warning, UI doesn't have colors\n");
        return false;
    }

    //set colors:
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_GREEN, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_WHITE);

    raw();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();	
    curs_set(0); //no blinking cursor

    //init array
    for (int i=0;i<MAX_CHANGES_PRINT;i++) {
        changes[i].pkt = 0;
        changes[i].rate = 0;
    }
    return true;
}
bool ui_cleanup() {
   // if (!IS_IN_GRAPHIC_MODE) return false;

    delwin(uiwindow);
    endwin();
    refresh();
    return true;
}
bool ui_show() {
    //if (!IS_IN_GRAPHIC_MODE) return false;

    clock_t start = clock();

    mvaddnstr(0, 0, "Minstrel stats:", 15);
    attron(COLOR_PAIR(1));
    mvaddstr(1, 0, "\tRates: \t\t\t");
    mvprintw(2, 0, "Best: \t\t%i\t\t", best);
    mvprintw(3, 0, "Second best: \t%i\t\t", second_best);
    mvprintw(4, 0, "Highest Prob: \t%i\t\t", highest_prob);
    mvprintw(5, 0, "Fallback: \t%i\t\t", fallback);
    mvprintw(6, 0, "Current: \t%i\t\t", current);
    mvprintw(7, 0, "Probe: \t\t%i\t\t", probe);
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));
    mvaddstr(8, 0, "\tStatistics: \t\t");
    mvprintw(9, 0, "Last Pkt ID: \t\t%i\t", last_pkt_id);
    mvprintw(10, 0, "EWMA: \t\t\t%f", ewma);
    mvprintw(11, 0, "Throughput: \t\t%f", throughput);
    mvprintw(12, 0, "Total Send: \t\t%i\t", total_send);
    mvprintw(13, 0, "Total Rcv: \t\t%i\t", total_recv);
    mvprintw(14, 0, "Bytes Sent: \t\t%i\t", bytes_send);
    mvprintw(15, 0, "Avg Duration: \t\t%i\t", avg_duration);
    mvprintw(16, 0, "\t\t\t\t");
    mvprintw(17, 0, "Last packet send status: %s\t", status_str);
    attroff(COLOR_PAIR(2));

    //print last changes:
    int x_offset = 50;
    mvaddstr(0, x_offset, "Pkt ID\t Rate \t");
    for (int i=0; i < MAX_CHANGES_PRINT; i++) {
        if (i == last_change) attron(COLOR_PAIR(3));
        mvprintw(1 + i, x_offset, "%i \t\t %i \t", changes[i].pkt, changes[i].rate);
        if (i == last_change) attroff(COLOR_PAIR(3));
    }
    refresh();

    clock_t diff = clock() - start;
    int msec = diff * 1000 / CLOCKS_PER_SEC;
    mvprintw(18, 0, "Time rendering: %i", msec);
    mvprintw(19, 0, "diff: %i", diff);
}
bool ui_update(Minstrel* minstrel) {
    //if (!IS_IN_GRAPHIC_MODE) return false;
    #define ANSI_COLOR_GREEN   "\x1b[32m"
    #define ANSI_COLOR_RESET   "\x1b[0m"

    puts("\n****************** MINSTREL STATISTICS *************************");
    printf("* fallback:      %d -> %d SPS\n", minstrel->rates.fallback, MINSTREL_RATES[minstrel->rates.fallback]);
    printf("* highest_prob:  %d -> %d SPS\n", minstrel->rates.highest_prob, MINSTREL_RATES[minstrel->rates.highest_prob]);
    printf("* second_best:   %d -> %d SPS\n", minstrel->rates.second_best, MINSTREL_RATES[minstrel->rates.second_best]);
    printf("* best:          %d -> %d SPS\n", minstrel->rates.best, MINSTREL_RATES[minstrel->rates.best]);
    printf(ANSI_COLOR_GREEN "* current        %d -> %d SPS" ANSI_COLOR_RESET "\n", minstrel_get_next_rate(minstrel), MINSTREL_RATES[minstrel_get_next_rate(minstrel)]);
    printf("* probe:         %d -> %d SPS\n", minstrel->rates.probe, MINSTREL_RATES[minstrel->rates.probe]);
    printf("* is_probe:      %d\n", minstrel->rate_state == PROBE_RATE ? 1 : 0);
    printf("* last_pkt_id:   %d\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].last_pkt_id);
    printf("* pkt_count:     %d\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].pkt_count);
    printf("* ewma:          %f\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].ewma);
    printf("* throughput:    %f\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].throughput);
    printf("* total_send:    %d PKTs\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].total_send);
    printf("* total_recv:    %d PKTs\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].total_recv);
    printf("* bytes_send:    %d\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].bytes_send);
    printf("* avg_duration:  %d ms\n", minstrel->statistics[minstrel_get_next_rate(minstrel)].avg_duration);
    puts("****************************************************************\n\n\n");
}

bool ui_add_rate_change(int pkt_id, int new_rate) {
    //check if last change is another rate
    if (changes[last_change].rate != new_rate) {
        last_change++;
        if (last_change == MAX_CHANGES_PRINT) last_change = 0;
        changes[last_change].rate = new_rate;
        changes[last_change].pkt = pkt_id;
    } //else ignore
}

bool ui_add_last_status(packet_status_t status) {
    last_status = status;
    status_str = status_strs[status];
}
