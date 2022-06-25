/**
 * @file cc1200_mockup.c
 * @author Luca Jungnickel
 * @brief 
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * 
 * Mockup file for cc1200_rate testing, only for testing purpose!
 * Should not be compiled in real application!
 * 
 * Replaces the cc1200_rate.c, so the code can be tested without 
 * accessing the CC1200, because it uses SPI and prusslibs, which
 * are not available on normal linux machines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "cc1200_rate.h"


void cc1200_change_rate(uint32_t rate) {

}

// TODO: Error handling + fixed length (?)
void cc1200_send_packet(uint8_t* buffer, uint32_t len) {
    
}

// TODO: Error handling + fixed length (?)
uint32_t cc1200_get_packet(uint8_t** buffer) {
    
}
