#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <SPIv1.h>
#include <time.h>

#include "cc1200_rate.h"
#include "../controller/packet.h"


/**
 * @brief Builds PKG_CFG0 register in the correct format.
 *
 * It doesn't check if parameters are not to big.
 * If they are weird things could happen.
 *
 * @param length_config 2 bits long
 * @param pkt_bit_len 3 bits long
 * @param uart_mode_en 1 bit long
 * @param uart_swap_en 1 bit long
 * @return uint8_t PKG_CFG0 register
 */
static uint8_t build_pkg_cfg0(uint8_t length_config, uint8_t pkt_bit_len, uint8_t uart_mode_en, uint8_t uart_swap_en) {
    uint8_t build = 0;
    length_config = length_config << 5;
    pkt_bit_len = pkt_bit_len << 2;
    uart_mode_en = uart_mode_en << 1;
    //uart_swap_en = uart_swap_en << 0; (for completeness)

    //mask and or it to build
    build = build | (length_config  & 0b01100000);
    build = build | (pkt_bit_len    & 0b00011100);
    build = build | (uart_mode_en   & 0b00000010);
    build = build | (uart_swap_en   & 0b00000001);

    return build;
}

/**
 * @brief Calls build_pkg_cfg0 with default reg settings for UART
 * Default settings are from RegSettings
 *
 * @param length_config
 * @return uint8_t
 */
static uint8_t build_pkg_cfg0_std(uint8_t length_config) {
    uint8_t pkt_bit_len =   (RegSettings[PKT_CFG0].val & 0b00011100) >> 2;
    uint8_t uart_mode_en =  (RegSettings[PKT_CFG0].val & 0b00000010) >> 1;
    uint8_t uart_swap_en =  (RegSettings[PKT_CFG0].val & 0b00000001) /* >> 0 */;
    return build_pkg_cfg0(length_config, pkt_bit_len, uart_mode_en, uart_swap_en);
}

/**
 * @brief
 *
 * @param byte_swap_en 1 bit long
 * @param fg_mode_en    1 bit long
 * @param cca_mode      3 bits long
 * @param pkt_format    2 bits long
 * @return uint8_t
 */
static uint8_t build_pkg_cfg2(uint8_t byte_swap_en, uint8_t fg_mode_en, uint8_t cca_mode, uint8_t pkt_format) {
    uint8_t build = 0;

    byte_swap_en = byte_swap_en << 6;
    fg_mode_en = fg_mode_en     << 5;
    cca_mode = cca_mode         << 2;

    //mask and or it to build
    build = build | (byte_swap_en  & 0b01000000);
    build = build | (fg_mode_en    & 0b00100000);
    build = build | (cca_mode      & 0b00011100);
    build = build | (pkt_format    & 0b00000011);

    return build;
}

/**
 * Calls build_pkg_cfg2 with default reg settings (from imported reg file)
 * Default settings are from RegSettings
 *
 * @param pkt_format
 * @return uint8_t
 */
static uint8_t build_pkt_cfg2_std(uint8_t pkt_format) {
    uint8_t byte_swap_en = (RegSettings[PKT_CFG2].val & 0b01000000) >> 6;
    uint8_t fg_mode_en   = (RegSettings[PKT_CFG2].val & 0b00100000) >> 5;
    uint8_t cca_mode     = (RegSettings[PKT_CFG2].val & 0b00011100) >> 2;
    return build_pkg_cfg2(byte_swap_en, fg_mode_en, cca_mode, pkt_format);
}

const int TIMEOUT = 1000;

void cc1200_reset() {

}


void cc1200_init(int id) {
    //id can be ignored here

    // Initialize SPI
    if (spi_init()) {
        puts("error spi init");
        exit(1);
    }

    // Reset CC1200
    cc1200_cmd(SRES);
    cc1200_cmd(SNOP);

    int i, adr, val;
    // Set standard register
    for (i=0; i<MAX_REG; i++) {
        adr = RegSettings[i].adr;
        val = RegSettings[i].val;
        cc1200_reg_write(adr, val);
    }
    // Set extended register
    for (i=0; i<MAX_EXT_REG; i++) {
        adr = ExtRegSettings[i].adr;
        val = ExtRegSettings[i].val;
        cc1200_reg_write(adr, val);
    }

    // Configure variable packet mode
    cc1200_reg_write(PKT_CFG0, build_pkt_cfg0_std(PKT_MODE));
    cc1200_reg_write(PKT_CFG2, build_pkt_cfg2_std(PKT_FORMAT));
    cc1200_reg_write(PKT_LEN, PKT_MAX_LEN);

    if (IS_DEBUG)
        puts("DEBUG: Initialized CC1200 registers.");
}

/**
 * @brief Calls build_pkg_cfg0 with default reg settings for UART
 * Default settings are from RegSettings
 *
 * @param length_config
 * @return uint8_t
 */
static uint8_t build_pkg_cfg0_std(uint8_t length_config) {
    uint8_t pkt_bit_len =   (RegSettings[PKT_CFG0].val & 0b00011100) >> 2;
    uint8_t uart_mode_en =  (RegSettings[PKT_CFG0].val & 0b00000010) >> 1;
    uint8_t uart_swap_en =  (RegSettings[PKT_CFG0].val & 0b00000001) /* >> 0 */;
    return build_pkg_cfg0(length_config, pkt_bit_len, uart_mode_en, uart_swap_en);
}

/**
 * Calls build_pkg_cfg2 with default reg settings (from imported reg file)
 * Default settings are from RegSettings
 *
 * @param pkt_format
 * @return uint8_t
 */
static uint8_t build_pkt_cfg2_std(uint8_t pkt_format) {
    uint8_t byte_swap_en = (RegSettings[PKT_CFG2].val & 0b01000000) >> 6;
    uint8_t fg_mode_en   = (RegSettings[PKT_CFG2].val & 0b00100000) >> 5;
    uint8_t cca_mode     = (RegSettings[PKT_CFG2].val & 0b00011100) >> 2;
    return build_pkg_cfg2(byte_swap_en, fg_mode_en, cca_mode, pkt_format);
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

// TODO: Error handling
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


// TODO: Error handling
packet_t* cc1200_get_packet(clock_t timeout_started, packet_status_t *status_back) {

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
