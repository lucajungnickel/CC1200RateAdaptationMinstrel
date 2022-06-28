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
 * SHOULD NOT BE COMPILED IN REAL APPLICATION!
 * 
 * Replaces the cc1200_rate.c, so the code can be tested without 
 * accessing the CC1200, because it uses SPI and prusslibs, which
 * are not available on normal linux machines.
 */

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "cc1200_rate.h"
#include "../controller/packet.h"

uint32_t shared_buffer_len;
uint8_t* shared_buffer;
pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;

void cc1200_reset() {
    shared_buffer_len = 0;
    shared_buffer = NULL;
    //shared_mutex = PTHREAD_MUTEX_INITIALIZER;
}

void cc1200_init() {

}

void cc1200_change_rate(uint32_t rate) {

}


void cc1200_send_packet(packet_t* packet) {
    //writes to shared memory for simulation
    uint8_t *buffer = malloc(packet_get_size(packet) * sizeof(uint8_t));
    packet_serialize(packet, buffer);
    
    pthread_mutex_lock(&shared_mutex);
    shared_buffer = buffer;
    shared_buffer_len = packet_get_size(packet);
    pthread_mutex_unlock(&shared_mutex);


}

packet_t* cc1200_get_packet() {
    //polls shared memory and checks if there is data to 'receive'
    
    uint8_t *data = 0;
    uint32_t data_len = 0;

    while (1) {
        sleep(0.001);
        pthread_mutex_lock(&shared_mutex);
        if (shared_buffer_len != 0) { //there is data
            data = shared_buffer;
            shared_buffer = 0;
            data_len = shared_buffer_len;
            shared_buffer_len = 0; 
            break;
        }
        pthread_mutex_unlock(&shared_mutex);       
    }
    pthread_mutex_unlock(&shared_mutex);  

    //now we got data
    packet_t *back = packet_deserialize(data);
    return back;
}




