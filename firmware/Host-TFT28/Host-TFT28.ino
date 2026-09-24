/**
 * @file      Host-TFT28.ino
 * @brief     เฟิร์มแวร์เครื่องส่วนกลาง รุ่นจอสี TFT 2.8 นิ้ว
 * @version   4.9.0-TFT
 * @date      2026-09-23
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Hardware
 * ESP32-S3 Dev Module (N16R8) + จอสี TFT 2.8" (ST7789V SPI 240x320) + Passive Buzzer
 *
 * @par Description
 * ทำหน้าที่เหมือนรุ่นจอ OLED ทุกอย่าง ต่างกันที่การแสดงผลบนจอสีแนวนอน 320x240
 * ออกแบบหน้าจอแนวทาง FOCUS ครึ่งซ้ายคือเตียงที่ต้องดูตอนนี้ ครึ่งขวาคือเตียงอื่น
 * เลือกเตียงที่เน้นอัตโนมัติ เหตุวิกฤตมาก่อน ตามด้วยใกล้หมดถุง
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 4.9.0-TFT | 2026-09-23 | เตียงที่หายไประหว่างให้น้ำเกลือมีเสียงเตือนแล้ว เพิ่มสุนัขเฝ้าบ้าน รายงานสาเหตุการรีบูต และปฏิเสธแพ็กเก็ตที่ค่าเป็นไปไม่ได้ |
 * | 4.8.1-TFT | 2026-09-23 | ย้ายโค้ดวาดจอออกไปไว้ที่ `HostScreen.h` ตรรกะไม่เปลี่ยนแม้แต่บรรทัดเดียว |
 * | 4.8.0-TFT | 2026-09-12 | เปลี่ยนจาก OLED 1.3" ขาวดำ เป็นจอสี TFT 2.8" พร้อมหน้าจอแนวทาง FOCUS |
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
 *  เปลี่ยนใน V4.8.0-TFT: เปลี่ยนจอ OLED 1.3" ขาวดำ -> จอสี TFT 2.8" (ST7789V 240x320)
 *   - ใช้จอแนวนอน 320x240 ออกแบบหน้าจอใหม่แนวทาง "FOCUS"
 *     ครึ่งซ้าย = เตียงที่ต้องดูตอนนี้ (ตัวเลขใหญ่ อ่านได้จากระยะไกล)
 *     ครึ่งขวา  = รายการเตียงอื่นพร้อมอัตราไหลและแถบความคืบหน้า (ปรับขนาดแถวตามจำนวนเตียง)
 *     แถบล่าง   = สรุปเหตุการณ์ที่ต้องทำตอนนี้
 *   - เตียงที่ถูกเน้นเลือกอัตโนมัติ: เหตุวิกฤตมาก่อน ตามด้วยใกล้หมดถุง
 *     กดปุ่ม PAGE เพื่อเลื่อนดูเตียงอื่นเองได้ (กลับเป็นอัตโนมัติเมื่อมีเหตุใหม่)
 *   - มีจอเตือนเต็มจอสีแดงเมื่อเกิดเหตุวิกฤตและยังไม่พักเสียง
 *   - ตัดไลบรารี U8g2 ออก ใช้ Adafruit GFX + ST7789 แทน (เฟิร์มแวร์นี้รองรับจอ TFT อย่างเดียว)
 *   - ตรรกะการวัด การแจ้งเตือน ESP-NOW และหน้าเว็บทั้งหมดเหมือน V4.7.0 ทุกประการ
 *
 *  เปลี่ยนใน V4.7.0: ตัดระบบจัดการ Wi-Fi (Router/อินเทอร์เน็ต) ออกทั้งหมด
 *   - Host ทำงานเป็น Access Point อย่างเดียว (WIFI_AP) ไม่ต่อ Router ไม่ใช้ NTP
 *     -> ไม่มีการสแกนช่อง/รีคอนเนกต์มาแย่งเวลาคลื่นวิทยุ ESP-NOW และ Web Server อีก
 *   - รองรับสมาร์ตโฟน/แท็บเล็ตพร้อมกันได้ถึง AP_MAX_CLIENTS เครื่อง (เดิมค่าปริยาย 4
 *     และในโหมด AP+STA เหลือใช้งานจริงเพียง 2-3 เครื่อง)
 *   - ปิดโหมดประหยัดพลังงานของ Wi-Fi (WIFI_PS_NONE) ให้ตอบสนองหลายเครื่องพร้อมกันได้นิ่ง
 *   - ESP-NOW ส่งผ่านอินเทอร์เฟซ AP (WIFI_IF_AP) เพราะไม่มี STA แล้ว
 *   - ตั้งเวลาจากนาฬิกาของเครื่องที่เปิดหน้าเว็บโดยอัตโนมัติ (แทน NTP) และสำรองเวลาไว้ใน NVS
 *     ทุก 10 นาที เพื่อให้เวลาไม่หายเมื่อไฟดับ (แสดงเป็น "เวลาโดยประมาณ" จนกว่าจะซิงก์ใหม่)
 *   - หน้าเว็บ: เอาเมนูจัดการ Wi-Fi ออก เหลือหน้าต่างแสดงข้อมูลจุดเชื่อมต่อแบบอ่านอย่างเดียว
 *     และหยุด Poll ข้อมูลเมื่อสลับแท็บไปทำอย่างอื่น (ลดภาระเมื่อมีผู้ใช้หลายเครื่อง)
 *
 *  เพิ่มใน V4.6.0: แจ้งเตือนใกล้หมด (เตรียมถุงใหม่)
 *   - ตั้ง % ต่อเตียงจากหน้าเว็บ (50-95%, ค่าเริ่มต้น 80%) เก็บถาวรใน NVS
 *   - Host เสียงเตือนเบา 3 ครั้ง ซ้ำทุก 5 นาทีจนกว่าจะรับทราบ (ที่เตียง / ปุ่ม POWER / หน้าเว็บ)
 *   - รับทราบแล้วซิงก์ไปทุกจุด และล้างเมื่อกด "เริ่มถุงใหม่" หรือแก้ปริมาตร/เปอร์เซ็นต์
 *
 *  สรุปการแก้ไขจาก V4.4.1 (โค้ดภายในระบุ 6.3.2)
 *   1) แก้จุดที่คอมไพล์ไม่ผ่าน (ฟิลด์ใน StationData / ชื่อฟังก์ชัน-ตัวแปรใน loop)
 *   2) โครงสร้างแพ็กเก็ต ESP-NOW ตรงกับ Station ทุกไบต์ (มี static_assert ตรวจขนาด)
 *   3) Station ส่งอัตราไหลที่คำนวณจากช่วงห่างระหว่างหยด + เวลาตั้งแต่หยดล่าสุด
 *      -> เลิกคำนวณ rate จากหน้าต่าง 2 วินาทีที่ทำให้แจ้งเตือนสายพับผิด
 *   4) Host เป็นผู้ตัดสินรหัสเตือนจุดเดียว เรียก evaluateClinicalAlerts() ทุก 1 วินาที
 *   5) ส่ง Sync ทุก 1 วินาที (เดิม 10 วินาที ขณะที่ Station ตัดสินว่าหลุดที่ 5 วินาที)
 *   6) เพิ่ม API /api/stations/config และ /api/stations/reset
 *      ค่า Target Rate / Plan Volume / Drop Factor / ชื่อผู้ป่วย เก็บที่ Host (NVS)
 *   7) Drop factor ใช้ค่าของแต่ละเตียง (10/15/20/60) แทนค่าคงที่ 20
 *   8) นับหยดรายนาทีจากผลต่าง totalDrops -> แพ็กเก็ตหายไม่ทำให้ข้อมูลหาย
 *   9) SoftAP ล็อก Channel 1
 *  10) Deep sleep ใช้ ext0 wakeup (esp_deep_sleep_enable_gpio_wakeup ไม่รองรับ ESP32-S3)
 *  11) ปุ่ม POWER กดสั้นขณะมีเสียงเตือน = พักเสียง 2 นาที (Snooze)
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <time.h>
#include <sys/time.h>
#include <driver/rtc_io.h>
#include <esp_task_wdt.h>          // [4.9.0-TFT] เพิ่ม: สุนัขเฝ้าบ้าน กันเครื่องค้างเงียบ
#include <esp_system.h>            // [4.9.0-TFT] เพิ่ม: อ่านสาเหตุการรีบูตครั้งล่าสุด

// ----------------------------------------------------------------------------
// สุนัขเฝ้าบ้านและสาเหตุการรีบูต ([4.9.0-TFT] เพิ่มทั้งหมด)
//
// เครื่องนี้ทำหน้าที่เตือนภัย การค้างแบบเงียบจึงอันตรายกว่าการรีบูต เพราะจอยัง
// ค้างภาพเดิมไว้ พยาบาลจึงเข้าใจว่าระบบยังเฝ้าอยู่ ทั้งที่หยุดไปแล้ว
// ตั้งไว้ 8 วินาที ซึ่งยาวกว่ารอบ loop() ปกติหลายเท่า จึงไม่รีบูตเพราะงานหนักชั่วคราว
// ----------------------------------------------------------------------------
#define LOOP_WDT_TIMEOUT_S      8

esp_reset_reason_t bootResetReason = ESP_RST_UNKNOWN;


#define APP_VERSION         "4.9.0-TFT"
#define DEV_NAME            "กิตติพันธ์ รัตนคร"
#define DEV_ROLE            "นักวิชาการคอมพิวเตอร์"
#define DEV_INSTITUTION     "มหาวิทยาลัยมหาจุฬาลงกรณราชวิทยาลัย วิทยาเขตแพร่"

// ----------------------------------------------------------------------------
// กำหนดขาเชื่อมต่อฮาร์ดแวร์ฝั่ง Host
// ----------------------------------------------------------------------------
#define HOST_BAT_ADC_PIN    1   // ขาอ่านแบตเตอรี่ Host
#define POWER_BTN_PIN       4   // ปุ่มเปิด-ปิดหน้าจอ / พักเสียงเตือน (Snooze) — ต้องเป็น RTC GPIO (0-21)
#define PAGE_BTN_PIN        5   // ปุ่มเปลี่ยนหน้าจอ / ดับเบิ้ลคลิกเรียก Screensaver
#define BUZZER_PIN          7   // ขาต่อลำโพง Passive Buzzer

// ---- จอ TFT 2.8" ST7789V (SPI 14 ขา ไม่มีทัช) ----
// ต่อสาย: VCC->3V3, GND->GND, SCL/SCK->TFT_SCLK, SDA/MOSI->TFT_MOSI,
//         RES->TFT_RST, DC->TFT_DC, CS->TFT_CS, BLK->TFT_BLK (หรือต่อ 3V3 ตลอดแล้วตั้ง -1)
// หมายเหตุ: บอร์ด N16R8 ใช้ GPIO33-37 กับ PSRAM ห้ามนำมาใช้
#define TFT_CS              10
#define TFT_DC              9
#define TFT_RST             8
#define TFT_MOSI            11
#define TFT_SCLK            12
#define TFT_BLK             13  // -1 = ไม่ได้ต่อขาควบคุมไฟหน้าจอ
#define TFT_ROTATION        1   // 1 = แนวนอน 320x240 (ถ้าภาพกลับหัวให้ใช้ 3)

// ----------------------------------------------------------------------------
// ค่าคงที่ของระบบสื่อสารและการแจ้งเตือน (ต้องตรงกับ Station)
// ----------------------------------------------------------------------------
#define ESPNOW_CHANNEL          1        // ช่อง SoftAP / ESP-NOW เริ่มต้น
#define SYNC_INTERVAL_MS        1000     // ส่ง Sync ให้ Station ทุก 1 วินาที
#define ONLINE_TIMEOUT_MS       5000     // ไม่ได้รับข้อมูลเกิน 5 วินาที = OFFLINE
#define LOST_LINK_GRACE_MS      30000    // [4.9.0-TFT] เพิ่ม: เงียบต่อจาก OFFLINE อีกเท่านี้จึงถือว่าขาดการติดต่อจริง
#define MAX_PLAUSIBLE_RATE_HR   3000.0f  // [4.9.0-TFT] เพิ่ม: ชุดให้สารน้ำมาตรฐานไหลได้ไม่ถึงเท่านี้
#define MAX_PLAUSIBLE_BATT_V    6.0f     // [4.9.0-TFT] เพิ่ม: แบตลิเธียมเซลล์เดียว ไม่มีทางถึง 6 โวลต์
#define MIN_OCCLUSION_MS        8000     // เวลาต่ำสุดที่ไม่มีหยดก่อนถือว่าหยุดไหล
#define MAX_OCCLUSION_MS        300000   // เพดานเวลาตัดสินหยุดไหล (อัตราต่ำมาก)
#define RATE_DEVIATION_HOLD_MS  20000    // เร็ว/ช้าเกินต่อเนื่อง 20 วินาทีจึงเตือน
#define SNOOZE_MS               120000   // พักเสียง 2 นาที
#define NEAR_END_REMIND_MS      300000   // เตือนใกล้หมดซ้ำทุก 5 นาทีจนกว่าจะรับทราบ
#define SCREENSAVER_IDLE_MS     300000   // ไม่มีการกดปุ่ม 5 นาที -> เข้าโหมดนาฬิกา
#define MANUAL_FOCUS_HOLD_MS    60000    // เลือกเตียงเองแล้วคงไว้ 1 นาที ก่อนกลับเป็นอัตโนมัติ
#define DEFAULT_NEAR_END_PCT    80       // เตือนเมื่อให้ไปแล้ว 80% ของปริมาตรตามแผน
#define AP_MAX_CLIENTS          8        // จำนวนสมาร์ตโฟน/แท็บเล็ตที่ต่อพร้อมกันได้ (สูงสุด 10)
#define TIME_BACKUP_INTERVAL_MS 600000   // สำรองเวลาปัจจุบันลง NVS ทุก 10 นาที

// รหัสแจ้งเตือน (ใช้ร่วมกันทั้ง Host / Station / Web)
#define ALERT_NONE        0
#define ALERT_TOO_FAST    1
#define ALERT_TOO_SLOW    2
#define ALERT_NEAR_END    3   // ให้ไปแล้วถึง % ที่ตั้ง (เตือนเบา ไม่ใช่เหตุวิกฤต)
#define ALERT_COMPLETE    4   // ให้ครบตามแผนแล้ว
#define ALERT_OCCLUSION   5   // สายพับ / หยุดไหล
#define ALERT_LOST_LINK   7   // [4.9.0-TFT] เพิ่ม: เตียงที่กำลังให้น้ำเกลือหายไปจากอากาศ (ใช้ฝั่ง Host เท่านั้น)

SPIClass SPI_TFT(FSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI_TFT, TFT_CS, TFT_DC, TFT_RST);

const char *default_ap_ssid = "ESP32_Liquid_Monitor";
const char *default_ap_pass = "12345678";
const char *web_username    = "admin";
const char *web_password    = "password123";

// เขตเวลาไทย (ICT UTC+7) — ตั้งค่าในตัวเครื่อง ไม่พึ่ง NTP อีกต่อไป
const char* TZ_THAILAND     = "ICT-7";

#define MAX_SUPPORTED_STATIONS 8
#define MAX_LOGS 60

WebServer server(80);
Preferences preferences;
portMUX_TYPE dataMux = portMUX_INITIALIZER_UNLOCKED;

uint8_t activeStationCount  = 5;
int  manualFocusBed         = -1;      // เตียงที่ผู้ใช้เลือกดูเอง (-1 = อัตโนมัติ)
unsigned long manualFocusUntil = 0;
bool displaySleeping        = false;
bool screensaverActive      = false;
bool isPowerOffProgressActive = false;

// ---- โหมดของหน้าจอ ----
#define UI_MODE_MAIN    0
#define UI_MODE_ALARM   1
#define UI_MODE_SAVER   2
#define UI_MODE_POWER   3

int   uiMode           = -1;
int   uiFocusDrawn     = -1;
int   uiRowsDrawn      = -1;
String cacheClock      = "";
String cacheTopRight   = "";
String cacheFocusHead  = "";
String cacheStatusWord = "";
String cacheRate       = "";
String cacheSetRate    = "";
String cacheSummary    = "";
String cacheBottom     = "";
int    cachePct        = -999;
String cacheSide[MAX_SUPPORTED_STATIONS];
String cacheAlarmRate  = "";

unsigned long lastUserActivityTime = 0;
bool isTimeSynced           = false;   // ตั้งเวลาจากเครื่องผู้ใช้ผ่านหน้าเว็บแล้ว
bool isTimeApprox           = false;   // กู้เวลาจากที่สำรองไว้ตอนบูต (ยังไม่ซิงก์ใหม่)
unsigned long lastTimeBackup = 0;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ---- สถานะของเตียงสำหรับการแสดงผล ----
// ต้องประกาศไว้ตอนต้นไฟล์ เพราะ Arduino IDE แทรก prototype ของฟังก์ชันไว้ก่อนส่วนแสดงผล
enum BedUiStatus { BU_NORMAL = 0, BU_FAST, BU_SLOW, BU_NOFLOW, BU_NEAREND, BU_DONE, BU_PAUSED, BU_OFFLINE };

// ----------------------------------------------------------------------------
// โครงสร้างข้อมูลรับ-ส่ง ESP-NOW (Protocol v2) — ต้องเหมือนกับ Station ทุกไบต์
// ----------------------------------------------------------------------------
typedef struct __attribute__((packed)) struct_message {
  uint8_t  stationId;
  uint8_t  isRunning;
  uint32_t totalDrops;
  uint32_t periodDrops;
  float    flowRateHr;       // อัตราไหลที่ Station คำนวณจากช่วงห่างระหว่างหยด (mL/h)
  uint32_t msSinceLastDrop;  // มิลลิวินาทีนับจากหยดล่าสุด (0 = ยังไม่มีหยด)
  float    batteryVolts;     // 0 = ไม่มีวงจรวัดแบตเตอรี่
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
  uint8_t  resetSeq;         // เปลี่ยนค่าเมื่อกด "เริ่มถุงใหม่" -> Station รีเซ็ตตัวนับ
  uint8_t  hostChannel;
  uint8_t  nearEndPct;       // % ของปริมาตรตามแผนที่เริ่มเตือนใกล้หมด
  uint8_t  flags;            // bit0 = รับทราบเตือนใกล้หมดแล้ว
} struct_host_sync;          // 20 bytes

static_assert(sizeof(struct_message) == 23, "struct_message must be 23 bytes (match Station)");
static_assert(sizeof(struct_host_sync) == 20, "struct_host_sync must be 20 bytes (match Station)");

struct LogEntry {
  uint32_t minuteIndex;
  char timeStr[20];
  uint16_t drops;
  float volume;
  float rateHr;
  float targetRate;
  uint8_t alertCode;
  int8_t rssi;
  float battery;
};

struct BedConfig {
  float   targetRateHr = 0.0f;
  float   planVolumeMl = 0.0f;
  uint8_t dropFactor   = 20;
  uint8_t resetSeq     = 0;
  char    patientName[64] = "";
};

struct TrialCaseRecord {
  bool active = false;
  uint8_t caseNumber = 1;
  char startTimeStr[24] = "";
  float targetRate = 0.0;
  float planVolume = 0.0;
  float actualVolume = 0.0;
  float errorMl = 0.0;
  float errorPercent = 0.0;
};

struct StationData {
  bool active = false;
  bool isRunning = true;
  int8_t rssi = -100;
  float batteryVolts = 0.0;
  uint32_t totalDrops = 0;
  uint32_t lastTotalDrops = 0;
  bool hasBaseline = false;
  uint32_t dropsInCurrentMinute = 0;
  float totalVolumeMl = 0.0;
  float flowRate_ml_hr = 0.0;
  uint32_t msSinceLastDrop = 0;
  unsigned long lastRecvTime = 0;
  uint8_t alertCode = ALERT_NONE;
  unsigned long deviationSince = 0;
  uint8_t nearEndPct = DEFAULT_NEAR_END_PCT;   // เก็บแยก key ใน NVS เพื่อไม่ให้ค่าตั้งเดิมหาย
  bool nearEndAck = false;
  bool wasMonitoring = false;   // [4.9.0-TFT] เพิ่ม: เคยเห็นเตียงนี้ออนไลน์และกำลังนับอยู่จริง
  bool lostLinkAck = false;     // [4.9.0-TFT] เพิ่ม: พยาบาลรับทราบว่าเตียงนี้ขาดการติดต่อแล้ว
  unsigned long nearNextChime = 0;

  BedConfig cfg;
  TrialCaseRecord trialCase;
  LogEntry logs[MAX_LOGS];
  uint8_t logCount = 0;
};

StationData stations[MAX_SUPPORTED_STATIONS];
unsigned long lastCalcTime           = 0;
unsigned long lastMinuteLogTime      = 0;
unsigned long lastOledUpdateTime     = 0;
unsigned long lastSyncBroadcastTime  = 0;
unsigned long globalSnoozeUntil      = 0;
uint32_t globalMinuteCounter         = 0;

float hostBatteryVolts = 4.2;
int hostBatteryPct = 100;

bool globalAlarmTriggered            = false;
unsigned long lastBuzzerAlarmTime    = 0;
uint8_t nearChimeRemaining           = 0;
unsigned long nearChimeNextBeep      = 0;

// ฟังก์ชันวาดจอที่ย้ายไป HostScreen.h แล้ว แต่ถูกเรียกจากโค้ดเหนือบรรทัด include
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg);
void drawWifiIcon(int x, int y, int clients);
void drawBatteryIcon(int x, int y, int pct);
void updateBottomBar();
void updateSideList(int focus, bool force);
void drawFocusPanel(int focus, bool force);
void drawMainFramework();
void drawAlarmFramework();
void updateAlarmScreen();
void drawSaverFramework();
void updateHostDisplay();
void setDisplaySleep(bool sleep);
int  currentFocusBed();
void drawPowerOffProgress(int pct);
void drawGoodbyeScreen();

// ----------------------------------------------------------------------------
// ฟังก์ชันช่วย
// ----------------------------------------------------------------------------
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

uint8_t safeDropFactor(uint8_t df) {
  return (df == 10 || df == 15 || df == 20 || df == 60) ? df : 20;
}

bool isCriticalAlert(uint8_t code) {
  return code == ALERT_TOO_FAST || code == ALERT_TOO_SLOW ||
         code == ALERT_COMPLETE || code == ALERT_OCCLUSION ||
         code == ALERT_LOST_LINK;
}

uint8_t safeNearPct(uint8_t p) {
  return (p >= 50 && p <= 95) ? p : DEFAULT_NEAR_END_PCT;
}

bool isSnoozed() {
  return globalSnoozeUntil > 0 && millis() < globalSnoozeUntil;
}

bool isStationOnline(int i) {
  unsigned long lr = stations[i].lastRecvTime;
  if (lr == 0) return false;
  unsigned long now = millis();
  return (now < lr) || (now - lr < ONLINE_TIMEOUT_MS);
}

// [4.9.0-TFT] เพิ่ม: เตียงที่กำลังให้น้ำเกลืออยู่ แล้วเงียบหายไปจากอากาศ
// ต้องเคยเห็นว่าออนไลน์และกำลังนับมาก่อน เตียงที่ยังไม่เคยเปิดใช้จึงไม่ร้อง
bool isStationLostLink(int i) {
  const StationData &s = stations[i];
  if (!s.wasMonitoring || s.lostLinkAck) return false;
  if (isStationOnline(i)) return false;
  if (s.lastRecvTime == 0) return false;
  return (millis() - s.lastRecvTime) > (ONLINE_TIMEOUT_MS + LOST_LINK_GRACE_MS);
}

// เกณฑ์เวลา "ไม่มีหยด" = 2.5 เท่าของช่วงหยดตามเป้าหมาย (ขั้นต่ำ 8 วินาที) — สูตรเดียวกับ Station
uint32_t occlusionThresholdMs(float rateHr, uint8_t df) {
  if (rateHr <= 0.0f) return MIN_OCCLUSION_MS;
  float dropsPerHr = rateHr * (float)df;
  uint32_t expected = (uint32_t)(3600000.0f / dropsPerHr);
  uint32_t th = (expected * 5) / 2;
  if (th < MIN_OCCLUSION_MS) th = MIN_OCCLUSION_MS;
  if (th > MAX_OCCLUSION_MS) th = MAX_OCCLUSION_MS;
  return th;
}

bool isStationOccluded(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i) || !s.isRunning || s.totalDrops == 0 || s.msSinceLastDrop == 0) return false;
  unsigned long age = millis() - s.lastRecvTime;
  uint32_t since = s.msSinceLastDrop + (uint32_t)age;
  return since > occlusionThresholdMs(s.cfg.targetRateHr, safeDropFactor(s.cfg.dropFactor));
}

// [4.9.0-TFT] เพิ่ม: รหัสที่ส่งออกไปให้ Station — รหัสเฉพาะฝั่ง Host ต้องกลายเป็น ALERT_NONE
uint8_t alertCodeForStation(uint8_t code) {
  return (code == ALERT_LOST_LINK) ? ALERT_NONE : code;
}

const char* alertTextEn(uint8_t code) {
  switch (code) {
    case ALERT_TOO_FAST:  return "TOO FAST";
    case ALERT_TOO_SLOW:  return "TOO SLOW";
    case ALERT_NEAR_END:  return "NEXT BAG";
    case ALERT_COMPLETE:  return "BAG EMPTY";
    case ALERT_OCCLUSION: return "OCCLUSION";
    case ALERT_LOST_LINK: return "BED LOST";
    default:              return "NORMAL";
  }
}

String jsonEscape(const char* s) {
  String o;
  for (const char* p = s; *p; p++) {
    char c = *p;
    if (c == '"' || c == '\\') { o += '\\'; o += c; }
    else if ((uint8_t)c < 0x20) o += ' ';
    else o += c;
  }
  return o;
}

uint8_t currentWifiChannel() {
  uint8_t primary = 0;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);
  return primary;
}

// ----------------------------------------------------------------------------
// ค่าตั้งเตียง (เก็บถาวรใน NVS)
// ----------------------------------------------------------------------------
void loadBedConfigs() {
  preferences.begin("bed-cfg", true);
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
    char key[6];
    snprintf(key, sizeof(key), "b%d", i + 1);
    if (preferences.getBytesLength(key) == sizeof(BedConfig)) {
      preferences.getBytes(key, &stations[i].cfg, sizeof(BedConfig));
      stations[i].cfg.dropFactor = safeDropFactor(stations[i].cfg.dropFactor);
      stations[i].cfg.patientName[sizeof(stations[i].cfg.patientName) - 1] = '\0';
    }
    char pkey[6];
    snprintf(pkey, sizeof(pkey), "p%d", i + 1);
    stations[i].nearEndPct = safeNearPct(preferences.getUChar(pkey, DEFAULT_NEAR_END_PCT));
    stations[i].trialCase.active     = stations[i].cfg.planVolumeMl > 0;
    stations[i].trialCase.planVolume = stations[i].cfg.planVolumeMl;
    stations[i].trialCase.targetRate = stations[i].cfg.targetRateHr;
  }
  preferences.end();
}

void saveBedConfig(int idx) {
  char key[6];
  snprintf(key, sizeof(key), "b%d", idx + 1);
  preferences.begin("bed-cfg", false);
  preferences.putBytes(key, &stations[idx].cfg, sizeof(BedConfig));
  char pkey[6];
  snprintf(pkey, sizeof(pkey), "p%d", idx + 1);
  preferences.putUChar(pkey, stations[idx].nearEndPct);
  preferences.end();
}

// ----------------------------------------------------------------------------
// ระบบเวลาและการซิงก์ข้อมูล
// ----------------------------------------------------------------------------
String getFormattedDateTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
    char buf[24];
    snprintf(buf, sizeof(buf), "Min-%04u", (unsigned)globalMinuteCounter);
    return String(buf);
  }
  char buf[24];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

String getDisplayClockStr() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) return "--:--";
  char buf[12];
  strftime(buf, sizeof(buf), "%H:%M", &timeinfo);
  return String(buf);
}

// นาฬิกาพร้อมใช้งานหรือยัง (ตั้งจากเครื่องผู้ใช้ หรือกู้จากค่าที่สำรองไว้)
bool hasValidClock() {
  time_t now;
  time(&now);
  return now > 1600000000;
}

// บันทึกเวลาปัจจุบันลง NVS เพื่อกู้คืนเมื่อไฟดับ (ไม่มี RTC สำรองในบอร์ด)
void backupClockToNvs() {
  time_t now;
  time(&now);
  if (now <= 1600000000) return;
  preferences.begin("sys-config", false);
  preferences.putUInt("lastEpoch", (uint32_t)now);
  preferences.end();
}

void broadcastSyncToNodes() {
  time_t now;
  time(&now);
  uint8_t ch = currentWifiChannel();

  for (int i = 0; i < activeStationCount; i++) {
    struct_host_sync syncMsg = {};
    syncMsg.epochTime    = (now > 1600000000) ? (uint32_t)now : 0;
    syncMsg.isSynced     = (now > 1600000000) ? 1 : 0;
    syncMsg.stationId    = i + 1;
    syncMsg.targetRateHr = stations[i].cfg.targetRateHr;
    syncMsg.totalPlanMl  = stations[i].cfg.planVolumeMl;
    syncMsg.alertCode    = alertCodeForStation(stations[i].alertCode);   // [4.9.0-TFT] แก้
    syncMsg.dropFactor   = safeDropFactor(stations[i].cfg.dropFactor);
    syncMsg.resetSeq     = stations[i].cfg.resetSeq;
    syncMsg.hostChannel  = ch;
    syncMsg.nearEndPct   = safeNearPct(stations[i].nearEndPct);
    syncMsg.flags        = stations[i].nearEndAck ? 0x01 : 0x00;

    esp_err_t r = esp_now_send(broadcastAddress, (uint8_t *)&syncMsg, sizeof(syncMsg));
    if (r != ESP_OK) {
      delay(10);
      esp_now_send(broadcastAddress, (uint8_t *)&syncMsg, sizeof(syncMsg));
    }
    delay(4);
  }
}

float readHostBattery() {
  int raw = analogRead(HOST_BAT_ADC_PIN);
  float volts = (raw / 4095.0) * 3.3 * 2.0;
  if (volts > 4.25) volts = 4.2;
  if (volts < 2.5) volts = 0.0;
  return volts;
}

int calculateBatteryPct(float volts) {
  if (volts <= 0.5) return 100;
  int pct = (int)(((volts - 3.2) / (4.2 - 3.2)) * 100.0);
  if (pct > 100) pct = 100;
  if (pct < 0) pct = 0;
  return pct;
}

void playWelcomeMelody() {
  int melody[] = { 523, 659, 784, 1046 };
  int durations[] = { 80, 80, 80, 200 };
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] + 30);
  }
  noTone(BUZZER_PIN);
}

void playShutdownMelody() {
  int melody[] = { 1046, 784, 659, 523 };
  int durations[] = { 80, 80, 80, 250 };
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] + 30);
  }
  noTone(BUZZER_PIN);
}

// ----------------------------------------------------------------------------
// ระบบควบคุม Power & Deep Sleep (ESP32-S3 ใช้ ext0 wakeup)
// ----------------------------------------------------------------------------
void enterDeepSleepWaitPowerButton() {
  stopLoopWatchdog();   // [4.9.0-TFT] เพิ่ม: เลิกเฝ้าก่อนหลับ ไม่งั้นถูกนับว่าค้าง
  rtc_gpio_pullup_en((gpio_num_t)POWER_BTN_PIN);
  rtc_gpio_pulldown_dis((gpio_num_t)POWER_BTN_PIN);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0);
  esp_deep_sleep_start();
}

void powerOffSystem() {
  isPowerOffProgressActive = true;

  drawGoodbyeScreen();

  playShutdownMelody();

  while (digitalRead(POWER_BTN_PIN) == LOW) { feedWatchdog(); delay(20); }
  delay(200);

  tft.fillScreen(0x0000);
#if TFT_BLK >= 0
  digitalWrite(TFT_BLK, LOW);
#endif

  server.stop();
  esp_now_deinit();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  enterDeepSleepWaitPowerButton();
}

// ----------------------------------------------------------------------------
// ประเมินสภาวะเตือนภัย (Host เป็นผู้ตัดสินจุดเดียว)
// ----------------------------------------------------------------------------
void evaluateClinicalAlerts() {
  bool anyCritical = false;
  unsigned long now = millis();

  for (int i = 0; i < activeStationCount; i++) {
    StationData &s = stations[i];
    // [4.9.0-TFT] แก้: แยก "ติดต่อเตียงไม่ได้" ออกจาก "พยาบาลสั่งหยุด" ของเดิมรวมสองกรณีนี้
    // ไว้ด้วยกันแล้วล้างรหัสเตือนทิ้งทั้งคู่ เตียงที่กำลังให้น้ำเกลืออยู่แล้วหายไป
    // จากอากาศจึงไม่มีเสียงใด ๆ ทั้งที่ระบบเลิกเฝ้าเตียงนั้นไปแล้ว
    if (!isStationOnline(i)) {
      uint8_t lost = isStationLostLink(i) ? ALERT_LOST_LINK : ALERT_NONE;
      s.alertCode = lost;
      s.deviationSince = 0;
      if (lost != ALERT_NONE) anyCritical = true;   // รหัสนี้ตัดสินตรงนี้ เพราะข้ามส่วนท้ายไป
      continue;
    }

    // ติดต่อได้ตามปกติ = ล้างสถานะขาดการติดต่อทิ้ง พร้อมรับเหตุการณ์ครั้งใหม่
    s.lostLinkAck = false;
    s.wasMonitoring = s.isRunning;        // เฝ้าอยู่จริงเฉพาะตอนที่กำลังนับ

    if (!s.isRunning) {
      s.alertCode = ALERT_NONE;
      s.deviationSince = 0;
      continue;
    }

    float tr   = s.cfg.targetRateHr;
    float act  = s.flowRate_ml_hr;
    float vol  = s.totalVolumeMl;
    float plan = s.cfg.planVolumeMl;
    uint8_t code = ALERT_NONE;

    if (isStationOccluded(i)) {
      code = ALERT_OCCLUSION;
      s.deviationSince = 0;
    } else if (plan > 0 && vol >= plan) {
      code = ALERT_COMPLETE;
      s.deviationSince = 0;
    } else {
      bool rateValid = (act > 0.0f) && (s.totalDrops >= 3);
      uint8_t devCode = ALERT_NONE;
      if (tr > 0 && rateValid && act > tr * 1.35f)      devCode = ALERT_TOO_FAST;
      else if (tr > 0 && rateValid && act < tr * 0.65f) devCode = ALERT_TOO_SLOW;

      if (devCode != ALERT_NONE) {
        if (s.deviationSince == 0) s.deviationSince = now;
        if (now - s.deviationSince >= RATE_DEVIATION_HOLD_MS) code = devCode;
      } else {
        s.deviationSince = 0;
      }

      if (code == ALERT_NONE && plan > 0 && vol >= plan * (float)safeNearPct(s.nearEndPct) / 100.0f) code = ALERT_NEAR_END;
    }

    s.alertCode = code;
    if (isCriticalAlert(code)) anyCritical = true;
  }

  // ถ้าไม่มีเหตุวิกฤตแล้ว ยกเลิก snooze เพื่อให้เหตุการณ์ครั้งถัดไปดังทันที
  if (!anyCritical) globalSnoozeUntil = 0;
  globalAlarmTriggered = anyCritical;
}

bool anyNearEndPending() {
  for (int i = 0; i < activeStationCount; i++) {
    if (stations[i].alertCode == ALERT_NEAR_END && !stations[i].nearEndAck) return true;
  }
  return false;
}

void acknowledgeAllNearEnd() {
  for (int i = 0; i < activeStationCount; i++) {
    if (stations[i].alertCode == ALERT_NEAR_END) stations[i].nearEndAck = true;
  }
  nearChimeRemaining = 0;
}

// เสียงเตือนใกล้หมด: 3 ครั้งสั้น ๆ ต่อเตียง ซ้ำทุก 5 นาทีจนกว่าจะรับทราบ (ไม่ดังทับเหตุวิกฤต)
void handleNearEndChime() {
  unsigned long now = millis();
  if (globalAlarmTriggered && !isSnoozed()) { nearChimeRemaining = 0; return; }

  if (nearChimeRemaining == 0) {
    for (int i = 0; i < activeStationCount; i++) {
      StationData &s = stations[i];
      if (s.alertCode != ALERT_NEAR_END || s.nearEndAck) { s.nearNextChime = 0; continue; }
      if (s.nearNextChime == 0 || now >= s.nearNextChime) {
        s.nearNextChime = now + NEAR_END_REMIND_MS;
        nearChimeRemaining = 3;
        nearChimeNextBeep = now;
        break;
      }
    }
  }
  if (nearChimeRemaining > 0 && now >= nearChimeNextBeep) {
    tone(BUZZER_PIN, 2000, 70);
    nearChimeNextBeep = now + 220;
    nearChimeRemaining--;
  }
}

void handleBuzzerAlarm() {
  static uint8_t beepPhase = 0;
  if (!globalAlarmTriggered || isSnoozed()) {
    beepPhase = 0;
    handleNearEndChime();
    return;
  }

  unsigned long now = millis();
  if (beepPhase == 0 && now - lastBuzzerAlarmTime >= 2500) {
    tone(BUZZER_PIN, 1500, 100);
    lastBuzzerAlarmTime = now;
    beepPhase = 1;
  } else if (beepPhase == 1 && now - lastBuzzerAlarmTime >= 200) {
    tone(BUZZER_PIN, 1500, 100);
    beepPhase = 0;
  }
}

void checkSmartphonePowerButton() {
  static unsigned long btnPressStart = 0;
  static unsigned long lastProgressFrameTime = 0;
  static bool isHolding = false;

  int btnState = digitalRead(POWER_BTN_PIN);

  if (btnState == LOW) {
    if (!isHolding) {
      btnPressStart = millis();
      isHolding = true;
    }

    unsigned long holdDuration = millis() - btnPressStart;

    if (holdDuration >= 450 && !displaySleeping) {
      isPowerOffProgressActive = true;

      if (millis() - lastProgressFrameTime >= 60) {
        lastProgressFrameTime = millis();
        int progressPct = map(holdDuration, 450, 2000, 0, 100);
        drawPowerOffProgress(progressPct);
      }
    }

    if (holdDuration >= 2000) powerOffSystem();
  }
  else {
    if (isHolding) {
      unsigned long totalHoldTime = millis() - btnPressStart;
      lastUserActivityTime = millis();

      if (totalHoldTime < 400 && globalAlarmTriggered && !isSnoozed()) {
        // กดสั้นขณะมีเสียงเตือน = พักเสียง 2 นาที
        globalSnoozeUntil = millis() + SNOOZE_MS;
        noTone(BUZZER_PIN);
        tone(BUZZER_PIN, 2200, 40);
        if (displaySleeping) setDisplaySleep(false);
        screensaverActive = false;
        isPowerOffProgressActive = false;
        uiMode = -1;
        updateHostDisplay();
      } else if (totalHoldTime < 400 && anyNearEndPending()) {
        // กดสั้นขณะมีเตือนใกล้หมดที่ยังไม่รับทราบ = รับทราบทุกเตียง
        acknowledgeAllNearEnd();
        tone(BUZZER_PIN, 2600, 40);
        if (displaySleeping) setDisplaySleep(false);
        screensaverActive = false;
        isPowerOffProgressActive = false;
        uiMode = -1;
        updateHostDisplay();
      } else if (screensaverActive) {
        screensaverActive = false;
        isPowerOffProgressActive = false;
        uiMode = -1;
        updateHostDisplay();
      } else if (totalHoldTime < 400) {
        setDisplaySleep(!displaySleeping);
        tone(BUZZER_PIN, 1800, 25);
        if (!displaySleeping) {
          isPowerOffProgressActive = false;
          updateHostDisplay();
        }
      }
      else if (isPowerOffProgressActive && !displaySleeping) {
        isPowerOffProgressActive = false;
        uiMode = -1;
        updateHostDisplay();
      }
      isPowerOffProgressActive = false;
      isHolding = false;
      btnPressStart = 0;
    }
  }
}

void checkPageButton() {
  static bool isPageHolding = false;
  static int clickCount = 0;
  static unsigned long lastReleaseTime = 0;

  int reading = digitalRead(PAGE_BTN_PIN);
  unsigned long now = millis();

  if (reading == LOW) {
    if (!isPageHolding) { isPageHolding = true; }
  } else {
    if (isPageHolding) {
      isPageHolding = false;
      clickCount++;
      lastReleaseTime = now;
    }
  }

  if (clickCount > 0 && (now - lastReleaseTime > 280)) {
    lastUserActivityTime = now;
    if (displaySleeping) setDisplaySleep(false);
    if (screensaverActive) {
      screensaverActive = false;              // ปลุกจอก่อน
    } else if (clickCount >= 2) {
      screensaverActive = true;               // ดับเบิลคลิก = โหมดนาฬิกา
    } else {
      // คลิกเดียว = เลื่อนไปดูเตียงถัดไปเอง (กลับเป็นอัตโนมัติเมื่อครบเวลาหรือมีเหตุใหม่)
      int base = (manualFocusBed >= 0) ? manualFocusBed : currentFocusBed();
      manualFocusBed = (base + 1) % activeStationCount;
      manualFocusUntil = now + MANUAL_FOCUS_HOLD_MS;
      tone(BUZZER_PIN, 2400, 15);
    }
    uiMode = -1;
    updateHostDisplay();
    clickCount = 0;
  }
}

// ----------------------------------------------------------------------------
// ESP-NOW Receive Callback
// ----------------------------------------------------------------------------
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  int8_t pktRssi = (info && info->rx_ctrl) ? info->rx_ctrl->rssi : -60;
#else
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len) {
  int8_t pktRssi = -60;
#endif
  if (len != sizeof(struct_message)) return;

  struct_message incoming;
  memcpy(&incoming, incomingDataBytes, sizeof(incoming));
  if (incoming.stationId < 1 || incoming.stationId > MAX_SUPPORTED_STATIONS) return;

  // [4.9.0-TFT] เพิ่ม: ปฏิเสธค่าที่เป็นไปไม่ได้ทางกายภาพ
  // แพ็กเก็ต ESP-NOW เป็นการกระจายเปล่า ไม่มีลายเซ็นและไม่มีเลขตรวจสอบ การตรวจเดิม
  // มีแค่ความยาวกับช่วงของเลขเตียง อุปกรณ์อื่นที่บังเอิญส่งขนาด 23 ไบต์ในช่องเดียวกัน
  // จึงถูกตีความเป็นข้อมูลเตียงได้ ทำให้ตัวเลขบนจอกระโดดและอาจปลุกเสียงเตือนผิด
  // เขียนกลับด้านเพื่อให้ดัก NaN ไปด้วยในตัว (การเทียบใด ๆ กับ NaN เป็นเท็จเสมอ)
  if (incoming.isRunning > 1) return;
  if (!(incoming.flowRateHr   >= 0.0f && incoming.flowRateHr   <= MAX_PLAUSIBLE_RATE_HR)) return;
  if (!(incoming.batteryVolts >= 0.0f && incoming.batteryVolts <= MAX_PLAUSIBLE_BATT_V)) return;

  int idx = incoming.stationId - 1;
  portENTER_CRITICAL(&dataMux);
  StationData &s = stations[idx];

  // นับหยดรายนาทีจากผลต่าง totalDrops (แพ็กเก็ตหายก็ไม่ทำให้หยดหาย)
  if (s.hasBaseline) {
    uint32_t delta = (incoming.totalDrops >= s.lastTotalDrops)
                     ? (incoming.totalDrops - s.lastTotalDrops)
                     : incoming.totalDrops;   // Station รีเซ็ต/รีบูต
    s.dropsInCurrentMinute += delta;
  } else {
    s.hasBaseline = true;
  }
  s.lastTotalDrops  = incoming.totalDrops;

  s.active          = true;
  s.isRunning       = (incoming.isRunning != 0);
  s.rssi            = pktRssi;
  s.batteryVolts    = incoming.batteryVolts;
  s.totalDrops      = incoming.totalDrops;
  s.flowRate_ml_hr  = s.isRunning ? incoming.flowRateHr : 0.0f;
  s.msSinceLastDrop = incoming.msSinceLastDrop;
  s.totalVolumeMl   = (float)s.totalDrops / (float)safeDropFactor(s.cfg.dropFactor);
  s.lastRecvTime    = millis();
  if ((incoming.flags & 0x01) && s.cfg.planVolumeMl > 0) s.nearEndAck = true;

  if (s.trialCase.active) {
    s.trialCase.actualVolume = s.totalVolumeMl;
    s.trialCase.errorMl = s.trialCase.actualVolume - s.trialCase.planVolume;
    if (s.trialCase.planVolume > 0) {
      s.trialCase.errorPercent = (s.trialCase.errorMl / s.trialCase.planVolume) * 100.0;
    }
  }
  portEXIT_CRITICAL(&dataMux);
}

// ----------------------------------------------------------------------------
// Host OLED Display & Block-Style UI Renderer
// ----------------------------------------------------------------------------
// ============================================================================
// ส่วนแสดงผลบนจอ TFT 2.8" (ST7789V 240x320 ใช้แนวนอน 320x240)
//   หน้าหลัก FOCUS : เตียงที่ต้องดูตอนนี้ตัวใหญ่ครึ่งจอซ้าย + รายการเตียงอื่นด้านขวา
//   จอเตือนเต็มจอ  : เมื่อมีเหตุวิกฤตและยังไม่พักเสียง
//   Screensaver    : นาฬิกาใหญ่เมื่อไม่มีการใช้งาน
// วาดเฉพาะส่วนที่ค่าเปลี่ยน เพื่อไม่ให้จอกะพริบและไม่กิน CPU ตอนรับ ESP-NOW
// ============================================================================

// ---- สีที่ใช้บนจอ (RGB565) ----
#define C_BG        0x0000
#define C_CARD      0x18C5   // การ์ดพื้นเทาเข้ม
#define C_BAR       0x2104   // รางแถบความคืบหน้า
#define C_OFFCARD   0x1082   // การ์ดเตียงออฟไลน์/ยังไม่ใช้งาน
#define C_TOPBAR    0x0A49   // แถบบนสุด
#define C_LINE      0x39E7
#define C_DIM       0x8410
#define C_WHITE     0xFFFF
#define C_GREEN     0x2FEB
#define C_CYAN      0x07FF
#define C_YELLOW    0xFFE0
#define C_ORANGE    0xFD20
#define C_RED       0xF800
#define C_SKY       0x5DFF

BedUiStatus bedUiStatus(int i) {
  if (!isStationOnline(i)) return BU_OFFLINE;
  const StationData &s = stations[i];
  if (!s.isRunning) return BU_PAUSED;
  switch (s.alertCode) {
    case ALERT_OCCLUSION: return BU_NOFLOW;
    case ALERT_TOO_FAST:  return BU_FAST;
    case ALERT_TOO_SLOW:  return BU_SLOW;
    case ALERT_COMPLETE:  return BU_DONE;
    case ALERT_NEAR_END:  return BU_NEAREND;
    default:              return BU_NORMAL;
  }
}

uint16_t bedColor(BedUiStatus s) {
  switch (s) {
    case BU_NORMAL:  return C_GREEN;
    case BU_FAST:    return C_ORANGE;
    case BU_SLOW:    return C_YELLOW;
    case BU_NOFLOW:  return C_RED;
    case BU_NEAREND: return C_ORANGE;
    case BU_DONE:    return C_CYAN;
    case BU_PAUSED:  return C_SKY;
    default:         return C_LINE;
  }
}

const char* bedWord(BedUiStatus s) {
  switch (s) {
    case BU_NORMAL:  return "NORMAL";
    case BU_FAST:    return "TOO FAST";
    case BU_SLOW:    return "TOO SLOW";
    case BU_NOFLOW:  return "NO FLOW";
    case BU_NEAREND: return "NEXT BAG";
    case BU_DONE:    return "COMPLETE";
    case BU_PAUSED:  return "PAUSED";
    default:         return "OFFLINE";
  }
}

bool bedIsAlarm(BedUiStatus s) {
  return s == BU_FAST || s == BU_SLOW || s == BU_NOFLOW || s == BU_DONE;
}


void textRight(const String &s, int xRight, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, xRight - (int)s.length() * 6 * size, y, size, col, bg);
}

void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, x + (w - (int)s.length() * 6 * size) / 2, y, size, col, bg);
}

String padTo(const String &s, unsigned int width) {
  String out = s;
  while (out.length() < width) out += ' ';
  return out;
}




// ---- ข้อมูลของเตียงที่ใช้บ่อย ----
int bedPct(int i) {
  const StationData &s = stations[i];
  if (s.cfg.planVolumeMl <= 0) return -1;
  int p = (int)(s.totalVolumeMl * 100.0f / s.cfg.planVolumeMl + 0.5f);
  return (p > 100) ? 100 : p;
}

int bedMinutesLeft(int i) {
  const StationData &s = stations[i];
  if (s.cfg.planVolumeMl <= 0 || !isStationOnline(i) || !s.isRunning) return -1;
  if (s.alertCode == ALERT_OCCLUSION) return -1;   // สายพับ/ไม่มีการไหล คำนวณเวลาที่เหลือไม่ได้
  float left = s.cfg.planVolumeMl - s.totalVolumeMl;
  if (left < 0) left = 0;
  float rate = (s.flowRate_ml_hr > 5.0f) ? s.flowRate_ml_hr : s.cfg.targetRateHr;
  if (rate <= 0) return -1;
  return (int)((left / rate) * 60.0f);
}

// เตียงที่ควรถูกเน้น: เหตุวิกฤตมาก่อน ตามด้วยใกล้หมดถุง ไม่มีเลยจึงใช้เตียงที่ผู้ใช้เลือกไว้
int autoFocusBed() {
  for (int i = 0; i < activeStationCount; i++) if (bedIsAlarm(bedUiStatus(i))) return i;
  for (int i = 0; i < activeStationCount; i++)
    if (stations[i].alertCode == ALERT_NEAR_END && !stations[i].nearEndAck) return i;
  return -1;
}

int currentFocusBed() {
  int a = autoFocusBed();
  if (a >= 0) return a;
  if (manualFocusBed >= 0 && manualFocusBed < activeStationCount) return manualFocusBed;
  return 0;
}

// ---------------------------------------------------------------- ผังหน้าจอ
#define SCR_W       320
#define SCR_H       240
#define TOPBAR_H    24
#define PANEL_X     5
#define PANEL_Y     30
#define PANEL_W     190
#define PANEL_H     188
#define SIDE_X      200
#define SIDE_W      115
#define BOTBAR_Y    222
#define BOTBAR_H    18

void resetUiCaches() {
  cacheClock = ""; cacheTopRight = ""; cacheFocusHead = ""; cacheStatusWord = "";
  cacheRate = ""; cacheSetRate = ""; cacheSummary = ""; cacheBottom = ""; cacheAlarmRate = "";
  cachePct = -999;
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) cacheSide[i] = "";
  uiFocusDrawn = -1;
  uiRowsDrawn = -1;
}


void updateTopBar() {
  String clockStr = getDisplayClockStr();
  if (clockStr != cacheClock) {
    cacheClock = clockStr;
    textCenterIn(clockStr, 0, SCR_W, 6, 2, isTimeApprox ? C_YELLOW : C_WHITE, C_TOPBAR);
  }
  String right = String(WiFi.softAPgetStationNum()) + ":" + String(hostBatteryPct / 5);
  if (right != cacheTopRight) {
    cacheTopRight = right;
    drawWifiIcon(SCR_W - 52, 6, (int)WiFi.softAPgetStationNum());
    drawBatteryIcon(SCR_W - 30, 7, hostBatteryPct);
  }
}






void updateMainScreen() {
  int focus = currentFocusBed();
  bool focusChanged = (focus != uiFocusDrawn);
  uiFocusDrawn = focus;
  updateTopBar();
  drawFocusPanel(focus, focusChanged);
  updateSideList(focus, focusChanged);
  updateBottomBar();
}




void updateSaverScreen() {
  String clockStr = getDisplayClockStr();
  if (clockStr != cacheClock) {
    cacheClock = clockStr;
    textCenterIn(clockStr, 0, SCR_W, 76, 7, C_WHITE, C_BG);
  }
  int online = 0;
  for (int i = 0; i < activeStationCount; i++) if (isStationOnline(i)) online++;
  char buf[40];
  snprintf(buf, sizeof(buf), "ONLINE %d/%d   ALL NORMAL", online, activeStationCount);
  String line = String(buf);
  if (line != cacheBottom) {
    cacheBottom = line;
    textCenterIn(line, 0, SCR_W, 156, 1, C_GREEN, C_BG);
  }
}

// ---- ตัวควบคุมการแสดงผลทั้งหมด ----
void updateHostDisplay() {
  if (displaySleeping || isPowerOffProgressActive) return;

  bool nearPending = anyNearEndPending();
  if (!screensaverActive && !globalAlarmTriggered && !nearPending &&
      (millis() - lastUserActivityTime >= SCREENSAVER_IDLE_MS)) {
    screensaverActive = true;
  }
  // มีเหตุวิกฤต (ยังไม่พักเสียง) หรือเตือนใกล้หมดที่ยังไม่รับทราบ -> ออกจาก Screensaver
  if (screensaverActive && ((globalAlarmTriggered && !isSnoozed()) || nearPending)) {
    screensaverActive = false;
  }

  int mode = UI_MODE_MAIN;
  if (screensaverActive) mode = UI_MODE_SAVER;
  else if (globalAlarmTriggered && !isSnoozed()) mode = UI_MODE_ALARM;

  if (mode != uiMode) {
    uiMode = mode;
    resetUiCaches();
    if (mode == UI_MODE_SAVER)      drawSaverFramework();
    else if (mode == UI_MODE_ALARM) drawAlarmFramework();
    else                            drawMainFramework();
  }

  if (mode == UI_MODE_SAVER)      updateSaverScreen();
  else if (mode == UI_MODE_ALARM) updateAlarmScreen();
  else                            updateMainScreen();
}





// ----------------------------------------------------------------------------
// REST APIs & Handlers
// ----------------------------------------------------------------------------
void handleApiData() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  String json;
  json.reserve(3000);
  json = "{";
  json += "\"version\":\"" APP_VERSION "\",";
  // [4.9.0-TFT] เพิ่ม: เครื่องรีบูตเองแล้วไม่มีใครรู้สาเหตุ = หาต้นตอในวอร์ดจริงไม่ได้
  json += "\"resetReason\":\"" + String(resetReasonText(bootResetReason)) + "\",";
  json += "\"activeCount\":" + String(activeStationCount) + ",";
  json += "\"currentTime\":\"" + getFormattedDateTime() + "\",";
  json += "\"timeSynced\":" + String(isTimeSynced ? "true" : "false") + ",";
  json += "\"timeApprox\":" + String(isTimeApprox ? "true" : "false") + ",";
  json += "\"apClients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"apMaxClients\":" + String(AP_MAX_CLIENTS) + ",";
  json += "\"channel\":" + String(currentWifiChannel()) + ",";
  json += "\"hostBatVolts\":" + String(hostBatteryVolts, 2) + ",";
  json += "\"hostBatPct\":" + String(hostBatteryPct) + ",";
  json += "\"snoozed\":" + String(isSnoozed() ? "true" : "false") + ",";
  json += "\"stations\":[";
  for (int i = 0; i < activeStationCount; i++) {
    const StationData &s = stations[i];
    bool isOnline = isStationOnline(i);
    if (i > 0) json += ",";
    json += "{";
    json += "\"id\":" + String(i + 1) + ",";
    json += "\"online\":" + String(isOnline ? "true" : "false") + ",";
    json += "\"running\":" + String(s.isRunning ? "true" : "false") + ",";
    json += "\"rssi\":" + String(isOnline ? s.rssi : 0) + ",";
    json += "\"battery\":" + String(s.batteryVolts, 2) + ",";
    json += "\"totalDrops\":" + String(s.totalDrops) + ",";
    json += "\"volumeMl\":" + String(s.totalVolumeMl, 2) + ",";
    json += "\"flowRateHr\":" + String(s.flowRate_ml_hr, 2) + ",";
    json += "\"msSinceLastDrop\":" + String(s.msSinceLastDrop) + ",";
    json += "\"targetRate\":" + String(s.cfg.targetRateHr, 1) + ",";
    json += "\"planVolume\":" + String(s.cfg.planVolumeMl, 0) + ",";
    json += "\"dropFactor\":" + String(safeDropFactor(s.cfg.dropFactor)) + ",";
    json += "\"patientName\":\"" + jsonEscape(s.cfg.patientName) + "\",";
    json += "\"alertCode\":" + String(s.alertCode) + ",";
    json += "\"nearEndPct\":" + String(safeNearPct(s.nearEndPct)) + ",";
    json += "\"nearEndAck\":" + String(s.nearEndAck ? "true" : "false") + ",";
    json += "\"caseActive\":" + String(s.trialCase.active ? "true" : "false") + ",";
    json += "\"caseStart\":\"" + String(s.trialCase.startTimeStr) + "\"";
    json += "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

int parseStationArg() {
  if (!server.hasArg("station")) return -1;
  int st = server.arg("station").toInt();
  if (st < 1 || st > MAX_SUPPORTED_STATIONS) return -1;
  return st - 1;
}

// POST /api/stations/config  station, rate, volume, df, name (ส่งเฉพาะที่ต้องการเปลี่ยน)
void handleStationConfig() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int idx = parseStationArg();
  if (idx < 0) { server.send(400, "text/plain", "Invalid station"); return; }

  BedConfig &c = stations[idx].cfg;
  if (server.hasArg("rate")) {
    float r = server.arg("rate").toFloat();
    if (r < 0 || r > 1000) { server.send(400, "text/plain", "Invalid rate"); return; }
    c.targetRateHr = r;
  }
  if (server.hasArg("volume")) {
    float v = server.arg("volume").toFloat();
    if (v < 0 || v > 10000) { server.send(400, "text/plain", "Invalid volume"); return; }
    c.planVolumeMl = v;
  }
  if (server.hasArg("df")) {
    int df = server.arg("df").toInt();
    if (df != 10 && df != 15 && df != 20 && df != 60) { server.send(400, "text/plain", "Invalid drop factor"); return; }
    c.dropFactor = (uint8_t)df;
  }
  bool nearChanged = false;
  float oldPlan = c.planVolumeMl;
  if (server.hasArg("nearpct")) {
    int np = server.arg("nearpct").toInt();
    if (np < 50 || np > 95) { server.send(400, "text/plain", "Invalid near-end percent (50-95)"); return; }
    if (np != stations[idx].nearEndPct) nearChanged = true;
    stations[idx].nearEndPct = (uint8_t)np;
  }
  if (server.hasArg("name")) {
    String n = server.arg("name");
    strncpy(c.patientName, n.c_str(), sizeof(c.patientName) - 1);
    c.patientName[sizeof(c.patientName) - 1] = '\0';
  }
  saveBedConfig(idx);

  portENTER_CRITICAL(&dataMux);
  StationData &s = stations[idx];
  s.totalVolumeMl = (float)s.totalDrops / (float)safeDropFactor(c.dropFactor);
  s.deviationSince = 0;
  if (nearChanged || c.planVolumeMl != oldPlan) { s.nearEndAck = false; s.nearNextChime = 0; }
  s.trialCase.active = c.planVolumeMl > 0;
  s.trialCase.planVolume = c.planVolumeMl;
  s.trialCase.targetRate = c.targetRateHr;
  portEXIT_CRITICAL(&dataMux);

  evaluateClinicalAlerts();
  broadcastSyncToNodes();
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/stations/reset  station — เริ่มถุงใหม่: รีเซ็ตตัวนับที่ Host และสั่ง Station รีเซ็ต
void handleStationReset() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int idx = parseStationArg();
  if (idx < 0) { server.send(400, "text/plain", "Invalid station"); return; }

  String nowStr = getFormattedDateTime();
  portENTER_CRITICAL(&dataMux);
  StationData &s = stations[idx];
  s.cfg.resetSeq++;
  s.hasBaseline = false;          // แพ็กเก็ตถัดไปใช้เป็นฐานใหม่ ไม่นับเป็นหยดของนาทีนี้
  s.totalDrops = 0;
  s.totalVolumeMl = 0;
  s.flowRate_ml_hr = 0;
  s.msSinceLastDrop = 0;
  s.dropsInCurrentMinute = 0;
  s.alertCode = ALERT_NONE;
  s.deviationSince = 0;
  s.nearEndAck = false;
  s.nearNextChime = 0;
  s.logCount = 0;
  s.trialCase.caseNumber++;
  s.trialCase.actualVolume = 0;
  s.trialCase.errorMl = 0;
  s.trialCase.errorPercent = 0;
  portEXIT_CRITICAL(&dataMux);
  snprintf(s.trialCase.startTimeStr, sizeof(s.trialCase.startTimeStr), "%s", nowStr.c_str());

  saveBedConfig(idx);
  broadcastSyncToNodes();
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/stations/ack  station — รับทราบเตือนใกล้หมดจากหน้าเว็บ
void handleStationAck() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int idx = parseStationArg();
  if (idx < 0) { server.send(400, "text/plain", "Invalid station"); return; }
  stations[idx].nearEndAck = true;
  stations[idx].lostLinkAck = true;   // [4.9.0-TFT] เพิ่ม: ปุ่มรับทราบเดิมใช้ปิดเสียงเตียงที่ขาดการติดต่อได้ด้วย
  broadcastSyncToNodes();
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleSetStationCount() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  if (server.hasArg("count")) {
    int count = server.arg("count").toInt();
    if (count >= 1 && count <= MAX_SUPPORTED_STATIONS) {
      activeStationCount = count;
      preferences.begin("sys-config", false);
      preferences.putUChar("bedCount", activeStationCount);
      preferences.end();

      if (manualFocusBed >= activeStationCount) manualFocusBed = -1;
      uiMode = -1;                 // จำนวนเตียงเปลี่ยน -> วาดหน้าจอใหม่ทั้งหมด
      updateHostDisplay();
      server.send(200, "text/plain", "OK");
      return;
    }
  }
  server.send(400, "text/plain", "Invalid count");
}

void handleApiLogs() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int st = 1;
  if (server.hasArg("station")) st = server.arg("station").toInt();
  if (st < 1 || st > activeStationCount) st = 1;
  int idx = st - 1;

  String json;
  json.reserve(8000);
  json = "{";
  json += "\"stationId\":" + String(st) + ",";
  json += "\"dropFactor\":" + String(safeDropFactor(stations[idx].cfg.dropFactor)) + ",";
  json += "\"totalLogs\":" + String(stations[idx].logCount) + ",";
  json += "\"logs\":[";
  for (int i = 0; i < stations[idx].logCount; i++) {
    const LogEntry &e = stations[idx].logs[i];
    if (i > 0) json += ",";
    json += "{";
    json += "\"min\":" + String(e.minuteIndex) + ",";
    json += "\"time\":\"" + String(e.timeStr) + "\",";
    json += "\"drops\":" + String(e.drops) + ",";
    json += "\"vol\":" + String(e.volume, 2) + ",";
    json += "\"rate\":" + String(e.rateHr, 2) + ",";
    json += "\"target\":" + String(e.targetRate, 1) + ",";
    json += "\"alert\":" + String(e.alertCode) + ",";
    json += "\"rssi\":" + String(e.rssi) + ",";
    json += "\"battery\":" + String(e.battery, 2);
    json += "}";
  }
  json += "]}";
  server.send(200, "application/json", json);
}

void handleDownloadCSV() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int st = 1;
  if (server.hasArg("station")) st = server.arg("station").toInt();
  if (st < 1 || st > activeStationCount) st = 1;
  int idx = st - 1;

  String csv = "\xEF\xBB\xBF";
  csv += "Record_Index,Date_Time,Drops_Per_Min,Drop_Factor,Total_Volume_mL,Flow_Rate_mL_hr,Target_Rate_mL_hr,Plan_Volume_mL,Near_End_Alert_Pct,Alert_Code,Alert_Text,RSSI_dBm,Battery_Volts\r\n";
  uint8_t df = safeDropFactor(stations[idx].cfg.dropFactor);
  for (int i = 0; i < stations[idx].logCount; i++) {
    const LogEntry &e = stations[idx].logs[i];
    csv += String(e.minuteIndex) + ",";
    csv += String(e.timeStr) + ",";
    csv += String(e.drops) + ",";
    csv += String(df) + ",";
    csv += String(e.volume, 2) + ",";
    csv += String(e.rateHr, 2) + ",";
    csv += String(e.targetRate, 1) + ",";
    csv += String(stations[idx].cfg.planVolumeMl, 0) + ",";
    csv += String(safeNearPct(stations[idx].nearEndPct)) + ",";
    csv += String(e.alertCode) + ",";
    csv += String(alertTextEn(e.alertCode)) + ",";
    csv += String(e.rssi) + ",";
    csv += String(e.battery, 2) + "\r\n";
  }

  String filename = "IV_Report_Bed_" + String(st) + ".csv";
  server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
  server.send(200, "text/csv; charset=utf-8", csv);
}

// ----------------------------------------------------------------------------
// ข้อมูลจุดเชื่อมต่อ (อ่านอย่างเดียว) และการตั้งเวลาจากเครื่องของผู้ใช้
// ----------------------------------------------------------------------------
void handleApStatus() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  String json = "{";
  json += "\"ssid\":\"" + jsonEscape(default_ap_ssid) + "\",";
  json += "\"password\":\"" + jsonEscape(default_ap_pass) + "\",";
  json += "\"apIP\":\"" + WiFi.softAPIP().toString() + "\",";
  json += "\"channel\":" + String(currentWifiChannel()) + ",";
  json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"maxClients\":" + String(AP_MAX_CLIENTS) + ",";
  json += "\"currentTime\":\"" + getFormattedDateTime() + "\",";
  json += "\"timeSynced\":" + String(isTimeSynced ? "true" : "false") + ",";
  json += "\"timeApprox\":" + String(isTimeApprox ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// หน้าเว็บส่งเวลาของเครื่องที่เปิดดู (epoch UTC) มาตั้งให้ Host แทนการใช้ NTP
void handleTimeSet() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  if (!server.hasArg("epoch")) {
    server.send(400, "text/plain", "Missing epoch");
    return;
  }
  uint32_t epoch = (uint32_t)strtoul(server.arg("epoch").c_str(), NULL, 10);
  if (epoch < 1600000000UL) {
    server.send(400, "text/plain", "Bad epoch");
    return;
  }
  struct timeval tv = { (time_t)epoch, 0 };
  settimeofday(&tv, NULL);
  isTimeSynced = true;
  isTimeApprox = false;
  backupClockToNvs();
  lastTimeBackup = millis();
  server.send(200, "text/plain", getFormattedDateTime().c_str());
}

// ----------------------------------------------------------------------------
// หน้าเว็บ Dashboard (HTML/CSS/JavaScript) แยกไว้ในไฟล์ web_dashboard.h
// ไฟล์นั้นนิยามตัวแปร PAGE_INDEX ที่ handleRoot() ส่งให้เบราว์เซอร์
// ----------------------------------------------------------------------------
#include "web_dashboard.h"

void handleRoot() {
  if (!server.authenticate(web_username, web_password)) {
    return server.requestAuthentication();
  }
  server.send_P(200, "text/html", PAGE_INDEX);
}

// ---------------------------------------------------------------------------
// โค้ดวาดจอทั้งหมดถูกแยกไปไว้ที่ไฟล์นี้ เพื่อให้ไฟล์หลักสั้นลงและหาของเจอเร็วขึ้น
// ต้อง #include ตรงนี้เท่านั้น คือหลังตัวแปรและฟังก์ชันช่วยทั้งหมด แต่ก่อน setup()
// ห้ามย้ายขึ้นไปบนสุด เพราะโค้ดในไฟล์นั้นใช้ตัวแปรและฟังก์ชันที่ประกาศไว้ด้านบน
// ---------------------------------------------------------------------------
#include "HostScreen.h"

// ==========================================
//  Setup & Initialization
// ==========================================
void setup() {
  Serial.begin(115200);

  rtc_gpio_deinit((gpio_num_t)POWER_BTN_PIN);   // คืนขาจากโหมด RTC หลังตื่นจาก deep sleep
  pinMode(POWER_BTN_PIN, INPUT_PULLUP);
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0 || wakeup_reason == ESP_SLEEP_WAKEUP_GPIO) {
    unsigned long pressStart = millis();
    bool validHoldToTurnOn = false;

    while (digitalRead(POWER_BTN_PIN) == LOW) {
      feedWatchdog();
      if (millis() - pressStart >= 1500) {
        validHoldToTurnOn = true;
        break;
      }
      delay(10);
    }

    if (!validHoldToTurnOn) {
      while (digitalRead(POWER_BTN_PIN) == LOW) { feedWatchdog(); delay(10); }
      delay(100);
      enterDeepSleepWaitPowerButton();
    }

    while (digitalRead(POWER_BTN_PIN) == LOW) { feedWatchdog(); delay(10); }
    delay(150);
  
  bootResetReason = esp_reset_reason();   // [4.9.0-TFT] เพิ่ม: จำไว้ว่ารีบูตครั้งล่าสุดเพราะอะไร
  Serial.printf("[boot] reset reason: %s\n", resetReasonText(bootResetReason));
  setupLoopWatchdog();
}

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(HOST_BAT_ADC_PIN, INPUT);

  pinMode(PAGE_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

#if TFT_BLK >= 0
  pinMode(TFT_BLK, OUTPUT);
  digitalWrite(TFT_BLK, HIGH);
#endif
  SPI_TFT.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(240, 320);              // ความละเอียดจริงของแผง ST7789V
  tft.setSPISpeed(40000000);
  tft.setRotation(TFT_ROTATION);   // หมุนเป็นแนวนอน 320x240
  tft.setTextWrap(false);
  tft.fillScreen(0x0000);

  hostBatteryVolts = readHostBattery();
  hostBatteryPct = calculateBatteryPct(hostBatteryVolts);

  preferences.begin("sys-config", true);
  activeStationCount = preferences.getUChar("bedCount", 5);
  uint32_t savedEpoch = preferences.getUInt("lastEpoch", 0);
  preferences.end();

  // เขตเวลาไทยถาวร แล้วกู้เวลาที่สำรองไว้ก่อนไฟดับ (จะถูกแทนที่ทันทีเมื่อเปิดหน้าเว็บ)
  setenv("TZ", TZ_THAILAND, 1);
  tzset();
  if (savedEpoch > 1600000000UL) {
    struct timeval tv = { (time_t)savedEpoch, 0 };
    settimeofday(&tv, NULL);
    isTimeApprox = true;
  }
  if (activeStationCount < 1 || activeStationCount > MAX_SUPPORTED_STATIONS) activeStationCount = 5;

  loadBedConfigs();

  // ---------------- Wi-Fi: Access Point อย่างเดียว (ไม่ต่อ Router / ไม่ใช้อินเทอร์เน็ต) ----------------
  // โหมด AP ล้วนทำให้ช่องสัญญาณนิ่ง ESP-NOW ไม่หลุด และรองรับผู้ใช้พร้อมกันได้หลายเครื่อง
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(default_ap_ssid, default_ap_pass, ESPNOW_CHANNEL, 0, AP_MAX_CLIENTS);
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);   // ไม่ประหยัดพลังงาน = ตอบสนองหลายเครื่องพร้อมกันได้นิ่ง

  // ล้างรหัส Wi-Fi ของ Router ที่เคยบันทึกไว้ในเฟิร์มแวร์รุ่นก่อน (ไม่ใช้แล้ว) ครั้งเดียว
  preferences.begin("wifi-config", false);
  if (preferences.isKey("ssid")) preferences.clear();
  preferences.end();

  Serial.printf("[HOST] SoftAP ready, channel = %d, max clients = %d\n", currentWifiChannel(), AP_MAX_CLIENTS);

  MDNS.begin("iv-monitor");

  // ---------------- ESP-NOW ----------------
  if (esp_now_init() != ESP_OK) {
    Serial.println("[HOST] ESP-NOW init FAILED");
  } else {
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;            // 0 = ใช้ช่องปัจจุบันของอินเทอร์เฟซ
    peerInfo.ifidx   = WIFI_IF_AP;    // ไม่มี STA แล้ว จึงส่งผ่านอินเทอร์เฟซ AP
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  server.on("/", handleRoot);
  server.on("/api/data", handleApiData);
  server.on("/api/stations/set", HTTP_POST, handleSetStationCount);
  server.on("/api/stations/config", HTTP_POST, handleStationConfig);
  server.on("/api/stations/reset", HTTP_POST, handleStationReset);
  server.on("/api/stations/ack", HTTP_POST, handleStationAck);
  server.on("/api/logs", handleApiLogs);
  server.on("/api/logs/csv", handleDownloadCSV);
  server.on("/api/ap/status", handleApStatus);
  server.on("/api/time/set", HTTP_POST, handleTimeSet);
  server.begin();

  drawSplashScreen();
  lastUserActivityTime = millis();
  playWelcomeMelody();
  delay(1400);
  uiMode = -1;

  unsigned long nowMs = millis();
  lastCalcTime = nowMs;
  lastMinuteLogTime = nowMs;
  lastSyncBroadcastTime = nowMs;
  updateHostDisplay();
}

// ==========================================
//  Main Execution Loop
// ==========================================
void loop() {
  feedWatchdog();   // [4.9.0-TFT] เพิ่ม: บอกสุนัขเฝ้าบ้านว่ายังเดินอยู่
  server.handleClient();

  checkSmartphonePowerButton();
  checkPageButton();
  handleBuzzerAlarm();

  unsigned long currentMillis = millis();

  // สำรองเวลาลง NVS เป็นระยะ เพื่อให้เวลายังใกล้เคียงเดิมหลังไฟดับ
  if (currentMillis - lastTimeBackup >= TIME_BACKUP_INTERVAL_MS) {
    lastTimeBackup = currentMillis;
    backupClockToNvs();
  }

  // ---- ประเมินเตือน + ส่ง Sync ทุก 1 วินาที ----
  if (currentMillis - lastSyncBroadcastTime >= SYNC_INTERVAL_MS) {
    lastSyncBroadcastTime = currentMillis;
    evaluateClinicalAlerts();
    broadcastSyncToNodes();
  }

  // ---- อัปเดตค่าที่คำนวณได้ทุก 2 วินาที ----
  if (currentMillis - lastCalcTime >= 2000) {
    hostBatteryVolts = readHostBattery();
    hostBatteryPct = calculateBatteryPct(hostBatteryVolts);

    portENTER_CRITICAL(&dataMux);
    for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
      stations[i].totalVolumeMl = (float)stations[i].totalDrops / (float)safeDropFactor(stations[i].cfg.dropFactor);
      if (!stations[i].isRunning || !isStationOnline(i)) {
        stations[i].flowRate_ml_hr = 0.0;
      }
    }
    portEXIT_CRITICAL(&dataMux);
    lastCalcTime = currentMillis;
  }

  // เลือกเตียงเองแล้วปล่อยไว้ครบเวลา -> กลับไปโหมดเลือกอัตโนมัติ
  if (manualFocusBed >= 0 && (long)(currentMillis - manualFocusUntil) >= 0) {   // [4.9.0-TFT] แก้: เทียบผลต่างแบบมีเครื่องหมาย จึงทนการวนรอบของ millis()
    manualFocusBed = -1;
    uiMode = -1;
  }

  unsigned long oledRefreshInterval = displaySleeping ? 1000 : 500;
  if (currentMillis - lastOledUpdateTime >= oledRefreshInterval) {
    updateHostDisplay();
    lastOledUpdateTime = currentMillis;
  }

  // ---- บันทึก Log รายนาที ----
  if (currentMillis - lastMinuteLogTime >= 60000) {
    globalMinuteCounter++;
    String currentTimestamp = getFormattedDateTime();

    for (int i = 0; i < activeStationCount; i++) {
      StationData &s = stations[i];

      portENTER_CRITICAL(&dataMux);
      uint32_t minuteDrops = s.dropsInCurrentMinute;
      s.dropsInCurrentMinute = 0;
      portEXIT_CRITICAL(&dataMux);

      uint8_t df = safeDropFactor(s.cfg.dropFactor);
      LogEntry entry;
      entry.minuteIndex = globalMinuteCounter;
      snprintf(entry.timeStr, sizeof(entry.timeStr), "%s", currentTimestamp.c_str());
      entry.drops = (minuteDrops > 65535) ? 65535 : (uint16_t)minuteDrops;
      entry.volume = s.totalVolumeMl;
      entry.rateHr = ((float)minuteDrops / (float)df) * 60.0f;   // mL/h จากหยดจริงใน 1 นาที
      entry.targetRate = s.cfg.targetRateHr;
      entry.alertCode = s.alertCode;
      entry.rssi = isStationOnline(i) ? s.rssi : 0;
      entry.battery = s.batteryVolts;

      if (s.logCount < MAX_LOGS) {
        s.logs[s.logCount++] = entry;
      } else {
        memmove(&s.logs[0], &s.logs[1], sizeof(LogEntry) * (MAX_LOGS - 1));
        s.logs[MAX_LOGS - 1] = entry;
      }
    }
    lastMinuteLogTime += 60000;
  }
}
