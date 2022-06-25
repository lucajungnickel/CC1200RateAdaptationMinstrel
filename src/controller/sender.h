#ifndef SENDER_H
#define SENDER_H

#include <stdint.h>

#include "packet.h"
#include "minstrel.h"

typedef struct sender_t {
    uint32_t next_ack;
    uint16_t token_sender;
    uint16_t token_receiver;
    Minstrel *minstrel; //minstrel algorithm reference
} sender_t;

/**
 * @brief Tries to establish a connection to a receiver.
 * 
 * @param minstrel reference to minstrel algorithm
 * 
 * @return sender_t sender or NULL if error ocurred
 */
sender_t* sender_init(Minstrel *minstrel);

/**
 * @brief Sends one packet with the given sender.
 * 
 * @param sender given sender
 * @param packet packet which will be send
 */
void sender_send(sender_t *sender, packet_t *packet);

/**
 * @brief Receives an ACK from receiver.
 * 
 * Blocking function 
 * 
 * @return status of the received packet 
 */
packet_status_t sender_rcv_ack(sender_t *sender);
#endif //SENDER_H