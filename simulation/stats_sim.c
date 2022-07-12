#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "stats_sim.h"


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

static void calc_ewma(MinstrelStatistics* statistics, float succ_prob) {
    statistics->ewma = (WEIGHT * succ_prob) + ((1 - WEIGHT) * statistics->ewma);
}

static void calc_throughput(MinstrelStatistics* statistics) {
    statistics->throughput = (statistics->ewma * statistics->bytes_send) / statistics->avg_duration;
}

static void calc_avg_duration(MinstrelStatistics* statistics, uint32_t duration) {
    statistics->avg_duration = ((statistics->avg_duration * statistics->pkt_count) + duration) / (statistics->pkt_count + 1);
}

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
    printf("succ_prob: %f\n", succ_prob);
    calc_ewma(statistics, succ_prob);
    calc_throughput(statistics);
}

static void update_rates(Minstrel* minstrel) {
    uint8_t best_rate_index, second_best_rate_index, highest_prob_index;
    float best_rate, second_best_rate, highest_prob;
    best_rate = second_best_rate = highest_prob = 0;

    for (int i=0; i<MAX_RATES; i++) {
        // Find highest success probability rate
        if (minstrel->statistics[i].ewma > highest_prob) {
            highest_prob = minstrel->statistics[i].ewma;
            highest_prob_index = i;
        }
        // Find best throughput rate
        if (minstrel->statistics[i].throughput > best_rate) {
            best_rate = minstrel->statistics[i].throughput;
            best_rate_index = i;
        }
        // Find 2nd best throughput rate
        if (minstrel->statistics[i].throughput > second_best_rate && minstrel->statistics[i].throughput < best_rate) {
            second_best_rate = minstrel->statistics[i].throughput;
            second_best_rate_index = i;
        }
    }

    // Update rates indices
    minstrel->rates.best = best_rate_index;
    minstrel->rates.second_best = second_best_rate_index;
    minstrel->rates.highest_prob = highest_prob_index;
    printf("best: %d\n", minstrel->rates.best);
    printf("second_best: %d\n", minstrel->rates.second_best);
    printf("highest prob: %d\n", minstrel->rates.highest_prob);
}

static void set_next_rate(Minstrel* minstrel, int is_probe) {
   if (is_probe)
       minstrel->rates.current = rand() % 10;
   else
       minstrel->rates.current = minstrel->rates.best;
}

void minstrel_update(Minstrel* minstrel, Packet* pkt) {
    int is_probe = 0;

    // Update statistics for the current rate
    log_package_status(&minstrel->statistics[minstrel->rates.current], pkt);

    // Only update rate every X packets TODO: Use time interval instead
    if ((pkt->id % 5) == 0) {
        puts("DEBUG: update rates");
        update_rates(minstrel);
    }

    // Send probe every X packets TODO: Use time interval instead
    if ((pkt->id % 10) == 0) {
        puts("DEBUG: is probe");
        is_probe = 1;
    }
    else
        puts("DEBUG: normal exchange");

    set_next_rate(minstrel, is_probe);
}



static void get_new_packet(Packet* packet) {
    int status_rand = rand() % 10;
    packet_status_t status = packet_status_ok_ack;
    if (status_rand == 9)
        status = packet_status_err_timeout;

    packet->status = status;
    packet->duration = rand() % 100 + 1;
    packet->id++;
}

static void summary(Minstrel* minstrel) {
    int rate_index = minstrel->rates.current;
    printf("rate: %d\n", MINSTREL_RATES[rate_index]);
    printf("** pkt_count: %d\n", minstrel->statistics[rate_index].pkt_count);
    printf("last_pkt_id: %d\n", minstrel->statistics[rate_index].last_pkt_id);
    printf("ewma: %f\n", minstrel->statistics[rate_index].ewma);
    printf("throughput: %f\n", minstrel->statistics[rate_index].throughput);
    printf("total_send: %d\n", minstrel->statistics[rate_index].total_send);
    printf("total_recv: %d\n", minstrel->statistics[rate_index].total_recv);
    printf("bytes_send: %d\n", minstrel->statistics[rate_index].bytes_send);
    printf("avg_duration: %d\n", minstrel->statistics[rate_index].avg_duration);
    puts("-------------------------------");
}


int main() {
    srand(time(NULL));

    Packet packet;
    packet.id = 0;
    packet.bytes_send = 50;
    packet.duration = 0;
    Minstrel* minstrel = minstrel_init();
    while(1) {
        printf("** current rate: %d\n", minstrel->rates.current);
        get_new_packet(&packet);
        printf("** packet id: %d\n", packet.id);
        printf("status: %s\n", packet.status==packet_status_ok_ack ? "ACK" : "TIMEOUT");
        minstrel_update(minstrel, &packet);
        summary(minstrel);
        sleep(1);
    }
}
