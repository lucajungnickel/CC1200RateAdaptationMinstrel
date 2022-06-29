#include <stdlib.h>
#include <string.h>

#include "minstrel.h"


uint32_t MINSTREL_RATES[] = {40, 250, 1200, 2400, 4800, 9600, 25000, 50000, 100000, 250000, 500000};

Minstrel* minstrel_init() {
    Minstrel* minstrel = malloc(sizeof(Minstrel));
    // Init all rates with lowest possible rate
    minstrel->rates.current = 0;
    minstrel->rates.second_best = 0;
    minstrel->rates.highest_prob = 0;
    minstrel->rates.fallback = 0;

    // Init statistics
    memset(minstrel->statistics, 0, sizeof(minstrel->statistics));

    minstrel->state = READY;
    return minstrel;
}

/**
 * @brief Calculate the exponential moving average of the success probability for a given rate.
 *
 * @param minstrel
 * @param succ_prob The newly calculated success probability which should receive the highest weight (#send/#acks).
 */
static void calc_ewma(MinstrelStatistics* statistics, uint32_t succ_prob) {
    statistics->ewma = (WEIGHT * succ_prob) + ((1 - WEIGHT) * statistics->ewma);
}

/**
 * @brief Calculate the throughput of a given rate.
 *
 * @param statistics Statistics of a given rate for which the throughput should be updated.
 */
static void calc_throughput(MinstrelStatistics* statistics) {
    statistics->throughput = (statistics->ewma * statistics->bytes_send) / statistics->avg_duration;
}

/**
 * @brief Reports the package status of the given package to the algorithm
 *
 * @param statistics Statistics of a specific rate for the minstrel algorithm that should be updated.
 * @param pkt The packet for which new information should be incorporated into the algorithm.
 */
static void log_package_status(MinstrelStatistics* statistics, Packet* pkt) {
    statistics->total_send++;
    if (pkt->status == packet_status_ok_ack)
        statistics->total_recv++;
    statistics->avg_duration = (statistics->avg_duration + pkt->duration) / statistics->total_send;
    statistics->last_pkt_id = pkt->id;
}

/**
 * @brief Update best rate, second best rate and highest throughput rate of the minstrel algorithm.
 *
 * @param minstrel
 */
static void update_rates(Minstrel* minstrel) {
}

/**
 * @brief Set the next to-be-used rate (minstrel->rates.current) of the minstrel algorithm.
 *
 * @param minstrel
 * @param is_probe Whether the next rate is a probe or not.
 */
static void set_next_rate(Minstrel* minstrel, int is_probe) {
}

// TODO: Update to fallback rate if timeout or error etc.
// TODO: Reset minstrel->statistics's total_{send, recv} to avoid overflow
void minstrel_update(Minstrel* minstrel, Packet* pkt) {
    int is_probe = 0;
    uint8_t rate_index = minstrel->rates.current;

    log_package_status(&minstrel->statistics[rate_index], pkt);

    // Only update statistics every X packets TODO: Use time interval instead
    if ((pkt->id % 10) == 0) {
        uint32_t succ_prob = minstrel->statistics[rate_index].total_send / minstrel->statistics[rate_index].total_recv;
        calc_ewma(&minstrel->statistics[rate_index], succ_prob);
        calc_throughput(&minstrel->statistics[rate_index]);

        update_rates(minstrel);
    }
    // Send probe every X packets TODO: Use time interval instead
    if ((pkt->id % 100) == 0)
        is_probe = 1;

    set_next_rate(minstrel, is_probe);
}
