#include "sender.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../minstrel/minstrel.h"
#include "../cc1200/cc1200_rate.h"
#include "../log.c/src/log.h"

static clock_t timer_started = 0x0;



sender_t* sender_init(Minstrel* minstrel, int socket_send, int socket_rcv) {
    if (minstrel == NULL) return NULL;

    sender_t *sender = calloc(1, sizeof(sender_t));
    if (sender == NULL) return NULL;
    
    //builds sender
    sender->minstrel = minstrel;
    sender->debug_number_wrong_checksum = 0;
    sender->next_ack = 1;
    sender->socket_rcv = socket_rcv;
    sender->socket_send = socket_send;
    sender->lastPacketRcv = NULL;
    sender->lastPacketSend = NULL;
    //randomly generating a token for sender:
    uint16_t token_sender = rand() % UINT16_MAX;
    sender->token_sender = token_sender;
    if (token_sender == 0) token_sender = 1;
    
    log_debug("Sender reference is built");

    //sends handshake packet, no payload
    sender_send_and_ack(sender, NULL, 0);
    
    return sender;
}

//use it like a local function
static void sender_send(sender_t *sender, packet_t *packet) {
    if (sender == NULL || packet == NULL) return;
    sender->next_ack = packet->id; //send last send and next expected ACK
    packet_destroy(sender->lastPacketSend); //destroy old reference
    sender->lastPacketSend = packet; //update last packet send

    bool should_send = true;
    while (should_send) {
        log_debug("Send the packet to cc1200 module");
        cc1200_status_send status = cc1200_send_packet(sender->socket_send, packet);
        log_debug("Send to module done");
        switch (status) {
            case cc1200_status_send_ok:
                should_send = false;
            break;
            case cc1200_status_send_error:
                log_warn("Warning: CC1200 sending failed");
                should_send = true;
            break;
        }
    }
    log_debug("Sender packet sent");
    
    //start timer
    timer_started = clock(); //start timer for read timeout
}

void sender_destroy(sender_t *sender) {
    if (sender == NULL) return;
    minstrel_destroy(sender->minstrel);
    packet_destroy(sender->lastPacketRcv);
    packet_destroy(sender->lastPacketSend);
    free(sender);
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

    //update last packet received, even if it's NULL
    packet_destroy(sender->lastPacketRcv);
    sender->lastPacketRcv = pkt;

    if (pkt == NULL) {
        return status;
    } else {
        if (status == packet_status_err_timeout) { 
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
}

/**
 * @brief Helper function for sender_send_and_ack, builds the packet and returns it
 * 
 */
static packet_t* sender_build_pkt(sender_t *sender, uint8_t* buffer, uint32_t len) {
    //build send packet
    packet_t *pkt = calloc(1, sizeof(packet_t));
    pkt->ack = 0;
    pkt->fallback_rate = sender->minstrel->rates.fallback;
    pkt->id = sender->next_ack;
    pkt->next_symbol_rate = sender->minstrel->rates.current;
    pkt->token_recv = sender->token_receiver;
    pkt->token_send = sender->token_sender;
    pkt->payload_len = len;
    pkt->p_payload = malloc(len * sizeof(uint8_t));
    memcpy(pkt->p_payload, buffer, len);
    pkt->type = packet_status_ok;

    packet_set_checksum(pkt);
    
    return pkt;
}

//Deep copies from buffer
void sender_send_and_ack(sender_t *sender, uint8_t* buffer, uint32_t len) {
    log_debug("Start send and ack");
    packet_t* pkt = sender_build_pkt(sender, buffer, len);
    sender_send(sender, pkt);
    
    bool should_send = true;
    while (should_send) {
        log_debug("Try to receive an ACK");
        packet_status_t status = sender_rcv_ack(sender);
        log_debug("Received at sender status: %i", status);
        if (status == packet_status_ok || status == packet_status_ok_ack) {
            should_send = false;
            log_info("Got good status, leave loop here");
            break;
        } else if (status == packet_status_warn_wrong_ack) {
            //Sender shouldn't receive invalid ACK
            //,then send packet again
            log_debug("Wrong ACK, send again");
            pkt = sender_build_pkt(sender, buffer, len);
            sender_send(sender, pkt);
        } else if (status == packet_status_err_timeout) {
            //then send packet again
            log_debug("Timeout, send again");
            pkt = sender_build_pkt(sender, buffer, len);
            log_debug("PKT built again");
            sender_send(sender, pkt);
            log_debug("packet after timeout sent");
        }
    }
    log_info("Sender done sending ack");
}
