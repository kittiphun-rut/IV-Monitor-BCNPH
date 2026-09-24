/**
 * @file      Station-C3-OLED.ino
 * @brief     เฟิร์มแวร์เครื่องประจำเตียง รุ่นบอร์ดเล็ก จอ OLED 0.42 นิ้ว
 * @version   1.2.0-C3
 * @date      2026-09-23
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Hardware
 * ESP32-C3 Super Mini + จอ OLED 0.42" (SSD1306 I2C 72x40)
 * TCRT5000 AO -> GPIO 4 · ตัวแบ่งแรงดันแบตเตอรี่ -> GPIO 0
 * ออดแบบมีวงจรกำเนิดเสียงในตัว -> GPIO 3 · ปุ่มกด -> GPIO 7
 * จอ OLED I2C -> SCL GPIO 6, SDA GPIO 5
 *
 * @par Description
 * ปรับปรุงจากไฟล์ `ESP32-C3-Station-V_4_4_2.ino` ให้เข้ากับระบบรุ่นปัจจุบัน
 * ของเดิมคุยกับ Host รุ่นนี้ไม่ได้เลย เพราะ `struct_message` เป็น 14 ไบต์
 * ขณะที่ Protocol v3 ต้องเป็น 23 ไบต์ Host จึงทิ้งแพ็กเก็ตตั้งแต่ต้น
 * คงผังขาเดิมของบอร์ดไว้ทุกขา ไม่ต้องแก้สายที่ประกอบไว้แล้ว
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 1.2.0-C3 | 2026-09-24 | เพิ่มตัวช่วยคาลิเบรตสี่ขั้นตอนที่หน้าเครื่อง วัดทิศสัญญาณเอง และบันทึกสิ่งที่เรียนรู้ลง NVS |
 * | 1.1.0-C3 | 2026-09-23 | จำนวนหยดสะสมรอดการรีบูตและไฟดับแล้ว และเพิ่มสุนัขเฝ้าบ้านกันเครื่องค้างเงียบ |
 * | 1.0.0-C3 | 2026-09-23 | สร้างสายใหม่จากไฟล์ v4.4.2 เดิม ปรับเป็น Protocol v3 ไล่หาช่องสัญญาณเอง ใช้ `drop_detector.h` ตัวเดียวกับสายหลัก และออกแบบหน้าจอ 72x40 ใหม่ทั้งหมด |
 *
 * @warning  ไฟล์วาดจอถูก `#include` ท้ายไฟล์นี้ก่อน `setup()` ห้ามย้ายขึ้นไปบนสุด
 *           เพราะโค้ดในไฟล์นั้นใช้ตัวแปรและฟังก์ชันช่วยที่ประกาศไว้ด้านบน
 *
 * @warning  `struct_message` และ `struct_host_sync` ต้องเหมือนกันทุกไบต์ในทุกเฟิร์มแวร์
 *           ตรวจด้วย `bash tools/protocol-test/run.sh` ก่อน commit ทุกครั้งที่แตะโครงสร้างนี้
 *
 * @warning  ยังไม่ได้ทดสอบบนฮาร์ดแวร์จริง ตรวจด้วยเครื่องมือจำลองบนเครื่อง PC เท่านั้น
 *
 * @par บันทึกการเปลี่ยนแปลงโดยละเอียด
 * เก็บข้อความเดิมไว้ทั้งหมด เพราะเหตุผลเชิงเทคนิคในนั้นหาจากที่อื่นไม่ได้
 *  ที่มา: ปรับปรุงจากเฟิร์มแวร์ ESP32-C3 v4.4.2 ให้เข้ากับระบบรุ่นปัจจุบัน
 *
 *  ของเดิมคุยกับ Host รุ่นปัจจุบัน "ไม่ได้เลย" เพราะโครงสร้างแพ็กเก็ตคนละแบบ
 *
 *    ของเดิม  struct_message   14 ไบต์   ไม่มี flowRateHr / msSinceLastDrop / flags
 *    ของเดิม  struct_time_sync  5 ไบต์   Host รุ่นปัจจุบันส่ง struct_host_sync 20 ไบต์
 *
 *  รุ่นนี้ใช้ Protocol v3 ครบถ้วน จึงเสียบเข้าระบบเดิมได้ทันทีโดยไม่ต้องแก้ฝั่ง Host
 *
 *  สิ่งที่แก้จาก v4.4.2 เรียงตามความสำคัญ
 *
 *   1) โครงสร้างแพ็กเก็ตเป็น Protocol v3 (23/20 ไบต์) พร้อม static_assert กันพลาด
 *      รับค่า dropFactor, targetRateHr, totalPlanMl, alertCode, nearEndPct จาก Host
 *      ของเดิมตรึง drop factor ไว้ที่ 20 ในโค้ด เปลี่ยนตามแผนการรักษาไม่ได้
 *
 *   2) การหาช่องสัญญาณ ของเดิมสแกนหา SSID ครั้งเดียวตอนบูตแล้วตรึงช่องไว้
 *      ถ้า Host ย้ายช่อง (ซึ่งเกิดแน่เมื่อ Host ไปเกาะเราเตอร์ตั้งแต่ v4.7.6)
 *      เครื่องจะเงียบตลอดไปจนกว่าจะรีเซ็ต รุ่นนี้ไล่หาช่อง 1-13 เองเมื่อไม่ได้ยิน Host
 *      แล้วจำช่องที่เจอลง NVS เหมือนสายหลัก อย่างช้าที่สุด 32.5 วินาทีก็กลับมาออนไลน์
 *
 *   3) เสียงออดของเดิมใช้ delay() ระหว่างส่งเสียงลูปหยุดทั้งเครื่อง ตรวจจับหยดไม่ทัน
 *      (เสียงยืนยันตอนคาลิเบรตยาวถึง 800 ms) รุ่นนี้เป็นคิวจังหวะแบบไม่บล็อก
 *
 *   4) สูตรอัตราการไหล ของเดิมนับหยดในหน้าต่าง 2.5 วินาทีแล้วคูณขึ้นเป็นต่อนาที
 *      ที่ 20 หยด/นาที หน้าต่างนั้นมีหยดเฉลี่ยไม่ถึง 1 หยด ตัวเลขจึงกระโดด 0-48
 *      รุ่นนี้ใช้หน้าต่างสะสม 8 หยดแบบเดียวกับสายหลัก (แก้ความลำเอียงของ Jensen)
 *
 *   5) ตัวตรวจจับหยดใช้ drop_detector.h ตัวเดียวกับสายหลัก เรียนรู้รูปคลื่นหยดเองได้
 *      และตั้งแต่ v1.2.0-C3 มีตัวช่วยคาลิเบรตสี่ขั้นตอนไว้ตรวจว่าวางเซนเซอร์ถูกที่
 *      ของเดิมใช้เกณฑ์คงที่ + ฮิสเทอรีซิส ซึ่งถุงแกว่งตอนคนไข้เดินก็นับเป็นหยด
 *
 *   6) เลขเตียงตั้งที่หน้าเครื่องแล้วจำลง NVS ของเดิมต้องแก้ #define แล้วคอมไพล์ใหม่
 *      ทีละเตียง ซึ่งใช้จริงในหอผู้ป่วยไม่ไหว
 *
 *   7) หน้าจอออกแบบใหม่ทั้งหมดสำหรับ 72x40 ดูหัวข้อในไฟล์ StationScreen.h
 *
 *  การต่อสาย (คงผังขาเดิมของบอร์ด v4.4.2 ไว้ทุกขา)
 *    TCRT5000 AO   -> GPIO 4  (ADC1_CH4)
 *    ตัวแบ่งแรงดันแบตเตอรี่ -> GPIO 0  (ADC1_CH0)
 *    ออดแบบมีวงจรกำเนิดเสียงในตัว -> GPIO 3  (สั่งงานด้วยลอจิกสูง)
 *    ปุ่มกด        -> GPIO 7  (ต่อลงกราวด์ ใช้ตัวต้านทานพูลอัพในตัว)
 *    จอ OLED I2C   -> SCL GPIO 6, SDA GPIO 5
 *
 *  ข้อจำกัด: ยังไม่ได้ทดสอบบนฮาร์ดแวร์จริง ตรวจด้วยเครื่องมือจำลองบน PC เท่านั้น
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Preferences.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <esp_task_wdt.h>          // [1.1.0-C3] เพิ่ม: สุนัขเฝ้าบ้าน กันเครื่องค้างเงียบ
#include <esp_system.h>            // [1.1.0-C3] เพิ่ม: อ่านสาเหตุการรีบูตครั้งล่าสุด

// ----------------------------------------------------------------------------
// สุนัขเฝ้าบ้านและสาเหตุการรีบูต ([1.1.0-C3] เพิ่มทั้งหมด)
//
// เครื่องนี้ทำหน้าที่เตือนภัย การค้างแบบเงียบจึงอันตรายกว่าการรีบูต เพราะจอยัง
// ค้างภาพเดิมไว้ พยาบาลจึงเข้าใจว่าระบบยังเฝ้าอยู่ ทั้งที่หยุดไปแล้ว
// ตั้งไว้ 8 วินาที ซึ่งยาวกว่ารอบ loop() ปกติหลายเท่า จึงไม่รีบูตเพราะงานหนักชั่วคราว
// ----------------------------------------------------------------------------
#define LOOP_WDT_TIMEOUT_S      8

esp_reset_reason_t bootResetReason = ESP_RST_UNKNOWN;


#include "drop_detector.h"   // อัลกอริทึมตรวจจับหยด (ทดสอบบน PC ได้: tools/detector-test)

#define APP_VERSION         "1.2.0-C3"

// ---------------------------------------------------------------------------
// ขา
// ---------------------------------------------------------------------------
#define SENSOR_AO_PIN       4
#define BAT_ADC_PIN         0
#define BUZZER_PIN          3
#define BUTTON_PIN          7
#define OLED_SCL_PIN        6
#define OLED_SDA_PIN        5

// ---------------------------------------------------------------------------
// ค่าคงที่ของระบบ (ให้ตรงกับสายหลัก จะได้ประพฤติเหมือนกัน)
// ---------------------------------------------------------------------------
#define DEFAULT_ESPNOW_CHANNEL  1
#define MAX_WIFI_CHANNEL        13
#define HOST_TIMEOUT_MS         5000
#define CHANNEL_SCAN_DWELL_MS   2500     // Host ส่ง Sync ทุก 1 วินาที -> รอแต่ละช่อง 2.5 วินาที
#define CHANNEL_SCAN_START_MS   6000     // หลังบูตรอช่องที่จำไว้ก่อน 6 วินาที
#define SEND_INTERVAL_MS        1000
#define SEND_JITTER_MS          60       // สุ่มบวกลบรอบคาบ กันสองสเตชันส่งชนกันค้างนาน

#define MIN_OCCLUSION_MS        8000
#define MAX_OCCLUSION_MS        300000
#define DEFAULT_DROP_FACTOR     20
#define DEFAULT_NEAR_END_PCT    80
#define RATE_WINDOW_DROPS       8
#define DROP_FLASH_MS           140      // ไฟกะพริบตอนนับได้หนึ่งหยด
#define SCREENSAVER_IDLE_MS     180000
#define AUTO_HOME_MS            20000

#define ALERT_NONE        0
#define ALERT_TOO_FAST    1
#define ALERT_TOO_SLOW    2
#define ALERT_NEAR_END    3
#define ALERT_COMPLETE    4
#define ALERT_OCCLUSION   5

// ---------------------------------------------------------------------------
// โครงสร้างแพ็กเก็ต Protocol v3 — ต้องเหมือน Host ทุกไบต์
// ตรวจอัตโนมัติด้วย bash tools/protocol-test/run.sh
// ---------------------------------------------------------------------------
typedef struct __attribute__((packed)) struct_message {
  uint8_t  stationId;
  uint8_t  isRunning;
  uint32_t totalDrops;
  uint32_t periodDrops;
  float    flowRateHr;
  uint32_t msSinceLastDrop;
  float    batteryVolts;
  uint8_t  flags;            // bit0 = พยาบาลกดรับทราบเตือนใกล้หมดที่เตียง
} struct_message;            // 23 bytes

typedef struct __attribute__((packed)) struct_host_sync {
  uint32_t epochTime;
  uint8_t  isSynced;
  uint8_t  stationId;
  float    targetRateHr;
  float    totalPlanMl;
  uint8_t  alertCode;
  uint8_t  dropFactor;
  uint8_t  resetSeq;
  uint8_t  hostChannel;
  uint8_t  nearEndPct;       // % ของปริมาตรตามแผนที่เริ่มเตือนใกล้หมด
  uint8_t  flags;            // bit0 = Host บันทึกว่ารับทราบเตือนใกล้หมดแล้ว
} struct_host_sync;          // 20 bytes

static_assert(sizeof(struct_message) == 23, "struct_message must be 23 bytes (match Host)");
static_assert(sizeof(struct_host_sync) == 20, "struct_host_sync must be 20 bytes (match Host)");

// ---------------------------------------------------------------------------
// อุปกรณ์
// ---------------------------------------------------------------------------
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                    /* clock=*/ OLED_SCL_PIN, /* data=*/ OLED_SDA_PIN);
Preferences stationPrefs;
iv::DropDetector detector;

