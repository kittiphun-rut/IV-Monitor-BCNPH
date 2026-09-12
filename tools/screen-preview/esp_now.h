#pragma once
#include <cstdint>
#define ESP_OK 0
typedef struct { uint8_t peer_addr[6]; uint8_t channel; int ifidx; bool encrypt; } esp_now_peer_info_t;
typedef struct { struct { int rssi; } *rx_ctrl; } esp_now_recv_info_t;
typedef void (*esp_now_recv_cb_t)(const esp_now_recv_info_t*, const uint8_t*, int);
int esp_now_init();
int esp_now_register_recv_cb(esp_now_recv_cb_t);
int esp_now_add_peer(const esp_now_peer_info_t*);
int esp_now_send(const uint8_t*, const uint8_t*, int);
