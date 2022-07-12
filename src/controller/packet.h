#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

/**
 *
 * Defines a packet status in the protocol header.
 * Is also used for communication in sender / receiver for status of the communication.

 * @see TYPE field in 8bit protocol header
 */
typedef enum packet_status_t {
    packet_status_none, //for initialization only
    packet_status_ok, //sender->receiver, if packet is valid
    packet_status_ok_ack, //receiver->sender, if ACK is valid
    packet_status_warn_wrong_ack, //warning, that the receiver didn't get a valid ack
    packet_status_err_timeout, //error, if timeout occured
    packet_status_err_checksum, //error, if a wrong checksum is detected
    packet_status_err_token_rcv, //error, if the receiver token is not correct
    packet_status_err_token_send //error, if the sender token is not correct
} packet_status_t;


typedef struct packet_t {
    uint32_t id; //SEQuential number, incremeting with each correct packet
    uint8_t next_symbol_rate; //for minstrel, next symbol rate the receiver will adapt after this packet is received
    uint8_t next_second_best_rate; //for minstrel
    uint8_t next_highest_pro_rate; //for minstrel
    uint16_t token_recv; //token of receiver unit
    uint16_t token_send; //token of sender unit
    uint8_t type; //packet status
    uint32_t payload_len; //len of payload, 0 if there is no payload
    uint8_t checksum; //checksum, see packet_set_checksum
    uint32_t ack; //acknowlegement number, only important when receiver ACKs a sender packet
    uint8_t fallback_rate; //for minstrel, fallback rate, set by sender. On receiver->sender it's not important
    uint8_t *p_payload; //pointer to payload
} packet_t;

/**
 * @brief Creates a packet object, payload pointer will be set to 0x0.
 *
 */
packet_t* packet_create (
    uint32_t const id,
    uint16_t  const token_recv,
    uint16_t  const token_send,
    uint8_t  const type,
    uint32_t  const payload_len,
    uint8_t  const checksum,
    uint32_t const ack,
    uint8_t  const next_symbol_rate,
    uint8_t const next_second_best_rate,
    uint8_t const next_highest_pro_rate,
    uint8_t  const fallback_rate);

/**
 * @brief Destroys the packet and also frees the allocated memory in payload data.
 * 
 */
void packet_destroy(packet_t* const packet);

/**
 * @brief Calculates and returns the checksum.
 * 
 */
uint8_t packet_calc_checksum(packet_t* const packet);

/**
 * @brief Calculates and adds the checksum to the given packet.
 * Any checksum in the packet will be overwritten.
 * 
 * @param packet 
 */
void packet_set_checksum(packet_t* const packet);

/**
 * @brief Converts a packet to a bitstream.
 * 
 * @param packet Packet, which will be serialized to the buffer
 * @param p_buffer Pointer to the empty bit stream. Should be the size of getSize(packet)
 * @return 0 if everything worked, 1 if error occured
 */
uint8_t packet_serialize(packet_t* const packet, uint8_t* p_buffer);

/**
 * @brief Converts a bitstream to a  packet.
 * 
 * 
 * @param p_buffer pointer to buffer which will be converted to packet
 * @return packet_t* packet or NULL IF an error occured
 */
packet_t* packet_deserialize(uint8_t* const p_buffer);

/**
 * @brief Get the complete size of the packet in bytes, header + payload included.
 */
uint32_t packet_get_size(packet_t* const packet);

void packet_print(packet_t* const packet);

/**
 * @brief Get the size of the header in bytes. The size is constant.
 */
uint32_t getHeaderSize();
#endif //PACKET_H
