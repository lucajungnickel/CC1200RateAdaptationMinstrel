/**
 * @file minstrel.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-06-18
 *
 * @copyright Copyright (c) 2022
 *
 * Main minstrel algorithm.
 */
#ifndef MINSTREL_H
#define MINSTREL_H

#include <stdint.h>
#include "../controller/packet.h"


#define MAX_RATES 11
// EWMA alpha value
#define WEIGHT 0.75

/*
 * Possible symbol rates for the algorithm.
 * Unit in sps, NOT ksps.
 * MINSTREL_RATES[0] is the lowest possible symbol rate, then it is incrementing.
 */
uint32_t MINSTREL_RATES[MAX_RATES];

/*
 * Possible states of the algorithm.
 */
typedef enum MinstrelState {
    READY,
    RESET,
    RUNNNING,
    STOPPED,
    DONE,
} MinstrelState;

/*
 * Available rates for the algorithm, fields are an index into MINSTREL_RATES.
 */
typedef struct AvailableRates {
    uint8_t current;
    uint8_t best;
    uint8_t second_best;
    uint8_t highest_prob;
    uint8_t fallback;
} AvailableRates;

/*
 * Statistics of the minstrel algorithm.
 */
typedef struct MinstrelStatistics {
    uint32_t last_pkt_id;
    uint32_t pkt_count;
    float ewma; // Success probability
    float throughput;
    uint32_t total_send;
    uint32_t total_recv;
    uint32_t bytes_send;
    uint32_t avg_duration;
} MinstrelStatistics;

/*
 * State of the minstrel algorithm.
 */
typedef struct Minstrel {
    //ATTENTION, no pointers here, because the content are only integers.
    AvailableRates rates;
    MinstrelState state;
    MinstrelStatistics statistics[MAX_RATES];
} Minstrel;

/*
 * A packet for the minstrel algorithm.
 */
typedef struct Packet {
    uint32_t id;
    packet_status_t status;
    uint32_t bytes_send;
    uint32_t duration;

} Packet;

/**
 * @brief Inits the algorithm
 */
Minstrel* minstrel_init();

/**
 * @brief Returns the next rate, which will be written to the next packet
 * of the sender.
 *
 * @param minstrel
 * @return uint8_t Index of next rate, @see MINSTREL_RATES[]
 */
uint8_t minstrel_get_next_rate(Minstrel* minstrel);

/**
 * @brief Prepares minstrel state for the next iteration. This includes the decision of whether we send a real packet or probe next (i.e. setting the symbol rate).
 *
 * @param minstrel
* @param pkt The packet for which new information should be incorporated into the algorithm.
 */
void minstrel_update(Minstrel* minstrel, Packet* pkt);

/**
 * @brief Destroys the minstrel struct and all related data in it.
 * 
 * @param minstrel 
 */
void minstrel_destroy(Minstrel* minstrel);

#endif //MINSTREL_H
