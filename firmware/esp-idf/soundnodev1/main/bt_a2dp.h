#pragma once
#include <stdbool.h>

bool bt_a2dp_init_soundnode();
bool bt_a2dp_is_connected();
void bt_a2dp_set_volume_pct(int pct);
int  bt_a2dp_volume_pct();