struct_message myData;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---------------------------------------------------------------------------
// สถานะ
// ---------------------------------------------------------------------------
uint8_t  stationId            = 1;
bool     isRunning            = true;

uint32_t totalDrops           = 0;
uint32_t periodDropsCounter   = 0;
float    currentFlowRate_ml_hr = 0.0f;
float    currentGttMin        = 0.0f;
float    totalVolumeMl        = 0.0f;
float    batteryVolts         = 0.0f;
int      sensorRaw            = 0;

unsigned long lastDropTimestamp   = 0;
bool     hasFirstDropOccurred     = false;
bool     skipNextInterval         = false;
bool     sensorHasDrops           = false;
unsigned long dropFlashUntil      = 0;   // ใช้วาดจังหวะหยดบนจอ

// ---- ค่าที่ Host ส่งมา ----
volatile uint8_t  hostAlertCode   = ALERT_NONE;
float    targetRateHr             = 0.0f;
float    totalPlanMl              = 0.0f;
uint8_t  dropFactor               = DEFAULT_DROP_FACTOR;
uint8_t  nearEndPct               = DEFAULT_NEAR_END_PCT;
uint8_t  lastResetSeq             = 0;
bool     hostAckedNearEnd         = false;
bool     nearEndAckRequest        = false;   // พยาบาลกดรับทราบที่เตียง
unsigned long lastHostSyncTime    = 0;
bool     isClockSynced            = false;

// ---- วิทยุ ----
uint8_t  espnowChannel            = DEFAULT_ESPNOW_CHANNEL;
uint8_t  savedChannel             = DEFAULT_ESPNOW_CHANNEL;
unsigned long lastChannelHopTime  = 0;
unsigned long lastSendTime        = 0;
uint16_t sendJitter               = 0;

// ---- ออด (คิวจังหวะแบบไม่บล็อก) ----
uint8_t  beepRemaining            = 0;
uint16_t beepOnMs                 = 0;
uint16_t beepOffMs                = 0;
bool     beepIsOn                 = false;
unsigned long beepNextChange      = 0;

// ---- หน้าต่างคำนวณอัตราไหล (เหมือนสายหลัก) ----
unsigned long rateWinMs[RATE_WINDOW_DROPS] = {0};
uint8_t  rateWinIdx   = 0;
uint8_t  rateWinCount = 0;

// ---- ปุ่มและหน้าจอ ----
enum UiPage : uint8_t { PAGE_RATE = 0, PAGE_GTT, PAGE_VOLUME, PAGE_STATUS, PAGE_COUNT };
enum UiMode : uint8_t { MODE_NORMAL = 0, MODE_SET_ID, MODE_ALERT, MODE_SAVER,
                        MODE_CALIB };   // [1.2.0-C3] เพิ่ม: ตัวช่วยคาลิเบรตสี่ขั้นตอน

