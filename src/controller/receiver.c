#include "receiver.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../cc1200/cc1200_rate.h"

static clock_t timer_started = 0x0;

receiver_t* receiver_init(int socket_send, int socket_rcv) {
    receiver_t* rcv = calloc(1, sizeof(receiver_t));
    if (rcv == NULL) return NULL;

    uint16_t token_recv = rand() % UINT16_MAX;
    rcv->token_receiver = token_recv;
    if (token_recv == 0) token_recv = 1;
    
    rcv->socket_send = socket_send;
    rcv->socket_rcv = socket_rcv;
    rcv->debug_number_wrong_checksum = 0;
    
    packet_t* pkt = NULL;
    packet_status_t status = packet_status_none;
    while (status != packet_status_ok 
            && status != packet_status_ok_ack
            && status != packet_status_warn_wrong_ack) { //TODO better error handling
        pkt = receiver_receive(rcv, &status);
    }
    for (int i=0;i<pkt->payload_len;i++) {
        printf("%i ", pkt->p_payload[i]);
    }
    printf("\n");

    //assume everything went perfect

    //now set ACK, TOKEN_SEND
    rcv->last_ack_rcv = pkt->id;
    rcv->token_sender = pkt->token_send;
    
    //send ACK packet
    receiver_ack(rcv);

    return rcv;
}

void receiver_destroy(receiver_t *rcv) {
    if (rcv == NULL) return;
    packet_destroy(rcv->lastPacketRcv);
    packet_destroy(rcv->lastPacketSend);
    free(rcv);
}

/**
 * @brief Receives and checks a packet for correct ACK and checksum.
 * 
 * More like a local function. You should use receiver_receive_and_ack()
 * 
 * @param receiver 
 * @param status pointer to status, will be set with content and acts as a second return argument
 * @return packet_t* valid packet, or NULL if an error occured
 */
packet_t* receiver_receive(receiver_t* receiver, packet_status_t *status_back) {
    packet_status_t status = 0;

    timer_started = clock(); //start timer

    packet_t* pkt = cc1200_get_packet(receiver->socket_send, timer_started, &status);

    printf("receiver rcv status of packet: %i\n", status);
    if (pkt != NULL) {
        //TODO check for correct recv token
        //TODO check for correct checksum
        printf("Checksum rcv %i, calced %i\n", pkt->checksum, packet_calc_checksum(pkt));
        if (pkt->checksum != packet_calc_checksum(pkt)) {
            status = packet_status_err_timeout;
            receiver->debug_number_wrong_checksum++;
            printf("Got wrong checksum %i\n", receiver->debug_number_wrong_checksum);
        }
        //check for valid ACK:
        if (pkt->ack != receiver->last_ack_rcv) {
            //if there is a wrong ACK, ignore that and assume we got the correct packet            
            status = packet_status_warn_wrong_ack; //just a hint for the next layer, but not an error
        }
        //assume everything went perfect
        receiver->last_ack_rcv = pkt->id;
        receiver->lastPacketRcv = pkt;
        *status_back = status;
        return pkt;
    } else {
        *status_back = status;
        return NULL;
    }
}

uint8_t receiver_receive_and_ack(receiver_t* receiver, uint8_t** buffer) {
    packet_status_t status = packet_status_none;
    packet_t* pkt=packet_status_none;
    while (status != packet_status_ok 
            && status != packet_status_warn_wrong_ack 
            && status != packet_status_ok_ack) {
        pkt = receiver_receive(receiver, &status);
        printf("Receiver rcv status: %i\n", status);
    }
    receiver_ack(receiver);
    printf("receiver correct status: %i\n", status);
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
    pkt_send->type = packet_status_ok_ack;
    packet_set_checksum(pkt_send);

    cc1200_send_packet(receiver->socket_rcv, pkt_send);
    receiver->lastPacketSend = pkt_send;
}
