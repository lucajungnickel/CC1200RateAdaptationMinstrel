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

#include "packet.h"
#include "../minstrel/minstrel.h"
#include "../cc1200/cc1200_rate.h"

static void start() {
    // Init Minstrel state
    Minstrel* minstrel = minstrel_init();
    if (minstrel->state != READY) {
        puts("ERROR: Couldn't initialize minstrel");
        exit(1);
    }
    //packet id of the next sending packet
    uint32_t pkt_id = 0;
    while (true) {
        unsigned int next_rate = minstrel->rates.current;
        
        //change rate
        cc1200_change_rate(next_rate);

        //build next package
        packet_t *pkt = malloc(sizeof(packet_t));
        
        pkt->ack = 0;
        // TODO: use minstrel->rates.X here
        pkt->fallback_rate = MINSTREL_RATES[minstrel_get_fallback_rate(minstrel)];
        
        pkt->id = pkt_id;

        pkt->next_symbol_rate = next_rate;

        pkt->token_recv = 5; //TODO implement handshake
        pkt->token_send = 6; //TODO implement handshake
        
        pkt->type = packet_status_ok;

        //set payload TODO überarbeiten
        pkt->payload_len = 50;
        pkt->p_payload = malloc(sizeof(uint8_t) * pkt->payload_len);
        for (int i=0; i<pkt->payload_len; i++) {
            pkt->p_payload[i] = i % 256;
        }
        
        packet_set_checksum(pkt);
        //packet build done

        //serialize
        uint32_t len = packet_get_size(pkt);
        uint8_t* send_buf = malloc(len * sizeof(uint8_t));
        packet_serialize(pkt, send_buf);

        //send
        cc1200_send_packet(pkt);
        free(send_buf); //clean up

        //receive
        uint8_t* recv_buf;
        uint32_t recv_size = cc1200_get_packet(recv_buf);

        //process packet..
        packet_t *pkt_rcv = packet_deserialize(recv_buf);
        
        //int res = waitForACK()
        //if (res == CONNECTION_LOST) return 0;
        //logPackageStatus(package->id, res)

        // Update Minstrel state + statistics (incl. used rate)
        minstrel_update(minstrel);
 
       //increment packet id for next send
        if (pkt_id != UINT32_MAX) pkt_id++;
        else pkt_id = 0;
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