// ---------------------------------------------------------------------------
// ตัวช่วยคาลิเบรตสี่ขั้นตอน ([1.2.0-C3] เพิ่มทั้งหมด)
//
// ของเดิมบอร์ดเล็กไม่มีหน้าคาลิเบรตเลย ตัวตรวจจับเรียนรู้เองได้ก็จริง แต่
//   1) ไม่มีใครรู้ว่าวางเซนเซอร์ถูกตำแหน่งหรือยัง จนกว่าจะรอดูว่ามันนับหรือเปล่า
//   2) สิ่งที่เรียนรู้ไว้หายทุกครั้งที่ปิดเครื่อง เปิดมาต้องเรียนใหม่ทุกที
// ทั้งสองข้อคือความล้มเหลวแบบเงียบ เพราะจอขึ้นว่าทำงานปกติในขณะที่ยังไม่นับอะไรเลย
//
// แบ่งเป็นสี่ขั้น เพราะแต่ละขั้นตอบคำถามคนละข้อ และบอกพยาบาลได้ว่าต้องไปแก้ตรงไหน
//   1 BASELINE  ปิดโรลเลอร์แคลมป์ให้นิ่ง — เซนเซอร์เสียบแน่นไหม มีแสงกวนไหม
//   2 DIRECTION เปิดให้หยด — หยดทำให้สัญญาณสูงขึ้นหรือต่ำลง ตัวตรวจจับหาเอง
//   3 LEARN     เก็บรูปคลื่นจาก 10 หยด — ได้ความสูงพัลส์ ความกว้าง และ SNR
//   4 VERIFY    นับอีก 10 หยดเพื่อยืนยันว่าจังหวะสม่ำเสมอจริง ไม่ใช่ฟลุก
// ผ่านครบสี่ขั้นจึงบันทึกลง NVS ขั้นไหนไม่ผ่านจะบอกสาเหตุ ไม่ปล่อยให้ค้าง
// ---------------------------------------------------------------------------
enum CalStep : uint8_t {
  CAL_BASELINE = 0, CAL_DIRECTION, CAL_LEARN, CAL_VERIFY, CAL_DONE, CAL_STEP_COUNT
};

#define CAL_QUIET_MS        3000     // ขั้น 1 ต้องนิ่งต่อเนื่องเท่านี้จึงผ่าน
#define CAL_PASS_SHOW_MS    900      // ค้างผลที่ผ่านไว้ให้เห็นก่อนขึ้นขั้นถัดไป
                                     // ขั้นที่ผ่านแล้ววาบหายไปทันที ไม่ได้บอกอะไรใคร
#define CAL_NOISE_MAX       12       // สัญญาณรบกวนสูงกว่านี้ = วางไม่นิ่งหรือมีแสงกวน
#define CAL_RAW_MIN         40       // ต่ำกว่านี้ = สายหลุดหรือเซนเซอร์ตัน
#define CAL_RAW_MAX         4050     // สูงกว่านี้ = ADC ชนเพดาน ไม่เหลือช่วงให้วัดหยด
#define CAL_LEARN_DROPS     10
#define CAL_VERIFY_DROPS    10
#define CAL_SNR_MIN_X10     60       // 6.0 เท่า — ต่ำกว่านี้นับพลาดง่ายเมื่อคนไข้ขยับ
#define CAL_DIR_DROPS       3        // เห็นหยดอย่างน้อยเท่านี้ก่อนจึงสรุปทิศ
#define CAL_DIR_MIN_DEV     40       // ส่วนเบี่ยงเบนจากเส้นฐานที่ถือว่าเป็นหยดจริง (ADC)
#define CAL_DIR_RATIO       3        // ทางที่ชนะต้องมากกว่าอีกทางกี่เท่าจึงสรุปได้
#define CAL_CONF_MIN        60       // ความสม่ำเสมอของช่วงหยด (%)
// เวลารอของแต่ละขั้น — ขั้นที่ต้องรอหยดกินเวลาตามอัตราไหลของผู้ป่วย ไม่ใช่ตามเรา
// ที่ 20 mL/h กับ drop factor 20 คือ 9 วินาทีต่อหยด สิบหยดจึงใช้เวลาราว 90 วินาที
// ตั้งไว้ 60 วินาทีเท่ากันทุกขั้นแบบเดิมจะทำให้เตียงที่ให้ยาช้า ๆ ไม่มีวันผ่าน
#define CAL_QUIET_TIMEOUT_MS   20000
#define CAL_DROP_TIMEOUT_MS   180000

CalStep  calStep          = CAL_BASELINE;
unsigned long calStepStart = 0;
unsigned long calOkSince   = 0;      // เริ่มเข้าเกณฑ์ของขั้นนี้เมื่อไร (0 = ยังไม่เข้า)
uint32_t calDropsAtStep    = 0;      // จำนวนหยดสะสมของตัวตรวจจับตอนเข้าขั้นนี้
uint32_t calRejectsAtStep  = 0;
bool     calFailed         = false;  // ขั้นนี้หมดเวลาแล้ว รอให้กดลองใหม่
const char *calFailHint    = "";
int      calResultSnrX10   = 0;      // เก็บไว้โชว์ในหน้าสรุป

// ---- ขั้นที่ 2 วัดทิศสัญญาณจากค่าดิบเอง ----
//
// เดิมคิดจะอ่านผลจาก detector.polarityLocked() แต่กลไกนั้นเป็นการ "ลองแล้ววัดผล"
// ซึ่งให้เวลาทิศละ 10 วินาที ครบแล้วยังจับจังหวะหยดไม่ได้ก็สลับทิศ ที่อัตราไหล
// ปกติกว่าจะจับจังหวะได้สม่ำเสมอสี่ช่วงต้องใช้ราว 5 หยด = เกิน 10 วินาที มันจึง
// สลับไปทิศที่ผิดก่อน แล้ว "ล็อก" ทิศผิดนั้นไว้ ตัวเลขยังนับได้เพราะไปเกาะไหล่
// ของพัลส์แทนตัวพัลส์ แต่ความสูงที่วัดได้เหลือครึ่งเดียว SNR จึงตกลงครึ่งหนึ่ง
// (เจอตอนเรนเดอร์หน้าจอจำลอง ขั้น 2 ขึ้น DROP HI ทั้งที่หยดทำให้ค่าลดลง)
//
// ขั้นนี้จึงวัดเอง เทียบค่าดิบกับค่าเฉลี่ยของตัวเอง แล้วดูว่าเบี่ยงไปทางไหนมากกว่า
// ได้คำตอบใน 3 หยด และไม่ขึ้นกับว่าตัวตรวจจับจะจับจังหวะได้เมื่อไร
float    calRawMean       = 0.0f;
uint32_t calRawN          = 0;
int      calDipMax        = 0;       // เบี่ยงลงมากสุด (หยดทำให้ค่าลดลง)
int      calBumpMax       = 0;       // เบี่ยงขึ้นมากสุด (หยดทำให้ค่าสูงขึ้น)
int8_t   calDirSign       = 0;       // 0 = ยังสรุปไม่ได้ · -1 = ลดลง · +1 = สูงขึ้น

UiPage   currentPage          = PAGE_RATE;
UiMode   uiMode               = MODE_NORMAL;
uint8_t  uiAlertCode          = ALERT_NONE;
unsigned long lastUserActivity = 0;
bool     screenNeedsRedraw    = true;

bool     lastBtnState         = HIGH;
unsigned long btnPressStart   = 0;
unsigned long lastReleaseTime = 0;
uint8_t  clickCount           = 0;
bool     longHandled          = false;
bool     veryLongHandled      = false;

// ---- ประกาศล่วงหน้า (ฟังก์ชันที่ถูกเรียกก่อนบรรทัดที่นิยามไว้) ----
// ฟังก์ชันวาดจอที่อยู่ใน StationScreen.h แต่ถูกเรียกจากโค้ดเหนือบรรทัด include
void drawScreen();
void drawSplash();

// ---------------------------------------------------------------------------
// ออด — คิวจังหวะแบบไม่บล็อก
// ของเดิมใช้ delay() ทำให้ลูปหยุด ตรวจจับหยดไม่ทันระหว่างมีเสียง
// ---------------------------------------------------------------------------
// เรียกได้ทุกที่ รวมถึงก่อนสมัครสมาชิก — ถ้ายังไม่ได้สมัครจะคืนค่าผิดพลาดเฉย ๆ
inline void feedWatchdog() {
  esp_task_wdt_reset();
}

