/**
 * @file sender_interface.h
 * @author Luca
 * @brief Inits a connection to a receiver and will keep it alive with regular 'ping' packets.
 *  Can send big amount of data, which will be split into fixed sized packets by this interface.
 * @date 2022-07-05
 * 
 */


#ifndef SENDER_INTERFACE_H
#define SENDER_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "sender.h"

//Maximum size of payload data
#define PACKET_SIZE 20

typedef struct sender_interface_t {
    bool isConnected;
    sender_t *sender;
    clock_t lastTimeSent;
} sender_interface_t;

/**
 * @brief Inits the sender interface, will connect to a receiver and performs a handshake.
 * 
 * @return sender interface instance or NULL if error occurred
 */
sender_interface_t* sender_interface_init(int socket_send, int socket_rcv);

/**
 * @brief Destroys the given interface with all it's content
 * 
 */
void sender_interface_destroy(sender_interface_t *interface);
/**
 * @brief Sends a large portion of data to the client.
 * Internally the big data amount will maybe split into smaller packets
 * by this function.
 * 
 * @param data pointer to data
 * @param len len of data
 * @return int return status. 0 is ok
 */
int sender_interface_send_data(sender_interface_t *interface, uint8_t *data, uint32_t len);

#endif //SENDER_INTERFACE_H