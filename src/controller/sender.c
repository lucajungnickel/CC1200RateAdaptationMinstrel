#include "sender.h"

#include <stdlib.h>

#include "minstrel.h"
#include "cc1200_rate.h"

sender_t* sender_init(Minstrel* minstrel) {
    sender_t *sender = calloc(1, sizeof(sender_t));
    if (sender == NULL) return NULL;

    sender->minstrel = minstrel;

    sender->next_ack = 1;

    uint16_t token_sender = rand() % UINT16_MAX;
    sender->token_sender = token_sender;


    //build handshake packet
    packet_t *pkt = calloc(1, sizeof(packet_t));
    pkt->ack = 0;
    pkt->id = sender->next_ack;
    pkt->fallback_rate = minstrel_get_fallback_rate(minstrel);
    pkt->next_symbol_rate = minstrel_get_next_rate(minstrel);
    pkt->p_payload = 0;
    pkt->payload_len = 0;
    pkt->token_recv = 0;
    pkt->token_send = sender->token_sender;
    pkt->type = packet_status_ok;
    packet_set_checksum(pkt);
    //sends handshake packet
    sender_send(sender, pkt);
    free(pkt);
    
    //wait for ack


    return sender;
}

void sender_send(sender_t *sender, packet_t *packet) {
    sender->next_ack = packet->id;
    cc1200_send_packet(packet);
}

packet_status_t sender_rcv_ack(sender_t *sender) {
    packet_t* pkt = cc1200_get_packet();
    //check for correct ACK
}