#include <stdint.h>


#define MAX_RATES 11
// EWMA alpha value
#define WEIGHT 0.75


uint32_t MINSTREL_RATES[MAX_RATES];

typedef enum MinstrelState {
    READY,
    RESET,
    RUNNNING,
    STOPPED,
    DONE,
} MinstrelState;

typedef struct AvailableRates {
    uint8_t current;
    uint8_t best;
    uint8_t second_best;
    uint8_t highest_prob;
    uint8_t fallback;
} AvailableRates;

typedef struct MinstrelStatistics {
    uint32_t last_pkt_id;
    uint32_t pkt_count;
    float ewma; // Success probability
    float throughput;
    uint32_t total_send;
    uint32_t total_recv;
    uint32_t bytes_send;
    uint32_t avg_duration;
} MinstrelStatistics;

typedef struct Minstrel {
    AvailableRates rates;
    MinstrelState state;
    MinstrelStatistics statistics[MAX_RATES];
} Minstrel;

typedef enum packet_status_t {
    packet_status_none, //for initialization only
    packet_status_ok,
    packet_status_ok_ack,
    packet_status_warn_wrong_ack,
    packet_status_err_timeout,
    packet_status_err_checksum
} packet_status_t;

typedef struct Packet {
    uint32_t id;
    packet_status_t status;
    uint32_t bytes_send;
    uint32_t duration;

} Packet;
