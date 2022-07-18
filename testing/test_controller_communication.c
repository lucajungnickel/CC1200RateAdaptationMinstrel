#include "test_controller_communication.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "receiver.h"
#include "sender.h"
#include "minstrel.h"
#include "cc1200_rate.h"
#include "sender_interface.h"
#include "../src/log.c/src/log.h"

volatile bool receive_done = false;
receiver_t *rcv = NULL;

static int id_sender = 10;
static int id_rcv = 20;

static void resetTests() {
    receive_done = false;
    rcv = NULL;
    cc1200_reset(id_sender);
    cc1200_reset(id_rcv);
}

static void *thread_receive_init() {
    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread started");
    receiver_t* receiver = receiver_init(id_sender, id_rcv);
    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread finished");

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);
    assert(receiver->lastPacketRcv->ack == 0);
    

    rcv = receiver; //for testing in main thread
    receive_done = true;
}


/**
 * @brief Tests if the initialiation and handshake is successful.
 * No payload data will be transmitted.
 */
void test_communication_initialization() {
    //Setup a sender and receiver
    cc1200_init(id_sender);
    cc1200_init(id_rcv);



    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_receive_init, NULL);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel, id_sender, id_rcv);
    
    //Kill Thread
    while (!receive_done) {
        sleep(0.0001);
    }
    pthread_join(thread_rcv_id, NULL);

    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);

    sender_destroy(sender);
    receiver_destroy(rcv);
}

//----------------------------------------------------------------


static void *thread_receive_send_ok_rcv_ok() {
    if (!IS_IN_GRAPHIC_MODE) log_debug("started rcv");
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);

    rcv = receiver;
    if (!IS_IN_GRAPHIC_MODE) log_debug("Start to receive payload");
    uint8_t* buffer;
    uint8_t len = receiver_receive_and_ack(rcv, &buffer);

    //Check payload
    if (!IS_IN_GRAPHIC_MODE) log_debug("Received payload after handshake");
    for (int i=0;i<len;i++) {
        assert(buffer[i] == i);
        if (!IS_IN_GRAPHIC_MODE) log_debug("0x%x ", i);
    }
    if (!IS_IN_GRAPHIC_MODE) log_debug("");

    //free(buffer);
    
    receive_done = true;
}



/**
 * @brief Tests one successful and correct send and one 
 * successful and correct receive
 * with correct data. There will be a handshake before.
 */
void test_communication_send_ok_rcv_ok() {
    if (!IS_IN_GRAPHIC_MODE) log_debug("Starts send ok rcv ok test");
    //Setup a sender and receiver
    resetTests();  
    cc1200_init(id_sender);
    cc1200_init(id_rcv);

    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_receive_send_ok_rcv_ok, NULL);
    
    sleep(0.001);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel, id_sender, id_rcv);
    
    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);

    if (!IS_IN_GRAPHIC_MODE) log_debug("Handshake ok");

    uint32_t buffer_len = 20;
    uint8_t* buffer = calloc(buffer_len, sizeof(uint8_t));
    for (int i=0; i<buffer_len;i++) {
        buffer[i] = i % UINT8_MAX;
    }
    sender_send_and_ack(sender, buffer, buffer_len, false);
    free(buffer);


    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }
    pthread_join(thread_rcv_id, NULL);

    receiver_destroy(rcv);
    sender_destroy(sender);

    //check if ack was received

/*
    assert(sender->lastPacketRcv != NULL);
    assert(sender->lastPacketRcv->ack == 2);
    assert(sender->lastPacketRcv->id == 2);
    assert(sender->lastPacketRcv->payload_len == 0);
*/
}

//---------------------------------------------------------------

static void *thread_send_error_handshake() {
    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread started");
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);
    assert(receiver->lastPacketRcv->ack == 0);

    rcv = receiver;
    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread init ok");

    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread finished");
    receive_done = true;
}

/**
 * @brief Test for sending a packet which will be lost.
 * 
 * The sender should get an ACK Timeout and send the packet again.
 */
void test_communication_send_error_handshake() {
    //Setup a sender and receiver
    resetTests();  
    cc1200_init(id_sender);
    cc1200_init(id_rcv);

    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_send_error_handshake, NULL);
    
    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    cc1200_debug_block_next_write(id_sender);
    sender_t* sender = sender_init(minstrel, id_sender, id_rcv);
    
    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }

    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend != NULL);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);

    pthread_join(thread_rcv_id, NULL);

    sender_destroy(sender);
    receiver_destroy(rcv);

    cc1200_reset(id_sender);
    cc1200_reset(id_rcv);
}

//------------------------------------------------------------------
static void *thread_handshake_ack_error() {
    cc1200_debug_block_next_write(id_rcv); //block next ACK
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);

    rcv = receiver;

    //it's important to receive the next packet, only at this point we can detect if 
    //an ack failed
    uint8_t* buffer;
    uint8_t len = receiver_receive_and_ack(rcv, &buffer);

    //Check payload
    receive_done = true;
}

