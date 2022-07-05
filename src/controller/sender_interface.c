#include "sender_interface.h"

#include <stdlib.h>
#include <stdbool.h>

#include "../log.c/src/log.h"

static void sender_interface_ping_thread(sender_interface_t* interface) {
    //TODO implement
}

sender_interface_t* sender_interface_init(int socket_send, int socket_rcv) {
    sender_interface_t* back = calloc(1, sizeof(sender_interface_t));
    if (back == NULL) return NULL;

    back->isConnected = false;
    
    Minstrel* minstrel = calloc(1, sizeof(Minstrel));
    if (minstrel == NULL) return NULL;
    
    sender_t* sender = sender_init(minstrel, socket_send, socket_rcv);
    back->sender = sender;
    back->isConnected = true;
    //now the sender is connected!
    //start thread for regular PINGs
    //TODO implement
    
    return back;
}

void sender_interface_destroy(sender_interface_t* interface) {
    sender_destroy(interface->sender);
    free(interface);
}

int sender_interface_send_data(sender_interface_t *interface, uint8_t *data, uint32_t len) {
    if (interface == NULL || data == NULL || len == 0) {
        log_warn("Invalid vars for sender_interface_send_data, Interface: %p Data: %p Len: %i", interface, data, len);
        return 0;
    }

    if (!interface->isConnected) {
        log_warn("Sender interface is not connected, ignoring send data");
        return 1;
    }

    //split up data
    int numFullPackets = len / PACKET_SIZE;
    int sizeLastPacket = len % PACKET_SIZE;
    log_debug("Len sending data: %i, max packet size: %i", len, PACKET_SIZE);
    log_debug("Number full packets for interface sending: %i", numFullPackets);
    log_debug("Size last packet for interface sending: %i", sizeLastPacket);

    //send full packets
    for (int i=0; i<numFullPackets ; i++) {
        uint8_t *slice = data + i * PACKET_SIZE;
        sender_send_and_ack(interface->sender, slice, PACKET_SIZE);
    }

    //send small last packet
    if (sizeLastPacket != 0) {
        uint8_t *slice = data + numFullPackets * PACKET_SIZE;
        sender_send_and_ack(interface->sender, slice, sizeLastPacket);
    }

    return 0;
}