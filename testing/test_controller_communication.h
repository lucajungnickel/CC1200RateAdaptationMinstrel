#ifndef TEST_CONTROLLER_COMMUNICATION_H
#define TEST_CONTROLLER_COMMUNICATION_H

void test_communication_send_ok_rcv_ok();

void test_communication_initialization();

void test_communication_send_error_handshake();

void test_communication_handshake_ack_error();

void test_communication_send_wrong_checksum_error();
#endif //TEST_CONTROLLER_COMMUNICATION_H