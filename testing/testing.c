/**
 * @file testing.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-10
 * 
 * @copyright Copyright (c) 2022
 * 
 * For crude testing
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "packet.h"

static void test_packet_build() {
    packet_t* pkt = packet_create(1, 2, 3, 4, 5, 6, 7, 8, 9);
    assert(pkt->id == 1);
    assert(pkt->next_symbol_rate == 2);
    assert(pkt->p_payload == 0x0);

    packet_destroy(pkt);
}

static void test_packet_destroy() {
    packet_destroy(NULL);    
}

static void test_packet_checksum() {
    packet_t* pkt = malloc(sizeof(packet_t));
    pkt->ack = 1;
    pkt->fallback_rate = 2;
    pkt->id = 65000;
    pkt->next_symbol_rate = 3;
    pkt->payload_len = 300;
    pkt->token_recv = 250;
    pkt->token_send = 400;
    pkt->type = 5;

    //mock payload data:
    pkt->p_payload = malloc(sizeof(uint8_t) * pkt->payload_len);
    for (int i=0;i<pkt->payload_len;i++) {
        pkt->p_payload[i] = 1;
    }
    packet_set_checksum(pkt);

    assert (pkt->checksum == (65961 + 300) % 256); //Manual calculated
    assert (pkt->checksum != packet_calc_checksum(pkt)-1);

    packet_destroy(pkt);

    pkt = malloc(sizeof(packet_t));
    pkt->ack = 1;
    pkt->fallback_rate = 1;
    pkt->id = 1;
    pkt->next_symbol_rate = 1;
    pkt->payload_len = 1;
    pkt->token_recv = 1;
    pkt->token_send = 1;
    pkt->type = 1;
    pkt->checksum = 0;
    pkt->p_payload = NULL;

    packet_set_checksum(pkt);
    
    assert (pkt->checksum == 8); //Manual calculated
    assert (pkt->checksum != packet_calc_checksum(pkt)-1);

    free(pkt);
}

//Helper function for test_packet_serialize()
static void testSerializedPacket(packet_t* pkt, uint8_t* serialized) {
    uint32_t count = 0;
    //build id from uint8_t
    uint32_t id = serialized[count] | serialized[count+1] << 8 | serialized[count+2] << 16 | serialized[count+3] << 24;
    count += 4;
    assert(id == pkt->id);

    uint8_t next_symbol_rate = serialized[count];
    count += 1;
    assert(next_symbol_rate == pkt->next_symbol_rate);

    uint16_t token_recv = serialized[count] | serialized[count+1] << 8;
    count += 2;
    assert(token_recv == pkt->token_recv);

    uint16_t token_send = serialized[count] | serialized[count+1] << 8;
    count += 2;
    assert(token_send == pkt->token_send);

    uint8_t type = serialized[count];
    count += 1;
    assert(type == pkt->type);

    uint32_t payload_len = serialized[count] | serialized[count+1] << 8 | serialized[count+2] << 16 | serialized[count+3] << 24;
    count += 4;
    assert(payload_len == pkt->payload_len);
    
    uint8_t checksum = serialized[count];
    count += 1;
    assert(checksum == pkt->checksum);

    uint32_t ack = serialized[count] | serialized[count+1] << 8 | serialized[count+2] << 16 | serialized[count+3] << 24;
    count += 4;
    assert(ack == pkt->ack);
    
    uint8_t fallback_rate = serialized[count];
    count += 1;
    assert(fallback_rate == pkt->fallback_rate);

    //check payload:
    for (int i=0;i<payload_len;i++) {
        assert(serialized[count + i] == pkt->p_payload[i]);
    }
}

static void test_packet_serialize_normal() {
    //"normal" case
    packet_t* pkt = malloc(sizeof(packet_t));
    pkt->ack = 1;
    pkt->fallback_rate = 2;
    pkt->id = 65000;
    pkt->next_symbol_rate = 3;
    pkt->payload_len = 300;
    pkt->token_recv = 250;
    pkt->token_send = 400;
    pkt->type = 5;

    //mock payload data:
    pkt->p_payload = malloc(sizeof(uint8_t) * pkt->payload_len);
    for (int i=0;i<pkt->payload_len;i++) {
        pkt->p_payload[i] = i % 256;
    }
    packet_set_checksum(pkt);

    uint32_t size = packet_get_size(pkt);
    uint8_t* serialized = calloc(sizeof(uint8_t), size);
    
    assert(packet_get_size(pkt) == (getHeaderSize() + pkt->payload_len) );

    packet_serialize(pkt, serialized);

    testSerializedPacket(pkt, serialized);

    packet_destroy(pkt);
}

static void test_packet_serialize_zero() {

    packet_t* pkt = malloc(sizeof(packet_t));
    pkt->ack = 0;
    pkt->fallback_rate = 0;
    pkt->id = 0;
    pkt->next_symbol_rate = 0;
    pkt->payload_len = 1;
    pkt->token_recv = 0;
    pkt->token_send = 0;
    pkt->type = 0;

    //mock payload data:
    pkt->p_payload = malloc(sizeof(uint8_t) * pkt->payload_len);
    for (int i=0;i<pkt->payload_len;i++) {
        pkt->p_payload[i] = i % 256;
    }
    packet_set_checksum(pkt);

    uint32_t size = packet_get_size(pkt);
    uint8_t* serialized = calloc(sizeof(uint8_t), size);
    
    assert(packet_get_size(pkt) == (getHeaderSize() + pkt->payload_len) );

    packet_serialize(pkt, serialized);

    testSerializedPacket(pkt, serialized);

    packet_destroy(pkt);
}

static void test_packet_serialize_maximum() {

    packet_t* pkt = malloc(sizeof(packet_t));
    pkt->ack = UINT32_MAX;
    pkt->fallback_rate = UINT8_MAX;
    pkt->id = UINT32_MAX;
    pkt->next_symbol_rate = UINT8_MAX;
    pkt->payload_len = 256;
    pkt->token_recv = UINT16_MAX;
    pkt->token_send = UINT16_MAX;
    pkt->type = UINT8_MAX;

    //mock payload data:
    pkt->p_payload = malloc(sizeof(uint8_t) * pkt->payload_len);
    for (int i=0;i<pkt->payload_len;i++) {
        pkt->p_payload[i] = i % 256;
    }
    packet_set_checksum(pkt);

    uint32_t size = packet_get_size(pkt);
    uint8_t* serialized = calloc(sizeof(uint8_t), size);
    
    assert(packet_get_size(pkt) == (getHeaderSize() + pkt->payload_len) );

    packet_serialize(pkt, serialized);
    
    testSerializedPacket(pkt, serialized);

    packet_destroy(pkt);
}

static void test_packet_deserialize_normal() {
    //same case as in serialize_normal
    uint8_t input[] = {0xe8, 0xfd, 0x0, 0x0, 0x3, 0xfa, 0x0, 0x90, 0x1, 0x5, 0x2c, 0x1, 0x0, 0x0, 0xdb, 0x1, 0x0, 0x0, 0x0, 0x2, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x0};
    uint32_t len = 321;
    packet_t *pkt = packet_deserialize(input);
    
    assert(pkt->id == 65000);
    assert(pkt->next_symbol_rate == 3);
    assert(pkt->token_recv == 250);
    assert(pkt->token_send == 400);
    assert(pkt->type == 5);
    assert(pkt->payload_len == 300);
    assert(pkt->checksum == 0xdb);
    assert(pkt->ack == 1);
    assert(pkt->fallback_rate == 2);

    //check content
    for (int i=0;i<pkt->payload_len;i++) {
        assert(pkt->p_payload[i] == input[getHeaderSize() - 1 + i]);
    }
}

static void test_packet_deserialize_maximum() {
    uint8_t input[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0x1, 0x0, 0x0, 0x79, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, 0x0};
    uint32_t len = 277;
    packet_t *pkt = packet_deserialize(input);
    
    assert(pkt->id == UINT32_MAX);
    assert(pkt->next_symbol_rate == UINT8_MAX);
    assert(pkt->token_recv == UINT16_MAX);
    assert(pkt->token_send == UINT16_MAX);
    assert(pkt->type == UINT8_MAX);
    assert(pkt->payload_len == 256);
    assert(pkt->checksum == 0x79);
    assert(pkt->ack == UINT32_MAX);
    assert(pkt->fallback_rate == UINT8_MAX);

    //check content
    for (int i=0;i<pkt->payload_len;i++) {
        assert(pkt->p_payload[i] == input[getHeaderSize() - 1 + i]);
    }
}


int main(int argc, char** argv) {
    if (argc < 2) { //there are no options
        printf("No options which packet should be tested in arguments\n");
        return 1;
    }
    for (int i=0;i<argc;i++) {
        printf("Test run[%i]: %s\n", i, argv[i]);
    }
    if (!strcmp(argv[1], "packet.minstrel.build")) {
        printf("Start minstrel build\n");
        test_packet_build();
    } else if (!strcmp(argv[1], "packet.minstrel.destroy"))  {
        printf("Start minstrel destroy\n");
        test_packet_destroy();
    } else if (!strcmp(argv[1], "packet.minstrel.checksum")) {
        printf("Start minstrel checksum\n");
        test_packet_checksum();
    } else if (!strcmp(argv[1], "packet.minstrel.serialize.normal")) {
        printf("Start minstrel serialize normal\n");
        test_packet_serialize_normal();
    } else if (!strcmp(argv[1], "packet.minstrel.serialize.zero")) {
        printf("Start minstrel serialize zero case\n");
        test_packet_serialize_zero();
    } else if (!strcmp(argv[1], "packet.minstrel.serialize.maximum")) {
        printf("Start minstrel serialize maximum case\n");
        test_packet_serialize_maximum();
    } else if (!strcmp(argv[1], "packet.minstrel.deserialize.normal")) {
        printf("Start minstrel deserialize normal case\n");
        test_packet_deserialize_normal();
    } else if (!strcmp(argv[1], "packet.minstrel.deserialize.maximum")) {
        printf("Start minstrel deserialize maximum case\n");
        test_packet_deserialize_maximum();
    } else {
        printf("No valid test command.\n");
        return 1;
    }

    return 0;
}