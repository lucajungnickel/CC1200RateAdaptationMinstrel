#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

//See TYPE in 8bit protocol header field
//TODO überarbeiten
typedef enum packet_status_t {
    packet_status_none, //for initialization only
    packet_status_ok,
    packet_status_ok_ack,
    packet_status_warn_wrong_ack,
    packet_status_err_timeout
} packet_status_t;


typedef struct packet_t {
    uint32_t id;
    uint8_t next_symbol_rate;
    uint16_t token_recv; //token of receiver unit
    uint16_t token_send; //token of sender unit
    uint8_t type;
    uint32_t payload_len;
    uint8_t checksum;
    uint32_t ack;
    uint8_t fallback_rate;
    uint8_t *p_payload;
} packet_t;

/**
 * @brief Creates a packet object, payload pointer will be set to 0x0.
 *
 */
packet_t* packet_create (
    uint32_t const id,
    uint8_t  const next_symbol_rate,
    uint16_t  const token_recv,
    uint16_t  const token_send,
    uint8_t  const type,
    uint32_t  const payload_len,
    uint8_t  const checksum,
    uint32_t const ack,
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

/**
 * @brief Get the size of the header in bytes. The size is constant.
 */
uint32_t getHeaderSize();
#endif //PACKET_H