void setupLoopWatchdog() {
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  esp_task_wdt_config_t wdtCfg = {};
  wdtCfg.timeout_ms     = LOOP_WDT_TIMEOUT_S * 1000;
  wdtCfg.idle_core_mask = 0;            // ไม่เฝ้างานว่าง เฝ้าเฉพาะ loop() ของเรา
  wdtCfg.trigger_panic  = true;         // ค้างจริง = รีบูต ดีกว่าค้างเงียบต่อไป
  // core 3.x เปิดตัวเฝ้าไว้ให้แล้วในบางการตั้งค่า จึงต้องเผื่อทางตั้งค่าใหม่ด้วย
  if (esp_task_wdt_init(&wdtCfg) == ESP_ERR_INVALID_STATE) esp_task_wdt_reconfigure(&wdtCfg);
#else
  esp_task_wdt_init(LOOP_WDT_TIMEOUT_S, true);
#endif
  esp_task_wdt_add(NULL);
}

// ปิดการเฝ้าก่อนเข้าโหมดหลับ มิฉะนั้นการรอให้ปล่อยปุ่มจะถูกนับว่าค้าง
void stopLoopWatchdog() {
  esp_task_wdt_delete(NULL);
}

// ข้อความสั้น ๆ ไว้แสดงในหน้าเว็บและใน Serial เพื่อให้ตามรอยปัญหาในวอร์ดจริงได้
const char* resetReasonText(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:  return "power-on";
    case ESP_RST_EXT:      return "external";
    case ESP_RST_SW:       return "software";
    case ESP_RST_PANIC:    return "panic";
    case ESP_RST_INT_WDT:  return "int-wdt";
    case ESP_RST_TASK_WDT: return "task-wdt";
    case ESP_RST_WDT:      return "other-wdt";
    case ESP_RST_DEEPSLEEP:return "deep-sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO:     return "sdio";
    default:               return "unknown";
  }
}

void beepPattern(uint8_t times, uint16_t onMs, uint16_t offMs) {
  beepRemaining = times;
  beepOnMs      = onMs;
  beepOffMs     = offMs;
  beepIsOn      = false;
  beepNextChange = 0;          // ให้ serviceBeep เริ่มทันทีในรอบถัดไป
}

void serviceBeep(unsigned long now) {
  if (beepRemaining == 0 && !beepIsOn) { digitalWrite(BUZZER_PIN, LOW); return; }
  if (now < beepNextChange) return;

  if (!beepIsOn) {
    if (beepRemaining == 0) { digitalWrite(BUZZER_PIN, LOW); return; }
    digitalWrite(BUZZER_PIN, HIGH);
    beepIsOn = true;
    beepNextChange = now + beepOnMs;
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    beepIsOn = false;
    beepRemaining--;
    beepNextChange = now + beepOffMs;
  }
}

void soundClick()    { beepPattern(1, 20,  40); }
void soundStart()    { beepPattern(2, 35,  45); }
void soundStop()     { beepPattern(1, 140, 40); }
void soundConfirm()  { beepPattern(2, 60,  50); }
void soundAlert()    { beepPattern(3, 90,  70); }

// ---------------------------------------------------------------------------
// ค่าตั้งถาวรใน NVS
// ---------------------------------------------------------------------------
void saveStationId() {
  stationPrefs.begin("c3_cfg", false);
  stationPrefs.putUChar("bed", stationId);
  stationPrefs.end();
}

void saveChannel() {
  stationPrefs.begin("c3_cfg", false);
  stationPrefs.putUChar("ch", savedChannel);
  stationPrefs.end();
}

void loadConfig() {
  stationPrefs.begin("c3_cfg", true);
  stationId    = stationPrefs.getUChar("bed", 1);
  savedChannel = stationPrefs.getUChar("ch",  DEFAULT_ESPNOW_CHANNEL);
  stationPrefs.end();
  if (stationId < 1 || stationId > 8) stationId = 1;
  if (savedChannel < 1 || savedChannel > MAX_WIFI_CHANNEL) savedChannel = DEFAULT_ESPNOW_CHANNEL;
  espnowChannel = savedChannel;
}

// ---------------------------------------------------------------------------
// แบตเตอรี่
// ---------------------------------------------------------------------------
float readBattery() {
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(BAT_ADC_PIN);
  return ((sum / 8.0f) / 4095.0f) * 3.3f * 2.0f;
}

uint8_t batteryPct() {
  if (batteryVolts < 0.5f) return 0;                  // เสียบ USB ไม่มีแบตเตอรี่
  float p = (batteryVolts - 3.30f) / (4.20f - 3.30f) * 100.0f;
  if (p < 0)   p = 0;
  if (p > 100) p = 100;
  return (uint8_t)p;
}

// ---------------------------------------------------------------------------
// หน้าต่างคำนวณอัตราไหล — ยกมาจากสายหลักทั้งก้อน
//
// เหตุผลที่ไม่ใช้วิธีเดิม: การเฉลี่ย 1/Δt มีความลำเอียงสูงเสมอ (อสมการเจนเซน)
// และการนับหยดในหน้าต่าง 2.5 วินาทีที่อัตราต่ำจะได้ 0 หรือ 1 หยด ตัวเลขจึงกระโดด
// วิธีนี้วัด "ช่วงเวลาที่ครอบหลายหยด" แล้วหารครั้งเดียว จึงนิ่งและไม่ลำเอียง
// ---------------------------------------------------------------------------
void resetRateWindow() { rateWinIdx = 0; rateWinCount = 0; }

void pushRateSample(unsigned long t) {
  rateWinMs[rateWinIdx] = t;
  rateWinIdx = (rateWinIdx + 1) % RATE_WINDOW_DROPS;
  if (rateWinCount < RATE_WINDOW_DROPS) rateWinCount++;
}

unsigned long rateWindowSpanMs() {
  if (rateWinCount < 2) return 0;
  uint8_t newest = (rateWinIdx + RATE_WINDOW_DROPS - 1) % RATE_WINDOW_DROPS;
  uint8_t oldest = (rateWinIdx + RATE_WINDOW_DROPS - rateWinCount) % RATE_WINDOW_DROPS;
  return rateWinMs[newest] - rateWinMs[oldest];
}

unsigned long rateWindowMeanIntervalMs() {
  unsigned long span = rateWindowSpanMs();
  if (span == 0) return 0;
  return span / (rateWinCount - 1);
}

// ----------------------------------------------------------------------------
// เก็บจำนวนหยดสะสมให้รอดการรีบูต ([1.1.0-C3] เพิ่มทั้งหมด)
//
// เดิมตัวนับอยู่ใน RAM อย่างเดียว ไฟตกครึ่งวินาทีเดียวก็กลับเป็นศูนย์ ตัวเลข
// "ให้ไปแล้วกี่ mL" จึงผิดไปทั้งถุง และการเตือนใกล้หมด/ให้ครบตามแผนจะมาช้ากว่า
// ความจริงเท่ากับปริมาตรที่หายไป ซึ่งมีผลกับผู้ป่วยโดยตรง
//
// เก็บสองชั้นเพราะข้อจำกัดคนละแบบ
//   RTC — เขียนได้ไม่จำกัด จึงเขียนทุกรอบ รอดรีเซ็ตซอฟต์แวร์ วอตช์ด็อก ไฟตกชั่วขณะ
//   NVS — รอดไฟดับสนิท แต่แฟลชมีอายุการเขียนจำกัด จึงเขียนห่าง ๆ ทุก 5 นาที
// ตอนบูตใช้ค่าที่มากกว่าของสองชั้น และต้องเป็นของเตียงเดียวกันเท่านั้น
// ----------------------------------------------------------------------------
#define TALLY_NVS_SAVE_MS   300000UL
#define TALLY_RTC_MAGIC     0x49565431UL      // 'IVT1' ลายเซ็นบอกว่าค่าใน RTC ใช้ได้

RTC_DATA_ATTR uint32_t rtcTallyMagic = 0;
RTC_DATA_ATTR uint32_t rtcTallyDrops = 0;
RTC_DATA_ATTR uint32_t rtcTallyBed   = 0;

uint32_t      tallySavedToNvs   = 0;
unsigned long tallyNextNvsSave  = 0;
bool          tallyRestoredBoot = false;      // บูตนี้กู้ยอดเก่ากลับมา (ไว้บอกพยาบาล)

void tallyRemember(uint32_t drops, uint8_t bed) {
  rtcTallyMagic = TALLY_RTC_MAGIC;
  rtcTallyDrops = drops;
  rtcTallyBed   = bed;
}

