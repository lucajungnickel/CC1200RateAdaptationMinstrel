/**
 * SPIv1.h - Header for CC1200 Interaction
 * @author Garrit Honselmann
 **/


// Extended Register Space
#define EXT_ADR 0x2F00

//

/**
 * Read a single register from CC1200
 * If you want to read from extended Register space, 
 * bitwise or with EXT_ADR
 * e.g. extended address space FREQ1 = 0x0D
 * call would be:
 * cc1200_reg_read(EXT_ADR | 0x0D, &val)
 * @param adr the address to read
 * @param val pointer to write the read value to
 * @return the value read from register
 **/
int cc1200_reg_read(int adr, int *val);

/**
 * Write to a single register of CC1200
 * If you want to write to extended Register space, 
 * bitwise or with EXT_ADR
 * e.g. extended address space FREQ1 = 0x0D
 * call would be:
 * cc1200_reg_write(EXT_ADR | 0x0D, &val)
 * @param adr the address to write to
 * @param val pointer to write a value to
 * @return the value written
 **/
int cc1200_reg_write(int adr, int val);

/**
 * Command strobe to CC1200
 * @param cmd the command to write
 **/
int cc1200_cmd(int cmd);

/* command strobes */
#define SRES     0x30 /**< CC1200 in den Ausgangszustand setzen (Chip reset). */
#define SFSTXON  0x31 /**< Schalte den Frequenzsynthesizer ein und kalibriere ihn. */
#define SXOFF    0x32 /**< Gehe in den XOFF Zustand. */
#define SCAL     0x33 /**< Kalibriere Frequenzsynthesizer und schalte ihn aus. */
#define SRX      0x34 /**< Kalibriere Chip und schalte in den Empfangsmodus. */
#define STX      0x35 /**< Schalte in den Sendemodus. */
#define SIDLE    0x36 /**< Gehe in den Ruhezustand. */
#define SAFC     0x37 /**< Führe eine automatische Frequenz-Kompensation (AFC) aus. */
#define SWOR     0x38 /**< Starte die automatische RX polling Sequenz. */
#define SPWD     0x39 /**< Gehe in des SLEEP Mode. */
#define SFRX     0x3A /**< Lösche den RX FIFO. */
#define SFTX     0x3B /**< Lösche den TX FIFO. */
#define SWORRST  0x3C /**< Setze die eWOR Zeit. */
#define SNOP     0x3D  /**< Kein Kommando. 
                           * Wird benutzt um den Zustand des CC1200 zu ermitteln */
/**
  * Command Strobe to String
  * @param cmd the command strobe
 **/
char* cc1200_print_cmd (int cmd);

/**
 * last known CC1200 status
 *
 **/
int get_status_cc1200(void);

typedef enum {  
        IDLE          = 0, /**< Ruhe-Zustand */
        RX            = 1, /**< im Empfangsmode */
        TX            = 2, /**< im Sendemode */
        FSTXON        = 3, /**< Schneller Sendemode ist bereit */
        CALLIBRATE    = 4, /**< Frequenz Synthesizer wird kalibriert */
        SETTLING      = 5, /**< PLL rastet ein */
        RX_FIFO_ERROR = 6, /**< RX FIFO <C3><9C>ber- bzw. Unterlauf */
        TX_FIFO_ERROR = 7, /**< TX FIFO <C3><9C>ber- bzw. Unterlauf */
        UNKNOWN       = -1      /**< unbekannter Status */
} CC1200_STATES;

char* print_status (CC1200_STATES s);

/**
 * last know CC1200 status as string
 **/
const char *get_status_cc1200_str(void);

/**
 * Initialize SPI-CC1200 Interface
 **/
int spi_init(void);

/**
 * Shut down SPI-CC1200 Interface
 * Needs to be called before exiting program
 * Disable PRU
 **/
void spi_shutdown(void);

