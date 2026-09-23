/**
 * @file      Station-C3-OLED.ino
 * @brief     เฟิร์มแวร์เครื่องประจำเตียง รุ่นบอร์ดเล็ก จอ OLED 0.42 นิ้ว
 * @version   1.0.0-C3
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
 *   5) ตัวตรวจจับหยดใช้ drop_detector.h ตัวเดียวกับสายหลัก เรียนรู้เองไม่ต้องคาลิเบรต
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

#include "drop_detector.h"   // อัลกอริทึมตรวจจับหยด (ทดสอบบน PC ได้: tools/detector-test)

#define APP_VERSION         "1.0.0-C3"

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
enum UiMode : uint8_t { MODE_NORMAL = 0, MODE_SET_ID, MODE_ALERT, MODE_SAVER };

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
  myData.stationId       = stationId;
  myData.isRunning       = isRunning ? 1 : 0;
  myData.totalDrops      = totalDrops;
  myData.periodDrops     = isRunning ? periodDropsCounter : 0;
  myData.flowRateHr      = isRunning ? currentFlowRate_ml_hr : 0.0f;
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

    if (held >= 4000 && !veryLongHandled) {
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
    if (uiMode == MODE_SET_ID) {
      stationId = (stationId % 8) + 1;             // กดสั้นในหน้าตั้งเลข = เพิ่มเลขเตียง
      soundClick();
    } else if (uiMode == MODE_SAVER) {
      uiMode = MODE_NORMAL;                         // ปลุกจากหน้าพักจอ
      soundClick();
    } else if (uiMode == MODE_ALERT && uiAlertCode == ALERT_NEAR_END) {
      nearEndAckRequest = true;                     // รับทราบที่เตียง ส่งกลับ Host
      soundConfirm();
    } else if (clickCount >= 2) {
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

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  u8g2.begin();
  drawSplash();
  beepPattern(2, 40, 45);

  // ---- ตัวตรวจจับหยด ----
  iv::DetectorConfig dcfg;
  detector.begin(dcfg);
  detector.reset(analogRead(SENSOR_AO_PIN));

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
  unsigned long now = millis();

  // อ่านเซนเซอร์ให้ถี่ที่สุด — ตัวตรวจจับจัดจังหวะตัวอย่างเอง
  serviceDropSensor(isRunning && uiMode != MODE_SET_ID);

  serviceBeep(now);
  handleButton(now);
  serviceUiMode(now);
  manageChannelHunting(now);

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