void tallyWriteNvs(uint32_t drops, uint8_t bed) {
  stationPrefs.begin("c3_tally", false);
  stationPrefs.putUInt("drops", drops);
  stationPrefs.putUChar("bed", bed);
  stationPrefs.end();
  tallySavedToNvs = drops;
}

// เริ่มถุงใหม่ = ล้างทั้งสองชั้นทันที ไม่งั้นบูตครั้งหน้าจะกู้ยอดของถุงเก่ากลับมา
void tallyClear(uint8_t bed) {
  tallyRemember(0, bed);
  tallyWriteNvs(0, bed);
  tallyRestoredBoot = false;
}

uint32_t tallyRestore(uint8_t bed) {
  uint32_t best = 0;
  if (rtcTallyMagic == TALLY_RTC_MAGIC && rtcTallyBed == bed) best = rtcTallyDrops;
  stationPrefs.begin("c3_tally", true);
  uint32_t nv    = stationPrefs.getUInt("drops", 0);
  uint8_t  nvBed = stationPrefs.getUChar("bed", 0);
  stationPrefs.end();
  if (nvBed == bed && nv > best) best = nv;      // RTC หายเพราะไฟดับสนิท จึงถอยมาใช้ NVS
  tallySavedToNvs   = best;
  tallyRestoredBoot = (best > 0);
  tallyNextNvsSave  = millis() + TALLY_NVS_SAVE_MS;
  return best;
}

void serviceTallySave(unsigned long now, uint32_t drops, uint8_t bed) {
  tallyRemember(drops, bed);                     // ชั้นเร็ว ไม่กินอายุแฟลช
  if ((long)(now - tallyNextNvsSave) < 0) return;
  tallyNextNvsSave = now + TALLY_NVS_SAVE_MS;
  if (drops != tallySavedToNvs) tallyWriteNvs(drops, bed);
}

uint8_t safeDropFactor(uint8_t df) { return (df >= 10 && df <= 60) ? df : DEFAULT_DROP_FACTOR; }

float rateFromWindow(uint8_t df) {
  unsigned long span = rateWindowSpanMs();
  if (span == 0 || df == 0) return 0.0f;
  float dropsInSpan = (float)(rateWinCount - 1);
  return (dropsInSpan / (float)df) * 3600000.0f / (float)span;
}

// ---------------------------------------------------------------------------
// เมื่อนับได้หนึ่งหยด
// ---------------------------------------------------------------------------
void onDropDetected() {
  unsigned long now = millis();
  totalDrops++;
  periodDropsCounter++;
  sensorHasDrops = true;
  dropFlashUntil = now + DROP_FLASH_MS;

  uint8_t df = safeDropFactor(dropFactor);
  unsigned long minDropInterval = 60;

  if (!hasFirstDropOccurred || skipNextInterval) {
    resetRateWindow();            // เริ่มถุงใหม่/กลับจากหยุดชั่วคราว
  } else {
    unsigned long dropDeltaMs = now - lastDropTimestamp;
    unsigned long meanIv      = rateWindowMeanIntervalMs();
    if (dropDeltaMs <= minDropInterval || dropDeltaMs >= MAX_OCCLUSION_MS) {
      resetRateWindow();          // ช่วงหยดผิดปกติ ไม่เอามาปนกับของเดิม
    } else if (meanIv > 0 && (dropDeltaMs > meanIv * 3 || dropDeltaMs * 3 < meanIv)) {
      resetRateWindow();          // อัตราเปลี่ยนก้าวกระโดด (หมุนโรลเลอร์แคลมป์)
    }
  }

  pushRateSample(now);
  lastDropTimestamp     = now;
  hasFirstDropOccurred  = true;
  skipNextInterval      = false;

  currentFlowRate_ml_hr = rateFromWindow(df);
  currentGttMin         = currentFlowRate_ml_hr * df / 60.0f;
  totalVolumeMl         = (float)totalDrops / (float)df;

  screenNeedsRedraw = true;
}

bool serviceDropSensor(bool countDrops) {
  sensorRaw = analogRead(SENSOR_AO_PIN);
  bool got = detector.update(sensorRaw, micros());
  if (got && countDrops) onDropDetected();
  return got;
}

const iv::DetectorStatus &det() { return detector.status(); }

const char* sensorStateWord() {
  switch (det().lock) {
    case iv::Lock::Ready:    return "READY";
    case iv::Lock::Learning: return "LEARN";
    case iv::Lock::NoDrops:  return "CHECK";
    default:                 return "START";
  }
}

// ---------------------------------------------------------------------------
// เก็บสิ่งที่เรียนรู้ไว้ใน NVS ([1.2.0-C3] เพิ่ม)
// เปิดเครื่องมาแล้วพร้อมนับทันที ไม่ต้องรอเรียนใหม่ทุกครั้ง
// ---------------------------------------------------------------------------
void saveCalibration() {
  stationPrefs.begin("c3_cal", false);
  stationPrefs.putInt("lamp", det().learnedAmp);
  stationPrefs.putInt("lwid", det().learnedWidthMs);
  stationPrefs.putInt("pol",  detector.polarity());
  stationPrefs.end();
}

void loadCalibration() {
  stationPrefs.begin("c3_cal", true);
  int amp = stationPrefs.getInt("lamp", 0);
  int wid = stationPrefs.getInt("lwid", 0);
  int pol = stationPrefs.getInt("pol",  0);
  stationPrefs.end();
  if (pol == 1 || pol == -1) detector.setPolarity((int8_t)pol);
  else                       detector.setPolarityAuto();
  detector.seedLearning(amp, wid);
}

// ---------------------------------------------------------------------------
// ตัวช่วยคาลิเบรต — ตรรกะของทั้งสี่ขั้น ([1.2.0-C3] เพิ่มทั้งหมด)
// ---------------------------------------------------------------------------
const char* calStepName(CalStep s) {
  switch (s) {
    case CAL_BASELINE:  return "BASELINE";
    case CAL_DIRECTION: return "DIRECTION";
    case CAL_LEARN:     return "LEARN";
    case CAL_VERIFY:    return "VERIFY";
    default:            return "DONE";
  }
}

// บรรทัดบอกว่าต้องทำอะไรกับสาย ไม่ใช่บอกว่าเครื่องกำลังทำอะไร
const char* calStepHint(CalStep s) {
  switch (s) {
    case CAL_BASELINE:  return "close clamp";
    case CAL_DIRECTION: return "open clamp";
    case CAL_LEARN:     return "let it drip";
    case CAL_VERIFY:    return "keep dripping";
    default:            return "saved";
  }
}

// เริ่มขั้นใหม่ — จำจุดตั้งต้นไว้ เพื่อให้นับเฉพาะหยดของขั้นนี้
void calBeginStep(CalStep s, unsigned long now) {
  // เข้าขั้นหาทิศ = เริ่มจับเวลาลองทิศใหม่ตั้งแต่จังหวะที่เปิดโรลเลอร์แคลมป์
  //
  // ตัวตรวจจับให้เวลาลองทิศละ 10 วินาที ครบแล้วยังจับจังหวะหยดไม่ได้ก็สลับทิศ
  // ถ้าไม่ตั้งต้นใหม่ตรงนี้ เวลาส่วนนั้นจะถูกขั้นที่ 1 (ปิดแคลมป์ให้นิ่ง 3 วินาที)
  // กินไปก่อน เหลือเวลาไม่พอให้เห็นหยดครบห้าหยด ตัวตรวจจับจึงสลับไปทิศที่ผิด
  // ทั้งที่ยังไม่เคยเห็นหยดเลยสักหยด แล้วล็อกทิศผิดนั้นไว้
  // (เจอตอนเรนเดอร์หน้าจอจำลอง ขั้นที่ 2 ขึ้นว่า DROP HI ทั้งที่หยดทำให้ค่าลดลง)
  if (s == CAL_DIRECTION) {
    calRawMean = (float)analogRead(SENSOR_AO_PIN);
    calRawN = 1; calDipMax = 0; calBumpMax = 0; calDirSign = 0;
  }
  // เข้าขั้นเรียนรู้ = ล้างสิ่งที่เรียนไว้ตอนยังไม่รู้ทิศทิ้ง แล้วเรียนใหม่ด้วยทิศที่ถูก
  // (detector.reset ไม่แตะทิศ จึงคงทิศที่เพิ่งวัดได้ไว้)
  if (s == CAL_LEARN) detector.reset(analogRead(SENSOR_AO_PIN));
  calStep         = s;
  calStepStart    = now;
  calOkSince      = 0;
  calFailed       = false;
  calFailHint     = "";
  calDropsAtStep  = det().drops;
  calRejectsAtStep = det().rejects;
  screenNeedsRedraw = true;
}

