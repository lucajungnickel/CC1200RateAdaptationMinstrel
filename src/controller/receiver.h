/**
 * @file receiver.h
 * @author Luca Jungnickel
 * @brief Handles all the receiver logic.
 */

#ifndef RECEIVER_H
#define RECEIVER_H

#include "stdint.h"
#include "packet.h"
#include "../minstrel/minstrel.h"

typedef struct receiver_t {
    uint32_t last_ack_rcv;
    uint16_t token_receiver;
    uint16_t token_sender; //receiver needs to know how the sender is
    packet_t* lastPacketSend; //for better debugging
    packet_t* lastPacketRcv; //for better debugging
    int debug_number_wrong_checksum; //for debugging, could be removed in future
    int socket_send;
    int socket_rcv;
} receiver_t;


/**
 * @brief Tries to establish a connection to a sender.
 * Waits for a connection.
 * If successful, returns a receiver_t struct. 
 * 
 * Blocking function
 * 
 * @return receiver_t receiver or NULL if error
 */
receiver_t* receiver_init(int socket_send, int socket_rcv);

/**
 * @brief Destroys the given receiver with all it's data content.
 * 
 */
void receiver_destroy(receiver_t *rcv);

/**
 * @brief Receives a packet.
 * Checks the checksum and filter for correct rcv and sender tokens.
 * 
 * @param receiver given receiver
 * @param status pointer to status, will be returned like a second return value
 * @return packet_t the received packet or NULL if error
 */
packet_t* receiver_receive(receiver_t* const receiver, packet_status_t* status);

/**
 * @brief Receives and acks a packet and returns the payload.
 * 
 * Call like: receiver_receive_and_ack(rcv, &buffer)
 * 
 * @param receiver Receiver
 * @param buffer Pointer to buffer where data will be written to
 * @return uint8_t 
 */
uint8_t receiver_receive_and_ack(receiver_t* const receiver, uint8_t** buffer);

/**
 * @brief Acknowleges the last packet and sends this ACK packet.
 * @param receiver 
 */
void receiver_ack(receiver_t* const receiver);

#endif //RECEIVER_H
