#include <stdlib.h>

#include "packet.h"

void test_packet_build();

void test_packet_destroy();

void test_packet_checksum();

//Helper function for test_packet_serialize()
void testSerializedPacket(packet_t* pkt, uint8_t* serialized);

void test_packet_serialize_normal();

void test_packet_serialize_zero();

void test_packet_serialize_maximum();

void test_packet_deserialize_normal();

void test_packet_deserialize_maximum();