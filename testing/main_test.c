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

#include "test_controller_packet.h"
#include "test_controller_communication.h"

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
    } else if (!strcmp(argv[1], "communication.sendOK.rcvOK.small")) {
        printf("Start communication small send ok rcv ok test\n");
        test_communication_send_ok_rcv_ok();
    } else {
        printf("No valid test command.\n");
        return 1;
    }

    return 0;
}