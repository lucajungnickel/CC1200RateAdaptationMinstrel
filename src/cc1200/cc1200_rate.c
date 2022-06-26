#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <SPIv1.h>

#include "cc1200_rate.h"
#include "packet.h"

void cc1200_reset() {
    
}

void cc1200_change_rate(uint32_t rate) {
    float rate_in_ksps = rate / 1000.;
    uint32_t srate_e = log_2((rate_in_ksps * pow(2, 39)) / F_XOSC) - 20;
    uint32_t srate_m = ((rate_in_ksps * pow(2, 39)) / (F_XOSC * (1 << srate_e))) - (1 << 20);

    uint8_t symbol_rate_2 = ((srate_e & 0xF) << 4) | (0xF & (srate_m >> 16));
    uint8_t symbol_rate_1 = 0xFF & (srate_m >> 8);
    uint8_t symbol_rate_0 = 0xFF & srate_m;

    // Write exponent and mantissa [19:16]
    cc1200_reg_write(SYMBOL_RATE2, symbol_rate_2);
    // Write mantissa [15:8]
    cc1200_reg_write(SYMBOL_RATE1, symbol_rate_1);
    // Write mantissa [7:0]
    cc1200_reg_write(SYMBOL_RATE0, symbol_rate_0);
}

// TODO: Error handling + fixed length (?)
void cc1200_send_packet(packet_t* packet) {

    //To be tested:
    uint32_t len = packet_get_size(packet);
    uint8_t* buffer = malloc(len * sizeof(uint8_t));
    packet_serialize(packet, buffer);

    // ---------------- VARIABLE LENGTH ----------------------
    // Write length byte
    cc1200_reg_write(REG_FIFO, len);

    // Write TX FIFO
    for (int i=0; i<len; i++)
        cc1200_reg_write(REG_FIFO, buffer[i]);

    // Switch to TX mode
    cc1200_cmd(STX);

    // Wait till TX mode entered
    while (get_status_cc1200() != TX)
        cc1200_cmd(SNOP);
       
    // Wait till TX mode left
    while (get_status_cc1200() != IDLE)
        cc1200_cmd(SNOP);

    if (IS_DEBUG) {
        puts("DEBUG: Sent packet!");
        printf("DEBUG: Status: %s\n", get_status_cc1200_str());
    }
}

// TODO: Error handling + fixed length (?)
uint32_t cc1200_get_packet(uint8_t* buffer) {
    // ---------------- VARIABLE LENGTH ----------------------
    // Switch to RX mode
    cc1200_cmd(SRX);
    while (get_status_cc1200() != RX)
        cc1200_cmd(SNOP);

    unsigned int num_rx_bytes, pkt_len;
    while(1) {
        cc1200_reg_read(NUM_RXBYTES, &num_rx_bytes);

        // Got something in RX FIFO
        if (num_rx_bytes > 0) {
            while(1) {
                // Read packet len
                pkt_len = cc1200_reg_read(REG_FIFO, NULL);
                
                // Only read if whole packet is received
                if (num_rx_bytes >= pkt_len) {
                    buffer = calloc(pkt_len + PKT_OVERHEAD, sizeof(uint8_t));

                    // Read whole packet
                    for (int i=0; i < pkt_len + PKT_OVERHEAD; i++)
                        buffer[i] = cc1200_reg_read(REG_FIFO, NULL);

                    if (IS_DEBUG) {
                        printf("DEBUG: Packet received: \n\t");
                        for (int i=0; i < pkt_len; i++) {
                            printf("%c",buffer[i]);
                        }
                        printf("\n");
                    }
                    return pkt_len;
                }
            }
        }
    }
}

float log_2(float num) {
    return log(num) / log(2.);
}
