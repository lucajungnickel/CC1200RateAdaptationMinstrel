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
} receiver_t;


/**
 * @brief Tries to establish a connection to a sender.
 * Waits for a connection.
 * If successful, returns a receiver_t struct. 
 * 
 * Blocking function
 * 
 * @param device_id @see CC1200 device ID
 * 
 * @return receiver_t receiver or NULL if error
 */
receiver_t* receiver_init(int device_id);

/**
 * @brief Switches to the given device for receiving.
 * Could be ignored in the implementation.
 * 
 * @param device_id 
 */
void receiver_switch_device(int device_id);

/**
 * @brief Receives a packet.
 * Checks the checksum and filter for correct rcv and sender tokens.
 * 
 * @param receiver given receiver
 * @return packet_t the received packet or NULL if error
 */
packet_t* receiver_receive(receiver_t* const receiver);

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
