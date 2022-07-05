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

#include "../src/log.c/src/log.h"

int main(int argc, char** argv) {
    if (argc < 2) { //there are no options
        printf("No options which packet should be tested in arguments\n");
        return 1;
    }

    log_info("Unit test is starting");
    
    bool test_all=false;
    for (int i=0;i<argc;i++) {
        printf("Test run[%i]: %s\n", i, argv[i]);
    }
    if (!strcmp(argv[1], "--all")) {
        printf("Test all cases\n");
        test_all = true;
    }
    if (!strcmp(argv[1], "packet.minstrel.build") || test_all) {
        printf("\nStart minstrel build\n\n");
        test_packet_build();
    }
    if (!strcmp(argv[1], "packet.minstrel.destroy") || test_all)  {
        printf("\nStart minstrel destroy\n\n");
        test_packet_destroy();
    }
    if (!strcmp(argv[1], "packet.minstrel.checksum") || test_all) {
        printf("\nStart minstrel checksum\n\n");
        test_packet_checksum();
    }
    if (!strcmp(argv[1], "packet.minstrel.serialize.normal") || test_all) {
        printf("\nStart minstrel serialize normal\n\n");
        test_packet_serialize_normal();
    }
    if (!strcmp(argv[1], "packet.minstrel.serialize.zero") || test_all) {
        printf("\nStart minstrel serialize zero case\n\n");
        test_packet_serialize_zero();
    }
    if (!strcmp(argv[1], "packet.minstrel.serialize.maximum") || test_all) {
        printf("\nStart minstrel serialize maximum case\n\n");
        test_packet_serialize_maximum();
    }
    if (!strcmp(argv[1], "packet.minstrel.deserialize.normal") || test_all) {
        printf("\nStart minstrel deserialize normal case\n\n");
        test_packet_deserialize_normal();
    } 
    if (!strcmp(argv[1], "packet.minstrel.deserialize.maximum") || test_all) {
        printf("\nStart minstrel deserialize maximum case\n\n");
        test_packet_deserialize_maximum();
    }
    if (!strcmp(argv[1], "communication.init") || test_all) {
        printf("\nStart communication init test\n\n");
        test_communication_initialization();
    }
    if (!strcmp(argv[1], "communication.sendOK.rcvOK.small") || test_all) {
        printf("\nStart communication small send ok rcv ok test\n\n");
        test_communication_send_ok_rcv_ok();
    }
    if (!strcmp(argv[1], "communication.handshake.sendFAIL") || test_all) {
        printf("\nStart communication handshake send fail and retry test\n\n");
        test_communication_send_error_handshake();
    }
    if (!strcmp(argv[1], "communication.handshake.ackFAIL") || test_all) {
        printf("\nStart communication handshake ACK fail and retry test\n\n");
        test_communication_handshake_ack_error();
    }
    if (!strcmp(argv[1], "communication.handshake.checksum.sendFAIL") || test_all) {
        printf("\nStart communication handshake checksum error\n\n");
        test_communication_send_wrong_checksum_error();
    }
    if (!strcmp(argv[1], "communication.handshake.checksum.ackFAIL") || test_all) {
        printf("\nStart communication handshake checksum in ack error\n\n");
        test_communication_send_wrong_checksum_ack_error();
    }

    return 0;
}