#pragma once
typedef int gpio_num_t;
void rtc_gpio_deinit(gpio_num_t);
void rtc_gpio_pullup_en(gpio_num_t);
void rtc_gpio_pulldown_dis(gpio_num_t);
