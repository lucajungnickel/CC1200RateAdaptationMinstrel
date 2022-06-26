#include "test_controller_communication.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "receiver.h"
#include "sender.h"
#include "minstrel.h"
#include "cc1200_rate.h"

bool receive_done = false;
receiver_t *rcv = NULL;

static void resetTests() {
    receive_done = false;
    rcv = NULL;
    cc1200_reset();

}

static void *thread_receive_init() {
    printf("rcv thread started\n");
    receiver_t* receiver = receiver_init();
    printf("rcv thread finished\n");

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);

    rcv = receiver;
    receive_done = true;
}


/**
 * @brief Tests if the initialiation and handshake is successful.
 * No payload data will be transmitted.
 */
void test_communication_initialization() {
    //Setup a sender and receiver
    
    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_receive_init, NULL);
    sleep(0.1);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel);
    
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
}

//----------------------------------------------------------------


static void *thread_receive_send_ok_rcv_ok() {
    receiver_t* receiver = receiver_init();

    assert(receiver->token_receiver != 0);
    assert(receiver->token_sender != 0);
    assert(receiver->lastPacketRcv->id == 1);
    assert(receiver->lastPacketRcv->token_send != 0);
    assert(receiver->lastPacketRcv->token_recv == 0);
    assert(receiver->lastPacketSend->token_send == 
        receiver->lastPacketRcv->token_send);

    rcv = receiver;

    uint8_t* buffer;
  
    uint8_t len = receiver_receive_and_ack(rcv, &buffer);

    //Check payload
    for (int i=0;i<len;i++) {
        assert(buffer[i] == i);
    }
    receive_done = true;
}



/**
 * @brief Tests one successful and correct send and one 
 * successful and correct receive
 * with correct data.
 * 
 */
void test_communication_send_ok_rcv_ok() {
    //Setup a sender and receiver
    resetTests();  

    //Setup a sender and receiver
    
    //receiver thread:
    pthread_t thread_rcv_id;
    pthread_create(&thread_rcv_id, NULL, thread_receive_send_ok_rcv_ok, NULL);
    
    sleep(0.001);

    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel);
    //check for correct sender
    assert(sender->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->ack == 1);
    assert(rcv->lastPacketSend->id == 1);
    assert(rcv->lastPacketSend->payload_len == 0);
    assert(rcv->lastPacketRcv->payload_len == 0);
    assert(sender->next_ack == 2);

    uint32_t buffer_len = 20;
    uint8_t* buffer = calloc(buffer_len, sizeof(uint8_t));
    for (int i=0; i<buffer_len;i++) {
        buffer[i] = i % UINT8_MAX;
    }
    sender_send_and_ack(sender, buffer, buffer_len);
    //check if ack was received
    assert(sender->lastPacketRcv->ack == 2);
    assert(sender->lastPacketRcv->id == 2);
    assert(sender->lastPacketRcv->payload_len == 0);

    //Kill Thread
    while (!receive_done) {
        sleep(0.001);
    }
    pthread_join(thread_rcv_id, NULL);
}