void test_communication_handshake_ack_error() {
    //Setup a sender and receiver
    resetTests();  
    cc1200_init(id_sender);
    cc1200_init(id_rcv);

    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_handshake_ack_error, NULL);
    
    sleep(0.001);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel, id_sender, id_rcv);
    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }
    pthread_join(thread_rcv_id, NULL);

    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);

    sender_destroy(sender);
    receiver_destroy(rcv);

    cc1200_reset(id_sender);
    cc1200_reset(id_rcv);
}
//-------------------------------------------------------------------------------

static void *thread_communication_send_wrong_checksum_error() {
    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread started");
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    assert(receiver->debug_number_wrong_checksum == 1);
    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);
    assert(receiver->lastPacketRcv->ack == 0);

    rcv = receiver;
    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread init ok");

    if (!IS_IN_GRAPHIC_MODE) log_debug("rcv thread finished");
    receive_done = true;
}

void test_communication_send_wrong_checksum_error() {
//Setup a sender and receiver
    resetTests();  
    cc1200_init(id_sender);
    cc1200_init(id_rcv);

    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_communication_send_wrong_checksum_error, NULL);
    
    sleep(0.001);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    
    cc1200_debug_corrupt_next_checksum(id_sender);
    sender_t* sender = sender_init(minstrel, id_sender, id_rcv);
    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }
    pthread_join(thread_rcv_id, NULL);

    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);

    sender_destroy(sender);
    receiver_destroy(rcv);

    cc1200_reset(id_sender);
    cc1200_reset(id_rcv);
}


//-------------------------------------------------------------------------------------


void *thread_checksum_ack_error() {
    cc1200_debug_corrupt_next_checksum(id_rcv);
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);

    rcv = receiver;

    //it's important to receive the next packet, only at this point we can detect if 
    //an ack failed
    uint8_t* buffer;
    uint8_t len = receiver_receive_and_ack(rcv, &buffer);

    //Check payload
    receive_done = true;
}

void test_communication_send_wrong_checksum_ack_error() {
    //Setup a sender and receiver
    resetTests();  
    cc1200_init(id_sender);
    cc1200_init(id_rcv);

    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_checksum_ack_error, NULL);
    
    sleep(0.001);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel, id_sender, id_rcv);
    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }
    pthread_join(thread_rcv_id, NULL);

    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);
    assert(sender->debug_number_wrong_checksum == 1);

    sender_destroy(sender);
    receiver_destroy(rcv);
    
    cc1200_reset(id_sender);
    cc1200_reset(id_rcv);
}



//-------------------------------------------------------------------------------------

static uint32_t data_size = 1024 * 2 * 10;

void *thread_send_big_data() {
    receiver_t* receiver = receiver_init(id_sender, id_rcv);

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);

    rcv = receiver;
    

    uint32_t currentByte = 0;
    int numPacket = receiver->last_ack_rcv;
    while (currentByte < data_size) {
        uint8_t* buffer;
        uint8_t len = receiver_receive_and_ack(rcv, &buffer);

        for (int i=0;i<len;i++) {
            if (!IS_IN_GRAPHIC_MODE) log_debug("RCV: [%i] %i", currentByte, buffer[i]);
            assert(buffer[i] == currentByte % 256);
            currentByte++;
        }

        numPacket++;
        assert(rcv->last_ack_rcv == numPacket);
    }
    receive_done = true;
}

void test_communication_send_big_data() {
    //Setup a sender and receiver
    resetTests();  
    cc1200_init(id_sender);
    cc1200_init(id_rcv);

    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_send_big_data, NULL);
    
    sleep(0.001);

    //Test sender_interface
    if (!IS_IN_GRAPHIC_MODE) log_debug("Start init sender_interface in test function");
    sender_interface_t* s_interface = sender_interface_init(id_sender, id_rcv);
    if (!IS_IN_GRAPHIC_MODE) log_debug("Handshake succeeded in sender interface");

    if (!IS_IN_GRAPHIC_MODE) log_debug("Try to send a lot of data");
    uint8_t *buffer = calloc(data_size, sizeof(uint8_t));
    for (int i=0;i < data_size;i++) {
        buffer[i] = i % 256;
    }
    sender_interface_send_data(s_interface, buffer, data_size);
    if (!IS_IN_GRAPHIC_MODE) log_info("All data sent!");
    free(buffer);

    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }
    pthread_join(thread_rcv_id, NULL);

    //asserts
    assert(s_interface->isConnected == true);

    sender_interface_destroy(s_interface);
    receiver_destroy(rcv);
    
    cc1200_reset(id_sender);
    cc1200_reset(id_rcv);
}