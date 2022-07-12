/**
 * @file main_receiver.c
 * @author Luca
 * @date 2022-07-05
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../cc1200/cc1200_rate.h"
#include "../controller/receiver.h"
#include "../log.c/src/log.h"

static int id_sender = 10;
static int id_rcv = 20;

static void start() {
    
    cc1200_init(id_sender);
    cc1200_change_rate(id_sender, 0); //TODO remove, for debugging only
    //log_set_level(LOG_WARN);
    
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    uint32_t currentByte = 0;
    int numPacket = receiver->last_ack_rcv;
    while (true) {

        uint8_t* buffer;
        uint8_t len = receiver_receive_and_ack(receiver, &buffer);

        for (int i=0;i<len;i++) {
            log_debug("RCV: [%i] %i", currentByte, buffer[i]);
            currentByte++;
        }

        numPacket++;
    }
}

int main(int argc, char** argv)
{
    if (argc == 2) {
        puts("Using DEBUG mode\n");
        IS_DEBUG = 1;
    }
    start();
}
