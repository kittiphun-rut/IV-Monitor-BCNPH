#pragma once
// ตัวแทนของ esp_task_wdt.h สำหรับการจำลองบนเครื่อง PC
// บนเครื่อง PC ไม่มีสุนัขเฝ้าบ้าน ทุกฟังก์ชันจึงเป็นการเรียกเปล่า
// แต่ต้องมีให้ครบ เพื่อให้โค้ดชุดเดียวกับที่ลงบอร์ดจริงคอมไพล์ผ่านโดยไม่ต้องมี #ifdef
typedef struct {
  unsigned int timeout_ms;
  unsigned int idle_core_mask;
  bool trigger_panic;
} esp_task_wdt_config_t;

#ifndef ESP_ERR_INVALID_STATE
#define ESP_ERR_INVALID_STATE 0x103
#endif

inline int esp_task_wdt_init(const esp_task_wdt_config_t*) { return 0; }
inline int esp_task_wdt_init(unsigned int, bool)           { return 0; }
inline int esp_task_wdt_reconfigure(const esp_task_wdt_config_t*) { return 0; }
inline int esp_task_wdt_add(void*)    { return 0; }
inline int esp_task_wdt_delete(void*) { return 0; }
inline int esp_task_wdt_reset()       { return 0; }