uint32_t calDropsThisStep() {
  uint32_t d = det().drops;
  return (d >= calDropsAtStep) ? (d - calDropsAtStep) : 0;
}

void enterCalibration(unsigned long now) {
  uiMode = MODE_CALIB;
  // ล้างสิ่งที่เรียนไว้เดิมทิ้ง มิฉะนั้นขั้น 2 จะผ่านทันทีโดยไม่ได้วัดอะไรเลย
  detector.reset(analogRead(SENSOR_AO_PIN));
  detector.setPolarityAuto();
  calResultSnrX10 = 0;
  calBeginStep(CAL_BASELINE, now);
  beepPattern(2, 60, 60);
}

// ออกจากตัวช่วย — ผ่านครบจึงบันทึก ออกกลางคันให้คืนค่าเดิมที่เคยบันทึกไว้
void exitCalibration(bool save) {
  if (save) { saveCalibration(); soundConfirm(); }
  else      { loadCalibration(); soundClick();   }
  uiMode = MODE_NORMAL;
  currentPage = PAGE_RATE;
  screenNeedsRedraw = true;
}

// เงื่อนไขผ่านของแต่ละขั้น — แยกออกมาเพื่อให้อ่านทีละข้อได้
// คืน true เมื่อ "ขณะนี้เข้าเกณฑ์" ส่วนการนับเวลาค้างเกณฑ์อยู่ที่ผู้เรียก
bool calStepPassing(CalStep s) {
  const iv::DetectorStatus &d = det();
  switch (s) {
    case CAL_BASELINE:
      // สายหลุดหรือ ADC ชนเพดาน = วัดอะไรไม่ได้เลย ต้องบอกก่อนไปขั้นถัดไป
      if (sensorRaw < CAL_RAW_MIN || sensorRaw > CAL_RAW_MAX) return false;
      if (d.noiseSigma > CAL_NOISE_MAX) return false;
      return calDropsThisStep() == 0;          // ปิดแคลมป์แล้วต้องไม่มีหยดหลุดมา
    case CAL_DIRECTION:
      return calDirSign != 0 && calDropsThisStep() >= CAL_DIR_DROPS;
    case CAL_LEARN:
      return calDropsThisStep() >= CAL_LEARN_DROPS &&
             d.learnedAmp > 0 && d.snrX10 >= CAL_SNR_MIN_X10;
    case CAL_VERIFY:
      return calDropsThisStep() >= CAL_VERIFY_DROPS &&
             d.confidencePct >= CAL_CONF_MIN &&
             (d.rejects - calRejectsAtStep) <= 1;
    default:
      return true;
  }
}

// สาเหตุที่ขั้นนี้ไม่ผ่าน เขียนเป็นสิ่งที่พยาบาลลงมือแก้ได้ ไม่ใช่ศัพท์เทคนิค
const char* calFailReason(CalStep s) {
  const iv::DetectorStatus &d = det();
  switch (s) {
    case CAL_BASELINE:
      if (sensorRaw < CAL_RAW_MIN)  return "check wiring";
      if (sensorRaw > CAL_RAW_MAX)  return "too bright";
      if (d.noiseSigma > CAL_NOISE_MAX) return "hold still";
      return "clamp leaks";
    case CAL_DIRECTION:
      if (calDropsThisStep() < CAL_DIR_DROPS) return "no drops seen";
      return "signal unclear";
    case CAL_LEARN:
      if (calDropsThisStep() < CAL_LEARN_DROPS) return "too few drops";
      return "weak signal";
    case CAL_VERIFY:
      if (calDropsThisStep() < CAL_VERIFY_DROPS) return "too few drops";
      return "uneven timing";
    default: return "";
  }
}

// เดินตรรกะของตัวช่วยหนึ่งรอบ เรียกจาก loop() เฉพาะตอนอยู่ในโหมดนี้
// เดินตัววัดทิศหนึ่งตัวอย่าง — พัลส์หยดกินเวลาไม่ถึง 1% ของช่วงหยด
// ค่าเฉลี่ยจึงเป็นเส้นฐานที่เชื่อถือได้ โดยไม่ต้องกรองอะไรเพิ่ม
void calTrackDirection() {
  if (calRawN < 4000) calRawN++;                       // ~2 วินาทีแรกเฉลี่ยไว ๆ แล้วนิ่ง
  calRawMean += ((float)sensorRaw - calRawMean) / (float)calRawN;

  int dev = sensorRaw - (int)calRawMean;
  if (dev < 0 && -dev > calDipMax)  calDipMax  = -dev;
  if (dev > 0 &&  dev > calBumpMax) calBumpMax =  dev;

  int big   = (calDipMax > calBumpMax) ? calDipMax  : calBumpMax;
  int small = (calDipMax > calBumpMax) ? calBumpMax : calDipMax;
  if (big < CAL_DIR_MIN_DEV) return;                   // ยังไม่เห็นอะไรที่ใหญ่พอ
  if (big < small * CAL_DIR_RATIO) return;             // สองทางพอกัน = สรุปไม่ได้
  calDirSign = (calDipMax > calBumpMax) ? -1 : 1;
}

uint32_t calStepTimeoutMs(CalStep s) {
  return (s == CAL_BASELINE) ? CAL_QUIET_TIMEOUT_MS : CAL_DROP_TIMEOUT_MS;
}

void serviceCalibration(unsigned long now) {
  if (calStep == CAL_DONE || calFailed) return;
  if (calStep == CAL_DIRECTION) calTrackDirection();

  if (calStepPassing(calStep)) {
    if (calOkSince == 0) calOkSince = now;
    // ขั้นแรกต้องนิ่งค้างไว้จริง ๆ ขั้นอื่นค้างแค่พอให้เห็นผลที่เพิ่งผ่าน
    unsigned long need = (calStep == CAL_BASELINE) ? CAL_QUIET_MS : CAL_PASS_SHOW_MS;
    if (now - calOkSince >= need) {
      // ยืนยันทิศให้ตัวตรวจจับตรง ๆ (setPolarity ปิดโหมดลองผิดลองถูกไปในตัว)
      if (calStep == CAL_DIRECTION) detector.setPolarity(calDirSign);
      if (calStep == CAL_LEARN) calResultSnrX10 = det().snrX10;
      if (calStep == CAL_VERIFY) {
        calStep = CAL_DONE;
        screenNeedsRedraw = true;
        beepPattern(3, 60, 60);
        return;
      }
      calBeginStep((CalStep)(calStep + 1), now);
      beepPattern(1, 40, 0);
    }
    return;
  }

  calOkSince = 0;
  if (now - calStepStart >= calStepTimeoutMs(calStep)) {
    calFailed   = true;
    calFailHint = calFailReason(calStep);
    screenNeedsRedraw = true;
    beepPattern(2, 200, 120);
  }
}

// ---------------------------------------------------------------------------
// สถานะที่จอต้องรู้
// ---------------------------------------------------------------------------
bool isHostOnline() { return lastHostSyncTime > 0 && (millis() - lastHostSyncTime) < HOST_TIMEOUT_MS; }

unsigned long msSinceLastDrop() {
  if (!hasFirstDropOccurred) return 0;
  return millis() - lastDropTimestamp;
}

unsigned long occlusionThresholdMs() {
  unsigned long meanIv = rateWindowMeanIntervalMs();
  if (meanIv == 0) return MAX_OCCLUSION_MS;
  unsigned long th = meanIv * 5 / 2;               // 2.5 เท่าของช่วงหยดปกติ
  if (th < MIN_OCCLUSION_MS) th = MIN_OCCLUSION_MS;
  if (th > MAX_OCCLUSION_MS) th = MAX_OCCLUSION_MS;
  return th;
}

