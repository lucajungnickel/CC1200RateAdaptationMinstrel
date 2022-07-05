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
#include <stdbool.h>

#include "cc1200_rate.h"
#include "../controller/packet.h"



//Currently there are only two different CC1200 supported
static uint32_t shared_buffer_len_1 = 0;
static uint8_t* shared_buffer_1 = NULL;
static pthread_mutex_t shared_mutex_1 = PTHREAD_MUTEX_INITIALIZER;
static bool ignore_next_write_1 = false;
static bool corrupt_next_checksum_1 = false;

static uint32_t shared_buffer_len_2 = 0;
static uint8_t* shared_buffer_2 = NULL;
static pthread_mutex_t shared_mutex_2 = PTHREAD_MUTEX_INITIALIZER;
static bool ignore_next_write_2 = false;
static bool corrupt_next_checksum_2 = false;

const int TIMEOUT = 100;

static int id0 = 0;
static int id1 = 0;
static int numIdsSet = 0;
/**
 * @brief Only supports two device IDs at this time.
 */
void cc1200_init(int device_id) {
    if (numIdsSet == 0) {
        id0 = device_id;
    } else if(numIdsSet == 1) {
        id1 = device_id;
    } else {
        printf("CC1200 Mockup Warning: Too many ids set.\n");
    }
    numIdsSet++;
    printf("CC1200 Mockup init %i\n", device_id);
}

void cc1200_reset(int device_id) {
    free(shared_buffer_1);
    free(shared_buffer_2);
    shared_buffer_len_1 = 0;
    shared_buffer_1 = NULL;
    shared_buffer_len_2 = 0;
    shared_buffer_2 = NULL;
}

void cc1200_change_rate(int device_id, uint8_t rate) {

}

cc1200_status_send cc1200_send_packet(int device_id, packet_t* packet) {
    if (packet == NULL) printf("CC1200 Mockup: Warning send packet packet=NULL\n");
    
    //writes to shared memory for simulation
    uint8_t *buffer = malloc(packet_get_size(packet) * sizeof(uint8_t));
    packet_serialize(packet, buffer);
    
    if (device_id == id0) {
        if (!ignore_next_write_1) {
            if (corrupt_next_checksum_1) { //for debugging
                printf("CC1200 Mockup: Corrupt next checksum sending on device %i\n", device_id);
                uint8_t original_checksum = packet->checksum;
                packet->checksum = (packet->checksum +1) % 256;
                printf("CC1200 Mockup: new checksum: %i\n", packet->checksum);
                packet_serialize(packet, buffer); //write packet again
                packet->checksum = original_checksum;
                corrupt_next_checksum_1 = false;
            }
            pthread_mutex_lock(&shared_mutex_1);
            free(shared_buffer_1); //first free old alloced data
            shared_buffer_1 = buffer;
            shared_buffer_len_1 = packet_get_size(packet);
            pthread_mutex_unlock(&shared_mutex_1);
            printf("CC1200 Mockup: Wrote packet to buffer %i\n", device_id);
            printf("CC1200 Mockup: Buffer %i size %i\n", device_id, shared_buffer_len_1);
        } else {
            ignore_next_write_1 = false;
            free(buffer);
            printf("CC1200 Mockup: Ignored next write to buffer %i\n", device_id);
        }

    } else if (device_id == id1) {
        if (!ignore_next_write_2) {
            if (corrupt_next_checksum_2) { //for debugging
                printf("CC1200 Mockup: Corrupt next checksum sending on device %i\n", device_id);
                uint8_t original_checksum = packet->checksum;
                packet->checksum = (packet->checksum +1) % 256;
                printf("CC1200 Mockup: new checksum: %i\n", packet->checksum);
                packet_serialize(packet, buffer); //write packet again
                packet->checksum = original_checksum;
                corrupt_next_checksum_2 = false;
            }
            pthread_mutex_lock(&shared_mutex_2);
            free(shared_buffer_2); //first free old alloced data
            shared_buffer_2 = buffer;
            shared_buffer_len_2 = packet_get_size(packet);
            pthread_mutex_unlock(&shared_mutex_2);
            printf("CC1200 Mockup: Wrote packet to buffer %i\n", device_id);
            printf("CC1200 Mockup: Buffer %i size %i\n", device_id, shared_buffer_len_2);
        } else {
            ignore_next_write_2 = false;
            free(buffer);
            printf("CC1200 Mockup: Ignored next write to buffer %i\n", device_id);
        }
    } else {
        printf("CC1200 Mockup Warning, wrong device id\n");
    }
    printf("CC1200 Mockup: Sending done\n");
    return cc1200_status_send_ok;
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
            *status_back = packet_status_err_timeout;
            return NULL;
        }

        if (device_id == id0) {
            pthread_mutex_lock(&shared_mutex_1);
            if (shared_buffer_len_1 != 0) { //there is data
                data = shared_buffer_1;
                data_len = shared_buffer_len_1;
                shared_buffer_len_1 = 0; 
                break;
            }
            pthread_mutex_unlock(&shared_mutex_1);       

            } else if (device_id == id1) {
            pthread_mutex_lock(&shared_mutex_2);
            if (shared_buffer_len_2 != 0) { //there is data
                data = shared_buffer_2;
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
    //FREE and set pointer to NULL
    free(data);
    if (device_id == id0) {
        shared_buffer_1 = NULL;
    } else if (device_id == id1) {
        shared_buffer_2 = NULL;
    } else {
        printf("CC1200 Mockup Warning, wrong device ID\n");
    }
    return back;
}

/**
 * @brief Debugging function, will ignore the next write to the 
 * given device buffer.
 * 
 */
void cc1200_debug_block_next_write(int device_id) {
    if (device_id == id0) {
        ignore_next_write_1 = true;
    } else if (device_id == id1) {
        ignore_next_write_2 = true;
    } else {
        printf("CC1200 Mockup warning: invalid device number\n");
    }
}

/**
 * @brief Corrupts next checksum in sending in next packet.
 * ONLY FOR TESTING PURPOSE
 * 
 */
void cc1200_debug_corrupt_next_checksum(int device_id) {
if (device_id == id0) {
        corrupt_next_checksum_1 = true;
    } else if (device_id == id1) {
        corrupt_next_checksum_2 = true;
    } else {
        printf("CC1200 Mockup warning: invalid device number\n");
    }    
}
