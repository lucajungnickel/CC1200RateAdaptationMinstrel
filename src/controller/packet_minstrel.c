/**
 * @file packet_minstrel.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-06-18
 * 
 * @copyright Copyright (c) 2022
 * 
 * Defines a packet for the minstrel algorithm, with the correct header and a custom payload.
 */

#include "packet_minstrel.h"

#include <stdlib.h>
#include <string.h>

packet_minstrel_t* packet_minstrel_create (
    uint32_t const id,
    uint8_t  const next_symbol_rate,
    uint16_t  const token_recv,
    uint16_t  const token_send,
    uint8_t  const type,
    uint32_t  const payload_len,
    uint8_t  const checksum,
    uint32_t const ack,
    uint8_t  const fallback_rate) {

        packet_minstrel_t* back = malloc(sizeof(packet_minstrel_t));
        back->id = id;
        back->next_symbol_rate = next_symbol_rate;
        back->token_recv = token_recv;
        back->token_send = token_send;
        back->type = type;
        back->payload_len = payload_len;
        back->checksum = checksum;
        back->ack = ack;
        back->fallback_rate = fallback_rate;
        back->p_payload = 0x0;
        return back;
    }

/**
 * @brief Destroys the packet and also frees the allocated memory in payload data.
 * 
 */
void packet_minstrel_destroy(packet_minstrel_t* const packet) {
    if (packet != NULL) {
        free(packet->p_payload);
    }
    free(packet);
}


uint8_t packet_minstrel_calc_checksum(packet_minstrel_t* const packet) {
    //Uses Sum of Bytes % 256
    uint32_t checksum = 0;

    checksum += packet->id;
    checksum = checksum % 256;
    
    checksum += packet->next_symbol_rate;
    checksum = checksum % 256;

    checksum += packet->token_recv;
    checksum = checksum % 256;

    checksum += packet->token_send;
    checksum = checksum % 256;

    checksum += packet->type;
    checksum = checksum % 256;

    checksum += packet->payload_len;
    checksum = checksum % 256;

    checksum += packet->ack;
    checksum = checksum % 256;

    checksum += packet->fallback_rate;
    checksum = checksum % 256;

    if (packet->p_payload != NULL) {
        for (int i=0; i<packet->payload_len; i++) {
            checksum += packet->p_payload[i];
            checksum = checksum % 256;            
        }
    }
    
    return checksum;
}

void packet_minstrel_set_checksum(packet_minstrel_t* const packet) {
    uint8_t checksum = packet_minstrel_calc_checksum(packet);
    packet->checksum = checksum;
}

uint8_t packet_minstrel_serialize(packet_minstrel_t* const packet, uint8_t* p_buffer) {
    uint32_t index = 0;
    if (packet == NULL || p_buffer == NULL) return 1;
    
    //write content:
    p_buffer[index] = packet->id;
    p_buffer[index + 1] = packet->id >> 8;
    p_buffer[index + 2] = packet->id >> 16;
    p_buffer[index + 3] = packet->id >> 24;
    index += 4; //size in bytes
    p_buffer[index] = packet->next_symbol_rate;
    index += 1;
    p_buffer[index] = packet->token_recv;
    p_buffer[index + 1] = packet->token_recv >> 8;
    index += 2;
    p_buffer[index] = packet->token_send;
    p_buffer[index + 1] = packet->token_send >> 8;
    index += 2;
    p_buffer[index] = packet->type;
    index += 1;
    p_buffer[index] = packet->payload_len;
    p_buffer[index + 1] = packet->payload_len >> 8;
    p_buffer[index + 2] = packet->payload_len >> 16;
    p_buffer[index + 3] = packet->payload_len >> 24;
    index += 4;
    p_buffer[index] = packet->checksum;
    index += 1;
    p_buffer[index] = packet->ack;
    p_buffer[index + 1] = packet->ack >> 8;
    p_buffer[index + 2] = packet->ack >> 16;
    p_buffer[index + 3] = packet->ack >> 24;
    index += 4;
    p_buffer[index] = packet->fallback_rate;
    index += 1;    

    //write payload
    memcpy(p_buffer + index, packet->p_payload, packet->payload_len);

    return 0;
}

packet_minstrel_t* packet_minstrel_deserialize(uint8_t* const p_buffer) {
    packet_minstrel_t* back = malloc(sizeof(packet_minstrel_t));
    if (back == NULL) return NULL;

    //read header
    uint32_t header_size = getHeaderSize();
    uint32_t index = 0;
    //id
    back->id = p_buffer[index] | p_buffer[index + 1] << 8 | p_buffer[index + 2] << 16 | p_buffer[index + 3] << 24;
    index += 4;

    //next_symbol_rate
    back->next_symbol_rate = p_buffer[index];
    index += 1;

    //token_recv
    back->token_recv = p_buffer[index] | p_buffer[index + 1] << 8;
    index +=2;
    //token_send
    back->token_send = p_buffer[index] | p_buffer[index + 1] << 8;
    index +=2;
    //type
    back->type = p_buffer[index];
    index += 1;
    //payload_len
    back->payload_len = p_buffer[index] | p_buffer[index + 1] << 8 | p_buffer[index + 2] << 16 | p_buffer[index + 3] << 24;
    index += 4;
    //checksum
    back->checksum = p_buffer[index];
    index += 1;
    //ack
    back->ack = p_buffer[index] | p_buffer[index + 1] << 8 | p_buffer[index + 2] << 16 | p_buffer[index + 3] << 24;
    index += 4;
    //fallback_rate
    back->fallback_rate = p_buffer[index];
    index += 1;

    //BODY
    //payload
    back->p_payload = malloc(back->payload_len * sizeof(uint8_t));
    memcpy(back->p_payload, p_buffer + index, back->payload_len);

    return back;
}

uint32_t packet_minstrel_get_size(packet_minstrel_t* const packet) {
    return getHeaderSize() + packet->payload_len;
}


uint32_t getHeaderSize() {
    return 21;
}
