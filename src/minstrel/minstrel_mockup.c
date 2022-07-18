/**
 * @file minstrel_mockup.c
 * @author Luca Jungnickel
 * @date 2022-06-25
 * 
 * @brief Mockup file for replacing minstrel.c for test cases.
 * DO NOT COMPILE FOR PRODUCTIVE SYSTEM! USE minstrel.c INSTEAD
 */

#include <stdlib.h>

#include "../log.c/src/log.h"
#include "minstrel.h"


//uint32_t MINSTREL_RATES[] = {40, 250, 1200, 2400, 4800, 9600, 25000, 50000, 100000, 250000, 500000};
uint32_t MINSTREL_RATES[] = {1000, 1200, 2400, 4800, 9600, 12500, 25000, 37500, 50000};



Minstrel* minstrel_init() {
    Minstrel* back = calloc(1, sizeof(Minstrel));
    if (back == NULL) return NULL;
    back->statistics->pkt_count = 0;
    back->statistics->avg_duration = 1; //last success pkt
    return back;
}

/**
 * @brief Returns the next rate, which will be written to the next packet
 * of the sender.
 *
 * @param minstrel
 * @return uint8_t Index of next rate, @see MINSTREL_RATES[]
 */
uint8_t minstrel_get_next_rate(Minstrel* minstrel) {
    
    if (minstrel->statistics[0].avg_duration == 1) { //last pkt success
        //get next higher rate if possible, all 5 packets
        if (minstrel->statistics[0].pkt_count % 5 == 0) {
            if (minstrel->rates.current != (MAX_RATES-1) ) {
                minstrel->rates.current++;
            }
        }
    } else if (minstrel->statistics[0].avg_duration == 2) { //last pkt error
        //get lowest rate
        if (minstrel->rates.current != 0) {
            minstrel->rates.current = 0;
        }
    } else {
        if (!IS_IN_GRAPHIC_MODE) log_warn("Wrong average duration: %i", minstrel->statistics[0].avg_duration);
    }

    return minstrel->rates.current;
}

/**
 * @brief Prepares minstrel state for the next iteration. This includes the decision of whether we send a real packet or probe next (i.e. setting the symbol rate).
 *
 * @param minstrel
* @param pkt The packet for which new information should be incorporated into the algorithm.
 */
void minstrel_update(Minstrel* minstrel, minstrel_packet_t* pkt) {
    if (pkt->status == packet_status_ok || pkt->status == packet_status_ok_ack) {
        //avg_duration will be used for a flag, if the last packet is correct sent
        minstrel->statistics[0].avg_duration = 1;
    } else { //error
        minstrel->statistics[0].avg_duration = 2;
    }
    minstrel->statistics->pkt_count++;
}

/**
 * @brief Destroys the minstrel struct and all related data in it.
 *
 * @param minstrel
 */
void minstrel_destroy(Minstrel* minstrel) {
    free(minstrel);
}