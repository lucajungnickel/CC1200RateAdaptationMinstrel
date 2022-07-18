/**
 * @file main_sender.c
 * @author Luca Jungnickel
 * @brief
 * @version 0.1
 * @date 2022-06-18
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <curses.h>
#include <pthread.h>

#include "packet.h"
#include "ui_sender.h"
#include "sender_interface.h"
#include "../minstrel/minstrel.h"
#include "../cc1200/cc1200_rate.h"
#include "../log.c/src/log.h"

static int id_sender = 10;
static int id_rcv = 20;



static void start() {
    cc1200_init(id_sender);
    cc1200_init(id_rcv);
    cc1200_change_rate(id_sender, 0); //TODO remove, for debugging only
    
    //sender_interface
    log_debug("Start init sender_interface in test function");
    sender_interface_t* s_interface = sender_interface_init(id_sender, id_rcv);
    log_debug("Handshake succeeded in sender interface");

    uint32_t data_size = 1024 * 2;
    uint8_t *buffer = calloc(data_size, sizeof(uint8_t));
    for (int i=0;i < data_size;i++) {
        buffer[i] = i % 256;
    }    
    
    while (true) { //for testing purpose there is a while loop here
        log_debug("Try to send a lot of data");
        sender_interface_send_data(s_interface, buffer, data_size);
        log_info("All data sent!");
    }
    free(buffer);
}

void *thread_stop() {
    getch();
    ui_cleanup();
    cc1200_reset(0);
    exit(0);
}

void *thread_refresh() {
    while (true) {
        usleep(500 * 1000);
        ui_show();
    }
}

int main(int argc, char** argv)
{
    IS_IN_GRAPHIC_MODE = true;
    if (argc == 2) {
        puts("Using DEBUG mode\n");
        IS_DEBUG = 1;
        IS_IN_GRAPHIC_MODE = false;
    }
    if (IS_IN_GRAPHIC_MODE) {
        log_set_level(LOG_FATAL);
        //ui_init();
        
        //ui_show();
        //pthread_t thread_stop_id, thread_refresh_id;
        //pthread_create(&thread_stop_id, NULL, thread_stop, NULL);
        //pthread_create(&thread_refresh_id, NULL, thread_refresh, NULL);
    }
    start();
    ui_cleanup();
}
