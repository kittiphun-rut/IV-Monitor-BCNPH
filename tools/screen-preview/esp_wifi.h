#pragma once
#include <cstdint>
#define WIFI_IF_STA 0
#define WIFI_SECOND_CHAN_NONE 0
typedef int wifi_second_chan_t;
int esp_wifi_set_channel(uint8_t, wifi_second_chan_t);
int esp_wifi_get_channel(uint8_t*, wifi_second_chan_t*);
