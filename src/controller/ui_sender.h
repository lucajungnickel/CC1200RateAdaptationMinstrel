#ifndef UI_SENDER_H
#define UI_SENDER_H

#include <stdbool.h>

#include "../minstrel/minstrel.h"

bool ui_init();
bool ui_cleanup();
bool ui_show();
bool ui_update(Minstrel* minstrel);

#endif //UI_SENDER_H