/**
 * @file cc1200_rate.h
 * @author Jannis
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
#include "packet.h"

// Regs
#define SYMBOL_RATE2 0x13 
#define SYMBOL_RATE1 0x14 
#define SYMBOL_RATE0 0x15
#define REG_FIFO 0x3F
#define NUM_RXBYTES 0x2FD7

#define PKT_OVERHEAD 2 // 2 bytes packet overhead

// Crystal frequency of the TI boards (40MHz)
#define F_XOSC 40000



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
const int PACKET_TIMEOUT;

void cc1200_reset();

void cc1200_init();

void cc1200_change_rate(uint32_t rate);

/**
 * @brief Sends a packet.
 * 
 */
void cc1200_send_packet(packet_t* packet);

/**
 * @brief Reads and returns a packet.
 * 
 * Blocking function
 * 
 * @param timeout_started Time the timeout counter started.
 *  If (timeout_started + PACKET_TIMEOUT) > current_time
 *      nothing will be returned and a timeout is assumed
 * @param status_back Pointer to packet status of (maybe) received 
 * packet, will be updated by function.
 * 
 * @return packet in packet_t format
 */
packet_t* cc1200_get_packet(clock_t timeout_started, packet_status_t *status_back);

#endif //CC1200_RATE_H
