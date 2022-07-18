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
#include <string.h>
#include <stdlib.h>

#include "packet.h"

#include "test_controller_packet.h"
#include "test_controller_communication.h"

#include "../src/log.c/src/log.h"

int main(int argc, char** argv) {
    if (argc < 2) { //there are no options
        if (!IS_IN_GRAPHIC_MODE) log_error("No options which packet should be tested in arguments");
        return 1;
    }

    if (!IS_IN_GRAPHIC_MODE) log_info("Unit test is starting");
    
    bool test_all=false;
    for (int i=0;i<argc;i++) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Test run[%i]: %s", i, argv[i]);
    }
    if (!strcmp(argv[1], "--all")) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Test all cases");
        test_all = true;
    }
    if (!strcmp(argv[1], "packet.minstrel.build") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel build");
        test_packet_build();
    }
    if (!strcmp(argv[1], "packet.minstrel.destroy") || test_all)  {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel destroy");
        test_packet_destroy();
    }
    if (!strcmp(argv[1], "packet.minstrel.checksum") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel checksum");
        test_packet_checksum();
    }
    if (!strcmp(argv[1], "packet.minstrel.serialize.normal") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel serialize normal");
        test_packet_serialize_normal();
    }
    if (!strcmp(argv[1], "packet.minstrel.serialize.zero") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel serialize zero case");
        test_packet_serialize_zero();
    }
    if (!strcmp(argv[1], "packet.minstrel.serialize.maximum") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel serialize maximum case");
        test_packet_serialize_maximum();
    }
    if (!strcmp(argv[1], "packet.minstrel.deserialize.normal") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel deserialize normal case");
        test_packet_deserialize_normal();
    } 
    if (!strcmp(argv[1], "packet.minstrel.deserialize.maximum") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start minstrel deserialize maximum case");
        test_packet_deserialize_maximum();
    }
    if (!strcmp(argv[1], "communication.init") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication init test");
        test_communication_initialization();
    }
    if (!strcmp(argv[1], "communication.sendOK.rcvOK.small") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication small send ok rcv ok test");
        test_communication_send_ok_rcv_ok();
    }
    if (!strcmp(argv[1], "communication.handshake.sendFAIL") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication handshake send fail and retry test");
        test_communication_send_error_handshake();
    }
    if (!strcmp(argv[1], "communication.handshake.ackFAIL") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication handshake ACK fail and retry test");
        test_communication_handshake_ack_error();
    }
    if (!strcmp(argv[1], "communication.handshake.checksum.sendFAIL") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication handshake checksum error");
        test_communication_send_wrong_checksum_error();
    }
    if (!strcmp(argv[1], "communication.handshake.checksum.ackFAIL") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication handshake checksum in ack error");
        test_communication_send_wrong_checksum_ack_error();
    }
    if (!strcmp(argv[1], "communication.send.bigData") || test_all) {
        if (!IS_IN_GRAPHIC_MODE) log_info("Start communication send big data test");
        test_communication_send_big_data();
    }

    

    return 0;
}