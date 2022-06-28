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
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "cc1200_rate.h"
#include "../controller/packet.h"



//Currently there are only two different CC1200 supported
uint32_t shared_buffer_len_1;
uint8_t* shared_buffer_1;
pthread_mutex_t shared_mutex_1 = PTHREAD_MUTEX_INITIALIZER;

uint32_t shared_buffer_len_2;
uint8_t* shared_buffer_2;
pthread_mutex_t shared_mutex_2 = PTHREAD_MUTEX_INITIALIZER;


const int TIMEOUT = 100;

static int id0 = 0;
static int id1 = 0;
static int numIdsSet = 0;
/**
 * @brief Only supports two device IDs at this time.
 */
void cc1200_init(int id) {
    if (numIdsSet == 0) {
        id0 = id;
    } else if(numIdsSet == 1) {
        id1 = id;
    } else {
        printf("CC1200 Mockup Warning: Too many ids set.\n");
    }
    numIdsSet++;
    printf("CC1200 Mockup init %i\n", id);
}

void cc1200_reset() {
    shared_buffer_len_1 = 0;
    shared_buffer_1 = NULL;
    shared_buffer_len_2 = 0;
    shared_buffer_2 = NULL;

    //shared_mutex = PTHREAD_MUTEX_INITIALIZER;
}

void cc1200_change_rate(uint32_t rate) {

}


void cc1200_send_packet(int device_id, packet_t* packet) {
    if (packet == NULL) printf("CC1200 Mockup: Warning send packet packet=NULL\n");
    //writes to shared memory for simulation
    uint8_t *buffer = malloc(packet_get_size(packet) * sizeof(uint8_t));
    packet_serialize(packet, buffer);
    
    if (device_id == id0) {
        pthread_mutex_lock(&shared_mutex_1);
        shared_buffer_1 = buffer;
        shared_buffer_len_1 = packet_get_size(packet);
        pthread_mutex_unlock(&shared_mutex_1);
        printf("CC1200 Mockup: Wrote packet to buffer %i\n", device_id);
        printf("CC1200 Mockup: Buffer %i size %i\n", device_id, shared_buffer_len_1);
    } else if (device_id == id1) {
        pthread_mutex_lock(&shared_mutex_2);
        shared_buffer_2 = buffer;
        shared_buffer_len_2 = packet_get_size(packet);
        pthread_mutex_unlock(&shared_mutex_2);
        printf("CC1200 Mockup: Wrote packet to buffer %i\n", device_id);
        printf("CC1200 Mockup: Buffer %i size %i\n", device_id, shared_buffer_len_2);
    } else {
        printf("CC1200 Mockup Warning, wrong device id\n");
    }
    
}

packet_t* cc1200_get_packet(int device_id, clock_t timeout_started, packet_status_t *status_back) {
    //polls shared memory and checks if there is data to 'receive'
    
    uint8_t *data = 0;
    uint32_t data_len = 0;

    while (1) {
        sleep(0.001);
        //check for timeout
        clock_t time_d = clock() - timeout_started;
        int msec = time_d * 1000 / CLOCKS_PER_SEC;
        if (msec >= TIMEOUT) {
            printf("Timeout, read on %i\n", device_id);
            *status_back = packet_status_timeout;
            return NULL;
        }

        if (device_id == id0) {
            pthread_mutex_lock(&shared_mutex_1);
            if (shared_buffer_len_1 != 0) { //there is data
                data = shared_buffer_1;
                shared_buffer_1 = 0;
                data_len = shared_buffer_len_1;
                shared_buffer_len_1 = 0; 
                break;
            }
            pthread_mutex_unlock(&shared_mutex_1);       

            } else if (device_id == id1) {
            pthread_mutex_lock(&shared_mutex_2);
            if (shared_buffer_len_2 != 0) { //there is data
                data = shared_buffer_2;
                shared_buffer_2 = 0;
                data_len = shared_buffer_len_2;
                shared_buffer_len_2 = 0; 
                break;
            }
            pthread_mutex_unlock(&shared_mutex_2);       

        } else {
            printf("CC1200 Mockup Warning, wrong device ID\n");
        }

    }
    if (device_id == id0) {
        pthread_mutex_unlock(&shared_mutex_1);
    } else if (device_id == id1) {
        pthread_mutex_unlock(&shared_mutex_2);
    } else {
        printf("CC1200 Mockup Warning, wrong device ID\n");
    }
    printf("CC1200 Mockup: read data, device id %i, size %i\n", device_id, data_len);
    //now we got data
    *status_back = packet_status_ok;
    packet_t *back = packet_deserialize(data);
    return back;
}




