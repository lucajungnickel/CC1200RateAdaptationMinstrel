#include "test_controller_communication.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "receiver.h"
#include "sender.h"
#include "minstrel.h"

/**
 * @brief Tests if the initialiation and handshake is successful.
 * No payload data will be transmitted.
 */
void test_communication_initialization() {
    //Setup a sender and receiver
    receiver_t* receiver = receiver_init();
    
    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    assert(minstrel != NULL);
    sender_t* sender = sender_init(minstrel);
    
}

/**
 * @brief Tests one successful and correct send and one 
 * successful and correct receive
 * with correct data.
 * 
 */
void test_communication_send_ok_rcv_ok() {
    //Setup a sender and receiver
    //sender_t* sender = sender_init();
    //receiver_t* receiver = receiver_init();


}