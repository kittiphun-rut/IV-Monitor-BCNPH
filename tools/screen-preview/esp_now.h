#pragma once
#include <cstdint>
#define ESP_OK 0
#define ESP_NOW_ETH_ALEN 6
typedef struct { uint8_t peer_addr[6]; uint8_t channel; int ifidx; bool encrypt; } esp_now_peer_info_t;
// ให้เหมือนของจริงใน ESP-IDF 5.x: มี src_addr / des_addr / rx_ctrl
typedef struct { int rssi; int channel; } wifi_pkt_rx_ctrl_t;
typedef struct {
  uint8_t *src_addr;
  uint8_t *des_addr;
  wifi_pkt_rx_ctrl_t *rx_ctrl;
} esp_now_recv_info_t;
typedef void (*esp_now_recv_cb_t)(const esp_now_recv_info_t*, const uint8_t*, int);
int esp_now_init(); int esp_now_deinit();
int esp_now_register_recv_cb(esp_now_recv_cb_t);
int esp_now_add_peer(const esp_now_peer_info_t*);
int esp_now_send(const uint8_t*, const uint8_t*, int);
