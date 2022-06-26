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
    if (token_sender == 0) token_sender = 1;

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
    packet_status_t status = packet_status_lost;
    while (status != packet_status_ok) {
        status = sender_rcv_ack(sender);
    }
    return sender;
}

void sender_send(sender_t *sender, packet_t *packet) {
    sender->next_ack = packet->id;
    sender->lastPacketSend = packet;
    cc1200_send_packet(packet);
}

packet_status_t sender_rcv_ack(sender_t *sender) {
    packet_t* pkt = cc1200_get_packet();
    sender->lastPacketRcv = pkt;
    //check for correct ACK
    if (pkt->ack == sender->next_ack) {
        sender->next_ack++;
        return packet_status_ok;
    } else return packet_status_wrong_ack;
    
}

void sender_send_and_ack(sender_t *sender, uint8_t* buffer, uint32_t len) {
    //build send packet
    packet_t *pkt = calloc(1, sizeof(packet_t));
    pkt->ack = 0;
    pkt->fallback_rate = minstrel_get_fallback_rate(sender->minstrel);
    pkt->id = sender->next_ack;
    pkt->next_symbol_rate = minstrel_get_next_rate(sender->minstrel);
    pkt->token_recv = sender->token_receiver;
    pkt->token_send = sender->token_sender;
    pkt->payload_len = len;
    pkt->p_payload = buffer;
    packet_set_checksum(pkt);

    sender_send(sender, pkt);
    sender_rcv_ack(sender);
}