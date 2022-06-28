#include "receiver.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "cc1200_rate.h"

static clock_t timer_started = 0x0;

receiver_t* receiver_init() {
    receiver_t* rcv = calloc(1, sizeof(receiver_t));
    if (rcv == NULL) return NULL;

    uint16_t token_recv = rand() % UINT16_MAX;
    rcv->token_receiver = token_recv;
    if (token_recv == 0) token_recv = 1;
    
    packet_t* pkt = NULL;
    while (pkt == NULL) { //TODO better error handling
        pkt = receiver_receive(rcv);
    }
    //assume everything went perfect

    //now set ACK, TOKEN_SEND
    rcv->last_ack_rcv = pkt->id;
    rcv->token_sender = pkt->token_send;
    
    //send ACK packet
    receiver_ack(rcv);

    return rcv;
}

/**
 * @brief More like a local function. You should use receiver_receive_and_ack()
 * 
 * @param receiver 
 * @return packet_t* 
 */
packet_t* receiver_receive(receiver_t* receiver) {
    packet_status_t status = 0;
    packet_t* pkt = cc1200_get_packet(timer_started, &status);
    receiver->last_ack_rcv = pkt->id;
    receiver->lastPacketRcv = pkt;
    return pkt;
}

uint8_t receiver_receive_and_ack(receiver_t* receiver, uint8_t** buffer) {
    packet_t* pkt = receiver_receive(receiver);
    receiver_ack(receiver);
    *buffer = pkt->p_payload;
    return pkt->payload_len;
}

void receiver_ack(receiver_t* receiver) {
    //build ACK packet 
    packet_t *pkt_send = calloc(1, sizeof(packet_t));
    pkt_send->ack = receiver->last_ack_rcv;
    pkt_send->payload_len = 0; //no payload
    pkt_send->p_payload = 0x0;
    pkt_send->fallback_rate = 0; //rcv doesnt set this
    pkt_send->next_symbol_rate = 0; //rcv doesnt set this
    pkt_send->id = receiver->last_ack_rcv;
    pkt_send->token_recv = receiver->token_receiver;
    pkt_send->token_send = receiver->token_sender;
    pkt_send->type = packet_status_ack;
    packet_set_checksum(pkt_send);

    cc1200_send_packet(pkt_send);
    receiver->lastPacketSend = pkt_send;
    
}