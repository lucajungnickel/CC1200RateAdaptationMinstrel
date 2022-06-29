#include "sender.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "../minstrel/minstrel.h"
#include "../cc1200/cc1200_rate.h"

static clock_t timer_started = 0x0;



sender_t* sender_init(Minstrel* minstrel, int socket_send, int socket_rcv) {
    sender_t *sender = calloc(1, sizeof(sender_t));
    if (sender == NULL) return NULL;

    sender->minstrel = minstrel;

    sender->debug_number_wrong_checksum = 0;
    sender->next_ack = 1;

    sender->socket_rcv = socket_rcv;
    sender->socket_send = socket_send;

    uint16_t token_sender = rand() % UINT16_MAX;
    sender->token_sender = token_sender;
    if (token_sender == 0) token_sender = 1;

/*
    //build handshake packet
    packet_t *pkt = calloc(1, sizeof(packet_t));
    pkt->ack = 0;
    pkt->id = sender->next_ack;
    pkt->fallback_rate = minstrel_get_fallback_rate(minstrel);
    pkt->next_symbol_rate = minstrel->rates.current;
    pkt->p_payload = 0;
    pkt->payload_len = 0;
    pkt->token_recv = 0;
    pkt->token_send = sender->token_sender;
    pkt->type = packet_status_ok;
    packet_set_checksum(pkt);
    */
  
    //sends handshake packet
    sender_send_and_ack(sender, NULL, 0);
    
    /**
    sender_send(sender, pkt);

    //wait for ack
    packet_status_t status = packet_status_none;
    while (status != packet_status_ok) {
        status = sender_rcv_ack(sender);
        if (status == packet_status_err_timeout) { //timeout, try to send packet again
            sender_send(sender, pkt);
        }
    }*/
    return sender;
}

void sender_send(sender_t *sender, packet_t *packet) {
    sender->next_ack = packet->id;
    sender->lastPacketSend = packet;

    cc1200_send_packet(sender->socket_send, packet);
    printf("Sender packet sent\n");
    //start timer
    timer_started = clock();
}

/**
 * @brief Receives an ACK for the given sender.
 * 
 * Blocking function.
 * 
 * @param sender 
 * @return packet_status_t status of packet which was received
 */
packet_status_t sender_rcv_ack(sender_t *sender) {
    packet_status_t status;

    packet_t* pkt = cc1200_get_packet(sender->socket_rcv, timer_started, &status);

    sender->lastPacketRcv = pkt;
    if (pkt == NULL) return status;
    if (status == packet_status_err_timeout) { //if there is a timeout, start clock again!
        return packet_status_err_timeout;
    }
    //check for correct CHECKSUM
    if (pkt->checksum != packet_calc_checksum(pkt)) {
        sender->debug_number_wrong_checksum++;
        return packet_status_err_checksum;
    }
    //check for correct ACK
    if (pkt->ack == sender->next_ack) {
        sender->next_ack++;
        return packet_status_ok;
    } else return packet_status_warn_wrong_ack;
    
}

void sender_send_and_ack(sender_t *sender, uint8_t* buffer, uint32_t len) {
    //build send packet
    packet_t *pkt = calloc(1, sizeof(packet_t));
    pkt->ack = 0;
    pkt->fallback_rate = minstrel_get_fallback_rate(sender->minstrel);
    pkt->id = sender->next_ack;
    pkt->next_symbol_rate = sender->minstrel->rates.current;
    pkt->token_recv = sender->token_receiver;
    pkt->token_send = sender->token_sender;
    pkt->payload_len = len;
    pkt->p_payload = buffer;
    pkt->type = packet_status_ok;

    packet_set_checksum(pkt);

    sender_send(sender, pkt);
    
    bool should_send = true;
    while (should_send) {
        packet_status_t status = sender_rcv_ack(sender);
        printf("Received at sender: %i\n", status);
        if (status == packet_status_ok || status == packet_status_ok_ack) {
            should_send = false;
            break;
        } else if (status == packet_status_warn_wrong_ack) {
             //Sender shouldn't receive invalid ACK
             //,then send packet again
            sender_send(sender, pkt);
        } else if (status == packet_status_err_timeout) {
            //then send packet again
            sender_send(sender, pkt);
        }
    }
    printf("Sender done sending ack\n");
}
