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
#include "../controller/packet.h"


#define MAX_REG  47
#define MAX_EXT_REG  130
// Register
#define SYMBOL_RATE2 0x13
#define SYMBOL_RATE1 0x14
#define SYMBOL_RATE0 0x15
#define PKT_CFG2 0x26
#define PKT_CFG1 0x27
#define PKT_CFG0 0x28
#define PKT_LEN 0x2E
#define REG_FIFO 0x3F
#define NUM_RXBYTES 0x2FD7
// Packet config
#define PKT_MODE 1 // Variable length mode
#define PKT_FORMAT 0 // FIFO mode/normal mode
#define PKT_MAX_LEN 20

#define PKT_OVERHEAD 2 // 2 bytes packet overhead

// Crystal frequency of the TI boards (40MHz)
#define F_XOSC 40000


typedef struct {
        int adr;  /* Register Adresse  */
        int val;  /* Wert des Registers */
} REG_TYPE;

REG_TYPE RegSettings[MAX_REG];

REG_TYPE ExtRegSettings[MAX_EXT_REG];

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

void cc1200_reset();

/**
 * @brief Initializes a cc1200 unit with a given id.
 * 
 * @param id Theoretically a MCU could have multiple CC1200 attached,
 * with this parameter you could choose which to initialize
 */
void cc1200_init(int id);

void cc1200_change_rate(uint32_t rate);

/**
 * @brief Sends a packet on the given device_id.
 *
 */
void cc1200_send_packet(int device_id, packet_t* packet);

/**
 * @brief Reads and returns a packet.
 * 
 * Blocking function
 * @param device_id Device id, can be ignored in the implementation.
 * Only implement if you would like to support multiple CC1200 on one
 * MCU.
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

#endif //CC1200_RATE_H
