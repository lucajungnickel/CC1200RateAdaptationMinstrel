/**
 * @file minstrel_mockup.c
 * @author Luca Jungnickel
 * @date 2022-06-25
 * 
 * @brief Mockup file for replacing minstrel.c for test cases.
 * DO NOT COMPILE FOR PRODUCTIVE SYSTEM! USE minstrel.c INSTEAD
 */

#include <stdlib.h>

#include "minstrel.h"


uint32_t MINSTREL_RATES[MAX_RATES];




Minstrel* minstrel_init() {

}

void minstrel_log_package_status(uint8_t id, packet_status_t status) {

}


void minstrel_update(Minstrel* minstrel, Packet* pkt) {

}


uint8_t minstrel_get_next_rate(Minstrel* minstrel) {
    
}

void minstrel_destroy(Minstrel* minstrel) {
    if (minstrel == NULL) return;
    free(minstrel); //ATTENTION, ONLY FREE MINSTREL, NOT THE CONTENT
}