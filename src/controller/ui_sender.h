#ifndef UI_SENDER_H
#define UI_SENDER_H

#include <stdbool.h>

#include "../minstrel/minstrel.h"

bool IS_IN_GRAPHIC_MODE;

bool ui_init();
bool ui_cleanup();
bool ui_show();
bool ui_update(Minstrel* minstrel);
bool ui_add_rate_change(int pkt_id, int new_rate);
#endif //UI_SENDER_H