#ifndef UI_SENDER_H
#define UI_SENDER_H

#include <stdbool.h>

#include "../controller/packet.h"
#include "../minstrel/minstrel.h"

bool ui_init();
bool ui_cleanup();
bool ui_show();
bool ui_update(Minstrel* minstrel);
bool ui_add_rate_change(int pkt_id, int new_rate);
bool ui_add_last_status(packet_status_t status);
#endif //UI_SENDER_H