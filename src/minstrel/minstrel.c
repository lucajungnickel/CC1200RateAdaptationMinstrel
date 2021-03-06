#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minstrel.h"
#include "../cc1200/cc1200_rate.h"


uint32_t MINSTREL_RATES[] = {1000, 1200, 2400, 4800, 9600, 12500, 25000, 37500, 50000};

Minstrel* minstrel_init() {
    Minstrel* minstrel = malloc(sizeof(Minstrel));
    minstrel->is_new_rate = 0;
    // Init all rates with lowest possible rate
    minstrel->rates.fallback = 0;
    minstrel->rates.highest_prob = 0;
    minstrel->rates.second_best = 0;
    minstrel->rates.best = 0;
    minstrel->rates.current = 0;
    minstrel->rates.probe = 0;

    // Init statistics
    memset(minstrel->statistics, 0, sizeof(MinstrelStatistics) * MAX_RATES);

    minstrel->state = RUNNING;
    minstrel->rate_state = BEST_RATE;
    return minstrel;
}

/**
 * @brief Calculate the exponential moving average of the success probability for a given rate.
 *
 * @param minstrel
 * @param succ_prob The newly calculated success probability which should receive the highest weight (#send/#acks).
 */
static void calc_ewma(MinstrelStatistics* statistics, float succ_prob) {
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
static void log_package_status(MinstrelStatistics* statistics, minstrel_packet_t* pkt) {
    float succ_prob;

    statistics->total_send++;
    if (pkt->status == packet_status_ok) {
        statistics->total_recv++;
        // Has to be called before pkt_count is increased
        calc_avg_duration(statistics, pkt->duration);
        statistics->bytes_send = pkt->bytes_send;
        statistics->last_pkt_id = pkt->id;
        statistics->pkt_count++;
        // Update statistics
        succ_prob = (float) statistics->total_recv / statistics->total_send;
        calc_ewma(statistics, succ_prob);
        calc_throughput(statistics);
    }
}

/**
 * @brief Update best rate, second best rate and highest throughput rate of the minstrel algorithm.
 *
 * @param minstrel
 */
static void update_rates(Minstrel* minstrel) {
    uint8_t best_rate_index, second_best_rate_index, highest_prob_index;
    float best_rate, second_best_rate, highest_prob;
    best_rate_index = second_best_rate_index = highest_prob_index = 0;
    best_rate = second_best_rate = highest_prob = 0;

    for (int i=0; i<MAX_RATES; i++) {
        // Find highest success probability rate
        if (minstrel->statistics[i].ewma >= highest_prob && minstrel->statistics[i].ewma != 0) {
            highest_prob = minstrel->statistics[i].ewma;
            highest_prob_index = i;
        }
        // Find best throughput rate
        if (minstrel->statistics[i].throughput >= best_rate && minstrel->statistics[i].throughput != 0) {
            best_rate = minstrel->statistics[i].throughput;
            best_rate_index = i;
        }
    }
    // Find 2nd best throughput rate
    //for (int i=0; i<MAX_RATES; i++) {
      //  if (minstrel->statistics[i].throughput >= second_best_rate && minstrel->statistics[i].throughput < best_rate && i < best_rate_index && minstrel->statistics[i].throughput != 0) {
        //    second_best_rate = minstrel->statistics[i].throughput;
          //  second_best_rate_index = i;
        //}
    //}
    // Update rates indices
    minstrel->rates.best = best_rate_index;
    minstrel->rates.current = best_rate_index;
    minstrel->rates.second_best = best_rate_index == 0 ? 0 : (best_rate_index - 1);
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
static void set_probe_rate(Minstrel* minstrel) {
    uint8_t probe;
    // Pick a random rate not equal to the current one
    while ((probe = rand() % MAX_RATES) <= minstrel->rates.current);
    if (IS_DEBUG)
        printf("Probe rate: %d\n", probe);
    minstrel->rates.probe = probe;
}

uint8_t minstrel_get_next_rate(Minstrel* minstrel) {
    return minstrel->rates.r[minstrel->rate_state];
}

static RateState minstrel_state_to_rate_state(Minstrel *minstrel) {
    switch(minstrel->state) {
        case PROBE:
            set_probe_rate(minstrel);
            return PROBE_RATE;
         case PACKET_TIMEOUT:
             // Use the next lower rate according to the specification
             // i.e. best throughput -> second best throughput -> highest success probability -> fallback
             if (minstrel->rate_state != FALLBACK_RATE)
                 return minstrel->rate_state - 1;
             return FALLBACK_RATE;
         case UPDATE:
             return BEST_RATE;
         case RESET:
             return FALLBACK_RATE;
         case RUNNING:
         default:
             return CURRENT_RATE;
    }
}

static void summary(Minstrel* minstrel) {
    int rate_index = minstrel_get_next_rate(minstrel);
    puts("SUMMARY");
    printf("** rate: %d index: %d\n", MINSTREL_RATES[rate_index], rate_index);
    printf("** pkt_count: %d\n", minstrel->statistics[rate_index].pkt_count);
    printf("last_pkt_id: %d\n", minstrel->statistics[rate_index].last_pkt_id);
    printf("ewma: %f\n", minstrel->statistics[rate_index].ewma);
    printf("throughput: %f\n", minstrel->statistics[rate_index].throughput);
    printf("total_send: %d\n", minstrel->statistics[rate_index].total_send);
    printf("total_recv: %d\n", minstrel->statistics[rate_index].total_recv);
    printf("bytes_send: %d\n", minstrel->statistics[rate_index].bytes_send);
    printf("avg_duration: %d\n", minstrel->statistics[rate_index].avg_duration);
    puts("END");
}


int is_prob_ack;
int timeout_count;
// TODO: Reset minstrel->statistics's total_{send, recv} to avoid overflow
void minstrel_update(Minstrel* minstrel, minstrel_packet_t* pkt) {
    if (is_prob_ack) {
        is_prob_ack = 0;
        if (IS_DEBUG)
            puts("**************** SKIP ******************");
        return;
    }
    // Only update if last rate was a probe
    if (minstrel->state == PROBE) {
        update_rates(minstrel);
        minstrel->state = UPDATE;
    }
    // Reset state
    minstrel->state = RUNNING;
    // TODO: Improve minstrel_get_next_rate() usage here
    log_package_status(&minstrel->statistics[minstrel_get_next_rate(minstrel)], pkt);

    if (pkt->status != packet_status_ok) {
        if (timeout_count == 2) {
            minstrel->state = PACKET_TIMEOUT;
            timeout_count = 0;
        }
        else {
            timeout_count++;
            return;
        }
    }

    // Send probe every X packets
    // TODO: Use time interval instead
    // Note: If we got a packet timeout, we won't send a probe
    else if ((pkt->id % 10) == 0 && minstrel_get_next_rate(minstrel) != (MAX_RATES-1)) {
        minstrel->state = PROBE;
        is_prob_ack = 1;
    }

    // Set next rate
    uint8_t last_rate =  minstrel_get_next_rate(minstrel);
    minstrel->rate_state = minstrel_state_to_rate_state(minstrel);
    if (last_rate != minstrel_get_next_rate(minstrel))
        minstrel->is_new_rate = 1;
    else
        minstrel->is_new_rate = 0;

        if (IS_DEBUG) {
    summary(minstrel);
    printf("--- set rate to state: %d rate: %d\n", minstrel->rate_state, minstrel_get_next_rate(minstrel));
        puts("DEBUG: Updated rates.");
        printf("DEBUG: best: %d\n", minstrel->rates.best);
        printf("DEBUG: second_best: %d\n", minstrel->rates.second_best);
        printf("DEBUG: highest prob: %d\n", minstrel->rates.highest_prob);
}
}

void minstrel_destroy(Minstrel* minstrel) {
    if (minstrel == NULL) return;
    free(minstrel); //ATTENTION, ONLY FREE MINSTREL, NOT THE CONTENT
}