// รหัสเตือนที่จะเอาไปขึ้นจอ — รวมของ Host กับที่เครื่องตรวจเองได้
uint8_t effectiveAlert() {
  if (!isRunning) return ALERT_NONE;
  uint8_t c = hostAlertCode;

  // ข้อมูลในเครื่องใหม่กว่าเสมอ ถ้าหยดยังมาอยู่ก็ไม่ใช่การอุดตัน
  if (c == ALERT_OCCLUSION && msSinceLastDrop() < occlusionThresholdMs()) c = ALERT_NONE;
  if (c != ALERT_NONE) return c;

  // Host เงียบอยู่ แต่เครื่องเห็นเองว่าหยดหยุดไปนานผิดปกติ
  if (hasFirstDropOccurred && msSinceLastDrop() > occlusionThresholdMs()) return ALERT_OCCLUSION;
  return ALERT_NONE;
}

// ---------------------------------------------------------------------------
// ESP-NOW
// ---------------------------------------------------------------------------
void OnHostSyncRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(struct_host_sync)) return;      // ไม่ใช่แพ็กเก็ตของเรา

  struct_host_sync s;
  memcpy(&s, data, sizeof(s));

  lastHostSyncTime = millis();                      // ได้ยิน Host = ช่องนี้ถูกแล้ว

  if (s.epochTime > 1600000000UL) {
    struct timeval tv = { (time_t)s.epochTime, 0 };
    settimeofday(&tv, NULL);
    isClockSynced = (s.isSynced == 1);
  }

  if (s.stationId != stationId) return;             // คำสั่งของเตียงอื่น

  targetRateHr = s.targetRateHr;
  totalPlanMl  = s.totalPlanMl;
  dropFactor   = safeDropFactor(s.dropFactor);
  nearEndPct   = (s.nearEndPct >= 10 && s.nearEndPct <= 99) ? s.nearEndPct : DEFAULT_NEAR_END_PCT;
  hostAlertCode = s.alertCode;
  hostAckedNearEnd = (s.flags & 0x01) != 0;
  if (hostAckedNearEnd) nearEndAckRequest = false;  // Host รับทราบแล้ว เลิกขอ

  // Host สั่งเริ่มถุงใหม่
  if (s.resetSeq != lastResetSeq) {
    lastResetSeq = s.resetSeq;
    totalDrops = 0;
    tallyClear(stationId);   // [1.1.0-C3] เพิ่ม: เริ่มถุงใหม่ = ล้างยอดที่เก็บไว้ด้วย
    periodDropsCounter = 0;
    totalVolumeMl = 0.0f;
    currentFlowRate_ml_hr = 0.0f;
    currentGttMin = 0.0f;
    hasFirstDropOccurred = false;
    resetRateWindow();
    nearEndAckRequest = false;
    beepPattern(2, 40, 50);
  }
}

void setEspNowChannel(uint8_t ch) {
  if (esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE) == ESP_OK) espnowChannel = ch;
}

// ไล่หาช่องของ Host เมื่อไม่ได้ยินเสียงตอบ แล้วจำช่องที่เจอไว้
// จำเป็นตั้งแต่ Host v4.7.6 เป็นต้นไป เพราะ Host ไปใช้ช่องของเราเตอร์
void manageChannelHunting(unsigned long now) {
  if (isHostOnline()) {
    if (espnowChannel != savedChannel) { savedChannel = espnowChannel; saveChannel(); }
    return;
  }
  if (now < CHANNEL_SCAN_START_MS) return;          // ให้โอกาสช่องที่จำไว้ก่อน
  if (uiMode == MODE_SET_ID) return;                // กำลังตั้งเลขเตียง อย่าเพิ่งกวน

  if (now - lastChannelHopTime >= CHANNEL_SCAN_DWELL_MS) {
    lastChannelHopTime = now;
    setEspNowChannel((espnowChannel % MAX_WIFI_CHANNEL) + 1);
  }
}

void sendToHost() {
  // [1.2.0-C3] แก้: ระหว่างคาลิเบรตต้องรายงานว่า "ไม่ได้นับอยู่"
  //
  // ขั้นที่ 1 ให้ปิดโรลเลอร์แคลมป์ ถ้ายังบอก Host ว่ากำลังให้น้ำเกลืออยู่ Host จะ
  // เห็นว่าหยดหยุดไปแล้วปลุกเสียงเตือน "สายพับ" ขึ้นมาทั้งวอร์ด ทั้งที่พยาบาล
  // กำลังยืนปรับเซนเซอร์อยู่ตรงนั้นเอง ใช้ความหมายเดิมของโปรโตคอล ไม่ต้องเพิ่มรหัสใหม่
  bool countingNow       = isRunning && uiMode != MODE_CALIB;
  myData.stationId       = stationId;
  myData.isRunning       = countingNow ? 1 : 0;
  myData.totalDrops      = totalDrops;
  myData.periodDrops     = countingNow ? periodDropsCounter : 0;
  myData.flowRateHr      = countingNow ? currentFlowRate_ml_hr : 0.0f;
  myData.msSinceLastDrop = msSinceLastDrop();
  myData.batteryVolts    = batteryVolts;
  myData.flags           = nearEndAckRequest ? 0x01 : 0x00;

  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  periodDropsCounter = 0;
}

// ---------------------------------------------------------------------------
// ปุ่มกด
//   กดสั้น 1 ครั้ง  = เปลี่ยนหน้า (หรือรับทราบเตือนใกล้หมดเมื่ออยู่หน้าเตือน)
//   กดสั้น 2 ครั้ง  = เริ่ม/หยุดชั่วคราว
//   กดค้าง 1.5 วิ  = กลับหน้าแรก
//   กดค้าง 4 วิ    = เข้าหน้าตั้งเลขเตียง (ในหน้านั้น กดสั้นคือเพิ่มเลข กดค้างคือบันทึก)
// ---------------------------------------------------------------------------
void togglePause() {
  isRunning = !isRunning;
  if (isRunning) {
    soundStart();
    skipNextInterval = true;      // ช่วงที่หยุดไปไม่ใช่การไหล อย่าเอามาคิด
  } else {
    soundStop();
    currentFlowRate_ml_hr = 0.0f;
    currentGttMin = 0.0f;
  }
  currentPage = PAGE_RATE;
  screenNeedsRedraw = true;
}

void handleButton(unsigned long now) {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading == LOW && lastBtnState == HIGH) {
    btnPressStart    = now;
    longHandled      = false;
    veryLongHandled  = false;
  }

  if (reading == LOW) {
    unsigned long held = now - btnPressStart;

    if (held >= 4000 && !veryLongHandled && uiMode != MODE_CALIB) {   // [1.2.0-C3] แก้
      veryLongHandled = true;
      longHandled     = true;
      clickCount      = 0;
      if (uiMode == MODE_SET_ID) {        // อยู่ในหน้าตั้งเลขอยู่แล้ว = บันทึก
        saveStationId();
        uiMode = MODE_NORMAL;
        soundConfirm();
      } else {
        uiMode = MODE_SET_ID;
        beepPattern(3, 50, 45);
      }
      lastUserActivity  = now;
      screenNeedsRedraw = true;
    }
    else if (held >= 1500 && !longHandled && !veryLongHandled && uiMode != MODE_SET_ID) {
      longHandled = true;
      clickCount  = 0;
      // [1.2.0-C3] เพิ่ม: อยู่ในตัวช่วยคาลิเบรต = ออกโดยไม่บันทึก คืนค่าที่ใช้อยู่เดิม
      if (uiMode == MODE_CALIB) { exitCalibration(false); lastUserActivity = now; return; }
      currentPage = PAGE_RATE;
      uiMode      = MODE_NORMAL;
      soundClick();
      lastUserActivity  = now;
      screenNeedsRedraw = true;
    }
  }

  if (reading == HIGH && lastBtnState == LOW) {
    if (!longHandled && !veryLongHandled && (now - btnPressStart) > 40) {
      clickCount++;
      lastReleaseTime = now;
    }
  }

  if (clickCount > 0 && (now - lastReleaseTime > 320)) {
    lastUserActivity = now;
    // [1.2.0-C3] เพิ่ม: ปุ่มในตัวช่วยคาลิเบรต
    //   ขั้นที่ไม่ผ่าน -> กดเพื่อลองขั้นเดิมใหม่ (แก้สายแล้วไม่ต้องเริ่มจากขั้นแรก)
    //   หน้าสรุป      -> กดเพื่อบันทึกแล้วออก
    if (uiMode == MODE_CALIB) {
      if (calStep == CAL_DONE)   exitCalibration(true);
      else if (calFailed)      { calBeginStep(calStep, now); soundClick(); }
      else                       soundClick();
      clickCount = 0;
      screenNeedsRedraw = true;
      return;
    }
    if (uiMode == MODE_SET_ID) {
      stationId = (stationId % 8) + 1;             // กดสั้นในหน้าตั้งเลข = เพิ่มเลขเตียง
      tallyWriteNvs(totalDrops, stationId);        // [1.1.0-C3] เพิ่ม: ย้ายยอดสะสมไปใต้เลขเตียงใหม่
      soundClick();
    } else if (uiMode == MODE_SAVER) {
      uiMode = MODE_NORMAL;                         // ปลุกจากหน้าพักจอ
      soundClick();
    } else if (uiMode == MODE_ALERT && uiAlertCode == ALERT_NEAR_END) {
      nearEndAckRequest = true;                     // รับทราบที่เตียง ส่งกลับ Host
      soundConfirm();
    } else if (clickCount >= 3) {
      enterCalibration(now);       // [1.2.0-C3] เพิ่ม: กดสั้นสามครั้ง = เปิดตัวช่วยคาลิเบรต
    } else if (clickCount == 2) {
      togglePause();
    } else {
      currentPage = (UiPage)((currentPage + 1) % PAGE_COUNT);
      soundClick();
    }
    clickCount = 0;
    screenNeedsRedraw = true;
  }

  lastBtnState = reading;
}

