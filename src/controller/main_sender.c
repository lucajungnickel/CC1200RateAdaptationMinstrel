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

#include "packet.h"
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
        sleep(1);
        log_info("All data sent!");
    }
    free(buffer);
}

int main(int argc, char** argv)
{
    if (argc == 2) {
        puts("Using DEBUG mode\n");
        IS_DEBUG = 1;
    }
    start();
}
