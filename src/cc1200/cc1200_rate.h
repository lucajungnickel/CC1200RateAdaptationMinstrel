/**
 * @file cc1200_rate.h
 * @author Jannis, Luca
 * @brief
 * @version 0.1
 * @date 2022-06-19
 *
 * @copyright Copyright (c) 2022
 *
 * Contains settings and functions for the CC1200 rate changing.
 */

#ifndef CC1200_RATE_H
#define CC1200_RATE_H

#include <time.h>
#include "../controller/packet.h"
#include "register_config.h"

// Register
#define REG_FIFO 0x3F

// Packet config
#define PKT_MODE 1 // Variable length mode
#define PKT_FORMAT 0 // FIFO mode/normal mode
#define PKT_OVERHEAD 2 // 2 bytes packet overhead

// Crystal frequency of the TI boards (40MHz)
#define F_XOSC 40000


typedef enum cc1200_status_send {
    cc1200_status_send_ok,
    cc1200_status_send_error
} cc1200_status_send;

// Global debug flag
int IS_DEBUG;

/**
 * @brief Defines the timeout for the sender after it sends a packet
 * and waits for the ACK.
 * Is the timeout is reached, the receive function assumes that
 * something went wrong.
 *
 * Unit is in MILLISECONDS.
 *
 * Should be used in cc1200_get_packet
 */
const int TIMEOUT;

void cc1200_reset(int device_id);

/**
 * @brief Initializes a cc1200 unit with a given id.
 *
 * @param id Theoretically a MCU could have multiple CC1200 attached,
 * with this parameter you could choose which to initialize
 */
void cc1200_init(int device_id);

/**
 * @brief Switches to the given system with the id.
 * Could be implemented if you would like to have different CC1200
 * on one system, or ignored.
 * @param id
 */
void cc1200_switch_to_system(int device_id);


void cc1200_change_rate(int device_id, uint8_t rate);

/**
 * @brief Sends a packet.
 * 
 * @param device_id socket id for sender. Implementation can use this or ignore it
 * @param packet packet to be send
 * @return cc1200_status_send Status after sending
 */
cc1200_status_send cc1200_send_packet(int device_id, packet_t* packet);

/**
 * @brief Reads and returns a packet.
 *
 * Blocking function
 *
 * @param timeout_started Time the timeout counter started.
 *  If (timeout_started + TIMEOUT) >= current_time
 *      nothing will be returned and a timeout is assumed
 * @param status_back Pointer to packet status of (maybe) received
 * packet, will be updated by function.
 *
 * @return packet in packet_t format
 */
packet_t* cc1200_get_packet(int device_id, clock_t timeout_started, packet_status_t *status_back);

//only for testing simulation. Could be removed later
void cc1200_debug_block_next_write(int device_id);
void cc1200_debug_corrupt_next_checksum(int device_id);

#endif //CC1200_RATE_H