// ---------------------------------------------------------------------------
// เลือกโหมดของหน้าจอตามสถานการณ์
// ---------------------------------------------------------------------------
void serviceUiMode(unsigned long now) {
  if (uiMode == MODE_SET_ID) return;                // กำลังตั้งค่า อย่าเด้งไปไหน
  if (uiMode == MODE_CALIB)  return;                // [1.2.0-C3] เพิ่ม: กำลังคาลิเบรต

  uint8_t a = effectiveAlert();

  // เตือนใกล้หมดที่รับทราบแล้ว ไม่ต้องเด้งหน้าเตือนซ้ำ
  if (a == ALERT_NEAR_END && (hostAckedNearEnd || nearEndAckRequest)) a = ALERT_NONE;

  if (a != ALERT_NONE) {
    if (uiMode != MODE_ALERT || uiAlertCode != a) soundAlert();
    uiAlertCode = a;
    uiMode      = MODE_ALERT;
    screenNeedsRedraw = true;
    return;
  }

  if (uiMode == MODE_ALERT) {                       // เหตุหายแล้ว กลับหน้าปกติ
    uiMode      = MODE_NORMAL;
    uiAlertCode = ALERT_NONE;
    currentPage = PAGE_RATE;
    screenNeedsRedraw = true;
  }

  if (now - lastUserActivity > SCREENSAVER_IDLE_MS) {
    if (uiMode != MODE_SAVER) { uiMode = MODE_SAVER; screenNeedsRedraw = true; }
  } else if (uiMode == MODE_SAVER) {
    uiMode = MODE_NORMAL;
    screenNeedsRedraw = true;
  }

  if (uiMode == MODE_NORMAL && currentPage != PAGE_RATE &&
      now - lastUserActivity > AUTO_HOME_MS) {
    currentPage = PAGE_RATE;                        // กลับหน้าแรกเองเมื่อทิ้งไว้
    screenNeedsRedraw = true;
  }
}

// ---------------------------------------------------------------------------
// โค้ดวาดจอทั้งหมดถูกแยกไปไว้ที่ไฟล์นี้ เพื่อให้ไฟล์หลักสั้นลงและหาของเจอเร็วขึ้น
// ต้อง #include ตรงนี้เท่านั้น คือหลังตัวแปรและฟังก์ชันช่วยทั้งหมด แต่ก่อน setup()
// ห้ามย้ายขึ้นไปบนสุด เพราะโค้ดในไฟล์นั้นใช้ตัวแปรและฟังก์ชันที่ประกาศไว้ด้านบน
// ---------------------------------------------------------------------------
#include "StationScreen.h"

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(50);

  pinMode(SENSOR_AO_PIN, INPUT);
  pinMode(BAT_ADC_PIN,   INPUT);
  pinMode(BUTTON_PIN,    INPUT_PULLUP);
  pinMode(BUZZER_PIN,    OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  setenv("TZ", "ICT-7", 1);
  tzset();

  loadConfig();
  // [1.1.0-C3] เพิ่ม: กู้จำนวนหยดสะสมของเตียงนี้กลับมา ไฟดับแล้วยอดต้องไม่หาย
  totalDrops    = tallyRestore(stationId);
  totalVolumeMl = (float)totalDrops / (float)safeDropFactor(dropFactor);

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  u8g2.begin();
  drawSplash();
  beepPattern(2, 40, 45);

  // ---- ตัวตรวจจับหยด ----
  iv::DetectorConfig dcfg;
  detector.begin(dcfg);
  detector.reset(analogRead(SENSOR_AO_PIN));
  loadCalibration();   // [1.2.0-C3] เพิ่ม: เอาค่าที่เคยคาลิเบรตไว้กลับมา พร้อมนับทันที

  // ---- วิทยุ ----
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  setEspNowChannel(savedChannel);

  if (esp_now_init() == ESP_OK) {
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnHostSyncRecv));
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;              // 0 = ใช้ช่องปัจจุบัน (รองรับการไล่หาช่อง)
    peerInfo.ifidx   = WIFI_IF_STA;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  
  bootResetReason = esp_reset_reason();   // [1.1.0-C3] เพิ่ม: จำไว้ว่ารีบูตครั้งล่าสุดเพราะอะไร
  Serial.printf("[boot] reset reason: %s\n", resetReasonText(bootResetReason));
  setupLoopWatchdog();
} else {
    Serial.println("[C3] ESP-NOW init FAILED");
  }

  myData.stationId = stationId;
  batteryVolts     = readBattery();
  sendJitter       = (uint16_t)random(0, SEND_JITTER_MS * 2);

  lastUserActivity = millis();
  screenNeedsRedraw = true;
}

void loop() {
  feedWatchdog();   // [1.1.0-C3] เพิ่ม: บอกสุนัขเฝ้าบ้านว่ายังเดินอยู่
  unsigned long now = millis();

  // อ่านเซนเซอร์ให้ถี่ที่สุด — ตัวตรวจจับจัดจังหวะตัวอย่างเอง
  // [1.2.0-C3] แก้: ระหว่างคาลิเบรตยังต้องอ่านเซนเซอร์ แต่ห้ามนับเข้ายอดของผู้ป่วย
  serviceDropSensor(isRunning && uiMode != MODE_SET_ID && uiMode != MODE_CALIB);
  if (uiMode == MODE_CALIB) serviceCalibration(now);

  serviceBeep(now);
  handleButton(now);
  serviceUiMode(now);
  manageChannelHunting(now);
  serviceTallySave(now, totalDrops, stationId);   // [1.1.0-C3] เพิ่ม: เก็บยอดสะสมให้รอดการรีบูต

  // ส่งข้อมูลให้ Host ทุก 1 วินาที บวกลบสุ่มเล็กน้อยกันสองเตียงชนกันค้างนาน
  if (now - lastSendTime >= (unsigned long)(SEND_INTERVAL_MS - SEND_JITTER_MS + sendJitter)) {
    lastSendTime = now;
    sendJitter   = (uint16_t)random(0, SEND_JITTER_MS * 2);
    batteryVolts = readBattery();

    // หยดหยุดไปนานแล้ว ตัวเลขบนจอต้องไหลลงหาศูนย์ ไม่ใช่ค้างค่าเดิม
    if (isRunning && hasFirstDropOccurred && msSinceLastDrop() > occlusionThresholdMs()) {
      currentFlowRate_ml_hr = 0.0f;
      currentGttMin = 0.0f;
    }

    sendToHost();
    screenNeedsRedraw = true;
  }

  // วาดจอเมื่อมีอะไรเปลี่ยน หรือทุก 250 ms เพื่อให้จังหวะหยดและตัวกะพริบเดินต่อ
  static unsigned long lastDraw = 0;
  if (screenNeedsRedraw || now - lastDraw >= 250) {
    lastDraw = now;
    screenNeedsRedraw = false;
    drawScreen();
  }
}
