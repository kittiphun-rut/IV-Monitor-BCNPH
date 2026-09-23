#pragma once
#include <cstdint>
#define WIFI_IF_STA 0
#define WIFI_IF_AP 1
#define WIFI_SECOND_CHAN_NONE 0
#define WIFI_PS_NONE 0
typedef int wifi_second_chan_t;
typedef int wifi_interface_t;
int esp_wifi_get_channel(uint8_t*, wifi_second_chan_t*);
int esp_wifi_set_channel(uint8_t, wifi_second_chan_t);
int esp_wifi_set_ps(int);
typedef enum { ESP_SLEEP_WAKEUP_UNDEFINED=0, ESP_SLEEP_WAKEUP_EXT0=2, ESP_SLEEP_WAKEUP_GPIO=7 } esp_sleep_wakeup_cause_t;
esp_sleep_wakeup_cause_t esp_sleep_get_wakeup_cause();
int esp_sleep_enable_ext0_wakeup(int, int);
void esp_deep_sleep_start();
