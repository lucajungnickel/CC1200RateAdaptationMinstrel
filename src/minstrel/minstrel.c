#include <stdlib.h>

#include "minstrel.h"


uint32_t MINSTREL_RATES[] = {40, 250, 1200, 2400, 4800, 9600, 25000, 50000, 100000, 250000, 500000};

Minstrel* minstrel_init() {
    Minstrel* minstrel = malloc(sizeof(Minstrel));
    // Init all rates with lowest possible rate
    minstrel->rates.current = MINSTREL_RATES[0];
    minstrel->rates.second_best = MINSTREL_RATES[0];
    minstrel->rates.highest_prob = MINSTREL_RATES[0];

    minstrel->state = READY;
    return minstrel;
}

void minstrel_log_package_status(uint8_t id, package_status_t status) {

}

uint8_t minstrel_get_fallback_rate() {
    // TODO: Maybe put this info into the Minstrel struct itself
    return 0;
}

void minstrel_update(Minstrel* minstrel) {
    // TODO
}
