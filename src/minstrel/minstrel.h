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

/*
 * Possible symbol rates for the algorithm.
 * Unit in sps, NOT ksps.
 */
uint32_t MINSTREL_RATES[11];

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
    uint8_t second_best;
    uint8_t highest_prob;
} AvailableRates;

/*
 * TODO
 */
typedef struct MinstrelStatistics {
    uint32_t ewma;
    uint32_t pending_packets;
    uint32_t received_packets;
} MinstrelStatistics;

/*
 * State of the minstrel algorithm.
 */
typedef struct Minstrel {
    AvailableRates rates;
    MinstrelState state;
    MinstrelStatistics statistics;
} Minstrel;

/**
 * @brief Inits the algorithm
 */
Minstrel* minstrel_init();

/**
 * @brief Reports the package status of the given package to the algorithm
 * 
 * @param id ID of the package
 * @param status Use PACKET_LOST, PACKET_RECV for reporting
 */
void minstrel_log_package_status(uint8_t id, packet_status_t status);

/**
 * @brief Gets index of the next fallback rate, which will be written
 * to the next packet.
 * 
 */
uint8_t minstrel_get_fallback_rate(Minstrel* minstrel);

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
 */
void minstrel_update(Minstrel* minstrel);

#endif //MINSTREL_H