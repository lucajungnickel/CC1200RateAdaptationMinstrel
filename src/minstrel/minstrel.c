#include <stdlib.h>
#include <string.h>

#include "minstrel.h"


uint32_t MINSTREL_RATES[] = {40, 250, 1200, 2400, 4800, 9600, 25000, 50000, 100000, 250000, 500000};

Minstrel* minstrel_init() {
    Minstrel* minstrel = malloc(sizeof(Minstrel));
    // Init all rates with lowest possible rate
    minstrel->rates.current = MINSTREL_RATES[0];
    minstrel->rates.second_best = MINSTREL_RATES[0];
    minstrel->rates.highest_prob = MINSTREL_RATES[0];

    // Init statistics
    memset(minstrel->statistics, 0, sizeof(minstrel->statistics));

    minstrel->state = READY;
    return minstrel;
}

void log_package_status(MinstrelStatistics* statistics, Packet* pkt) {
    statistics->total_send++;
    if (pkt->status == packet_status_ack)
        statistics->total_recv++;
    statistics->avg_duration = (statistics->avg_duration + pkt->duration) / statistics->total_send;
    statistics->last_pkt_id = pkt->id;
}


uint8_t minstrel_get_fallback_rate(Minstrel* minstrel) {
    // TODO: Maybe put this info into the Minstrel struct itself
    return 0;
}

void update_rates(Minstrel* minstrel) {
}

void set_next_rate(Minstrel* minstrel) {
}

// TODO: Update to fallback rate if timeout or error etc.
// TODO: Reset minstrel->pkt_info's total_{send, recv} to avoid overflow
void minstrel_update(Minstrel* minstrel, Packet* pkt) {
    uint8_t rate_index = minstrel->rates.current;

    log_package_status(&minstrel->statistics[rate_index], pkt);

    // Only update statistics every X packets TODO: Use time interval instead
    if ((pkt->id % 10) == 0) {
        uint32_t succ_prob = minstrel->statistics[rate_index].total_send / minstrel->statistics[rate_index].total_recv;
        calc_ewma(&minstrel->statistics[rate_index], succ_prob);
        calc_throughput(&minstrel->statistics[rate_index]);

        update_rates(minstrel);
    }
    if (minstrel->is_probe) {
        minstrel->is_probe = 0;
        set_next_rate(minstrel);
    }
    // Send probe every X packets TODO: Use time interval instead
    if ((pkt->id % 100) == 0) {
        minstrel->is_probe = 1;
        set_next_rate(minstrel);
    }
}

uint8_t minstrel_get_next_rate(Minstrel* minstrel) {
}

void calc_ewma(MinstrelStatistics* statistics, uint32_t succ_prob) {
    statistics->ewma = (WEIGHT * succ_prob) + ((1 - WEIGHT) * statistics->ewma);
}

void calc_throughput(MinstrelStatistics* statistics) {
    statistics->throughput = (statistics->ewma * statistics->bytes_send) / statistics->avg_duration;
}
