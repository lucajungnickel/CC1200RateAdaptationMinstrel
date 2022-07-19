#include "receiver.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "../cc1200/cc1200_rate.h"
#include "../log.c/src/log.h"

static clock_t timer_started = 0x0;

receiver_t* receiver_init(int socket_send, int socket_rcv) {
    srand(time(0));

    receiver_t* rcv = calloc(1, sizeof(receiver_t));
    if (rcv == NULL) return NULL;

    uint16_t token_recv = rand() % UINT16_MAX;
    if (token_recv == 0) token_recv = 1;
    rcv->token_receiver = token_recv;
    
    log_fatal("Generated token recv: %i", rcv->token_receiver);

    rcv->socket_send = socket_send;
    rcv->socket_rcv = socket_rcv;
    rcv->debug_number_wrong_checksum = 0;
    rcv->last_symbol_rate = 0; //symbol rate is set to lowest at beginning
    rcv->timeout_counter = 0;

    packet_t* pkt = NULL;
    packet_status_t status = packet_status_none;
    while (status != packet_status_ok 
            && status != packet_status_ok_ack
            && status != packet_status_warn_wrong_ack) { //TODO better error handling
        pkt = receiver_receive(rcv, &status);
    }
    if (!IS_IN_GRAPHIC_MODE) log_debug("receiver init worked fine, here is the output of the payload");

    for (int i=0;i<pkt->payload_len;i++) {
        if (!IS_IN_GRAPHIC_MODE) log_debug("%i ", pkt->p_payload[i]);
    }
    if (!IS_IN_GRAPHIC_MODE) log_debug("");

    //assume everything went perfect

    //now set ACK, TOKEN_SEND
    rcv->last_ack_rcv = pkt->id;
    rcv->token_sender = pkt->token_send;
    log_fatal("Set token sender: %i", pkt->token_send);
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

    if (!IS_IN_GRAPHIC_MODE) {
        switch (status) {
            case 0:
            log_debug("receiver status: no status");
            break;
            case 1:
            log_debug("receiver status: OK");
            break;
            case 2:
            log_debug("receiver status: OK ack");
            break;
            case 3:
            log_debug("receiver status: WARN wrong ack");
            break;
            case 4:
            log_debug("receiver status: TIMEOUT");
            break;
            case 5:
            log_debug("receiver status: ERR CHECKSUM");
            break;
            case 6:
            log_debug("receiver status: ERR TOKEN RCV");
            break;
            case 7:
            log_debug("receiver status: ERR TOKEN SEND");
            break;
        }
    }
    if (pkt != NULL) {
        //TODO check for correct recv token

        //TODO check for correct checksum
        if (!IS_IN_GRAPHIC_MODE) log_debug("Checksum rcv %i, calced %i", pkt->checksum, packet_calc_checksum(pkt));
        if (pkt->checksum != packet_calc_checksum(pkt)) {
            status = packet_status_err_checksum;
            
            receiver->debug_number_wrong_checksum++;
            if (!IS_IN_GRAPHIC_MODE) log_warn("Got wrong checksum %i", receiver->debug_number_wrong_checksum);
            packet_destroy(pkt);
            return NULL;
        }
        //check for valid ACK:
        //TODO remove ACK 
        if (pkt->id != receiver->last_ack_rcv) {
            //if there is a wrong ACK, ignore that and assume we got the correct packet            
            status = packet_status_warn_wrong_ack; //just a hint for the next layer, but not an error
            //log_warn("Got wrong ack %i, should %i", receiver->last_ack_rcv, pkt->ack);
        }
        //assume everything went perfect
        receiver->last_ack_rcv = pkt->id;
        packet_destroy(receiver->lastPacketRcv);
        receiver->lastPacketRcv = pkt;
        
        //change rate if there is another symbol rate
        if (pkt->next_symbol_rate != receiver->last_symbol_rate) {
            //cc1200_change_rate(receiver->socket_rcv, pkt->next_symbol_rate);
            receiver->last_symbol_rate = pkt->next_symbol_rate;
        }

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
        if (status != packet_status_ok 
            && status != packet_status_warn_wrong_ack 
            && status != packet_status_ok_ack) {
                receiver->timeout_counter++;
                if (!IS_IN_GRAPHIC_MODE) log_warn("timeout or wrong status");
                //change to fallback rate
                if (receiver->timeout_counter == RCV_MAX_TIMEOUTS) {
                    if (!IS_IN_GRAPHIC_MODE) log_error("max timeouts reached, changing to second best rate");
                    cc1200_change_rate(receiver->socket_rcv, receiver->last_second_best_rate);
                } else if (receiver->timeout_counter == RCV_MAX_TIMEOUTS*2) {
                    if (!IS_IN_GRAPHIC_MODE) log_error("max timeouts reached, changing to highest prob rate");
                    cc1200_change_rate(receiver->socket_rcv, receiver->last_highest_prob_rate);
                } else if (receiver->timeout_counter == RCV_MAX_TIMEOUTS*3) {
                    if (!IS_IN_GRAPHIC_MODE) log_error("max timeouts reached, changing to fallback rate");
                    cc1200_change_rate(receiver->socket_rcv, receiver->last_fallback_rate);
                }
        }
    }
    receiver->timeout_counter = 0; //reset timeout counter and start at 0 again
    receiver_ack(receiver);
    if (!IS_IN_GRAPHIC_MODE) log_debug("receiver correct status: %i", status);
    //change rate after ack is send
    cc1200_change_rate(receiver->socket_rcv, pkt->next_symbol_rate);
    receiver->last_symbol_rate = pkt->next_symbol_rate;
    receiver->last_fallback_rate = pkt->fallback_rate;
    receiver->last_highest_prob_rate = pkt->next_highest_pro_rate;
    receiver->last_second_best_rate = pkt->next_second_best_rate;

    *buffer = pkt->p_payload;
    return pkt->payload_len;
}

void receiver_ack(receiver_t* receiver) {
    bool should_send = true;
    packet_t *pkt_send;
    while (should_send) {
        //build ACK packet 
        pkt_send = calloc(1, sizeof(packet_t));
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
        log_fatal("Send handshake token recv: %i", pkt_send->token_recv);
        cc1200_status_send status = cc1200_send_packet(receiver->socket_rcv, pkt_send);
        
        switch (status) {
            case cc1200_status_send_ok:
                should_send = false; //ACK is succesfully send, go on and break loop
            break;
            case cc1200_status_send_error:
                packet_destroy(pkt_send);
                should_send = true; //continue sending
            break;
        }
    }

    packet_destroy(receiver->lastPacketSend);
    receiver->lastPacketSend = pkt_send;
}
