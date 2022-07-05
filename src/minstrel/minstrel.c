#include <stdlib.h>
#include <string.h>

#include "minstrel.h"


uint32_t MINSTREL_RATES[] = {40, 250, 1200, 2400, 4800, 9600, 25000, 50000, 100000, 250000, 500000};

Minstrel* minstrel_init() {
    Minstrel* minstrel = malloc(sizeof(Minstrel));
    // Init all rates with lowest possible rate
    minstrel->rates.current = 0;
    minstrel->rates.best = 0;
    minstrel->rates.second_best = 0;
    minstrel->rates.highest_prob = 0;
    minstrel->rates.fallback = 0;

    // Init statistics
    memset(minstrel->statistics, 0, sizeof(MinstrelStatistics));

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
 * @brief Calculate the throughput for a given rate.
 *
 * @param statistics Statistics of a given rate for which the throughput should be updated.
 */
static void calc_throughput(MinstrelStatistics* statistics) {
    statistics->throughput = (statistics->ewma * statistics->bytes_send) / statistics->avg_duration;
}

/**
 * @brief Calculate the average duration (arithmetic mean) of a packet send -> ack received cycle for a given rate.
 *
 * @param statistics Statistics of a given rate for which the average duration should be updated.
 * @param duration Duration of the last packet.
 */
static void calc_avg_duration(MinstrelStatistics* statistics, uint32_t duration) {
    statistics->avg_duration = ((statistics->avg_duration * statistics->pkt_count) + duration) / (statistics->pkt_count + 1);
}

/**
 * @brief Reports the package status of the given package to the algorithm
 *
 * @param statistics Statistics of a specific rate for the minstrel algorithm that should be updated.
 * @param pkt The packet for which new information should be incorporated into the algorithm.
 */
// TODO: Reset packet count every X
static void log_package_status(MinstrelStatistics* statistics, Packet* pkt) {
    float succ_prob;

    statistics->total_send++;
    if (pkt->status == packet_status_ok_ack)
        statistics->total_recv++;
    // Has to be called before pkt_count is set
    calc_avg_duration(statistics, pkt->duration);
    statistics->bytes_send = pkt->bytes_send;
    statistics->last_pkt_id = pkt->id;
    statistics->pkt_count++;
    // Update statistics
    succ_prob = (float) statistics->total_recv / statistics->total_send;
    calc_ewma(statistics, succ_prob);
    calc_throughput(statistics);
}

/**
 * @brief Update best rate, second best rate and highest throughput rate of the minstrel algorithm.
 *
 * @param minstrel
 */
static void update_rates(Minstrel* minstrel) {
    uint8_t best_rate_index, second_best_rate_index, highest_prob_index;
    float best_rate, second_best_rate, highest_prob;
    best_rate = second_best_rate = highest_prob = 0;

    // Find highest success probability rate
    for (int i=0; i<MAX_RATES; i++) {
        if (minstrel->statistics[i].ewma > highest_prob) {
            highest_prob = minstrel->statistics[i].ewma;
            highest_prob_index = i;
        }
    }

    // Find 1st and 2nd best throughput rate
    for (int i=0; i<MAX_RATES; i++) {
        if (minstrel->statistics[i].throughput > best_rate) {
            best_rate = minstrel->statistics[i].throughput;
            best_rate_index = i;
        }
        if (minstrel->statistics[i].throughput > second_best_rate && minstrel->statistics[i].throughput < best_rate) {
            second_best_rate = minstrel->statistics[i].throughput;
            second_best_rate_index = i;
        }
    }
    // Update rates indices
    minstrel->rates.best = best_rate_index;
    minstrel->rates.second_best = second_best_rate_index;
    minstrel->rates.highest_prob = highest_prob_index;

    if (IS_DEBUG) {
        puts("DEBUG: Updated rates.");
        printf("DEBUG: best: %d\n", minstrel->rates.best);
        printf("DEBUG: second_best: %d\n", minstrel->rates.second_best);
        printf("DEBUG: highest prob: %d\n", minstrel->rates.highest_prob);
    }
}

/**
 * @brief Set the next to-be-used rate (minstrel->rates.current) of the minstrel algorithm.
 *
 * @param minstrel
 * @param is_probe Whether the next rate is a probe or not.
 */
static void set_next_rate(Minstrel* minstrel, int is_probe) {
   if (is_probe)
       // TODO: Make sure random index != current
       minstrel->rates.current = rand() % 10;
   else
       minstrel->rates.current = minstrel->rates.best;
}

// TODO: Update to fallback rate if timeout or error etc.
// TODO: Reset minstrel->statistics's total_{send, recv} to avoid overflow
void minstrel_update(Minstrel* minstrel, Packet* pkt) {
    int is_probe = 0;

    // Update statistics for the current rate
    log_package_status(&minstrel->statistics[minstrel->rates.current], pkt);

    // Only update rate every X packets TODO: Use time interval instead
    if ((pkt->id % 5) == 0)
        update_rates(minstrel);

    // Send probe every X packets TODO: Use time interval instead
    if ((pkt->id % 10) == 0)
        is_probe = 1;
    else
        puts("DEBUG: normal exchange");

    set_next_rate(minstrel, is_probe);
}

void minstrel_destroy(Minstrel* minstrel) {
    if (minstrel == NULL) return;
    free(minstrel); //ATTENTION, ONLY FREE MINSTREL, NOT THE CONTENT
}