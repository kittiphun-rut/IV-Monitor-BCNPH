/**
 * ============================================================================
 * โครงการวิจัย: ผลของการใช้นวัตกรรม Smart IV Alert ต่อความแม่นยำในการแจ้งเตือนและปริมาณสารน้ำที่ได้รับ
 * หน่วยงาน: หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * สถาบัน: วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 *
 * ระบบ: Central Host Gateway (เครื่องควบคุมและติดตามศูนย์กลาง)
 * เวอร์ชัน: 4.7.0 (Protocol v3 — ใช้คู่กับ Bed Station Firmware 7.4.x เท่านั้น)
 * บอร์ดประมวลผล: ESP32-S3 Dev Module (N16R8) + จอ 1.3" OLED (SH1106 I2C) + Passive Buzzer
 *
 * ผู้พัฒนาระบบ: นายกิตติพันธ์ รัตนคร (นักวิชาการคอมพิวเตอร์ มจร. วิทยาเขตแพร่)
 * อาจารย์ที่ปรึกษา: ดร.กรรณิการ์ กาศสมบูรณ์ (วิทยาลัยพยาบาลบรมราชชนนี แพร่)
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V4.7.0: ตัดระบบจัดการ Wi-Fi (Router/อินเทอร์เน็ต) ออกทั้งหมด
 *  - Host ทำงานเป็น Access Point อย่างเดียว (WIFI_AP) ไม่ต่อ Router ไม่ใช้ NTP
 *    -> ไม่มีการสแกนช่อง/รีคอนเนกต์มาแย่งเวลาคลื่นวิทยุ ESP-NOW และ Web Server อีก
 *  - รองรับสมาร์ตโฟน/แท็บเล็ตพร้อมกันได้ถึง AP_MAX_CLIENTS เครื่อง (เดิมค่าปริยาย 4
 *    และในโหมด AP+STA เหลือใช้งานจริงเพียง 2-3 เครื่อง)
 *  - ปิดโหมดประหยัดพลังงานของ Wi-Fi (WIFI_PS_NONE) ให้ตอบสนองหลายเครื่องพร้อมกันได้นิ่ง
 *  - ESP-NOW ส่งผ่านอินเทอร์เฟซ AP (WIFI_IF_AP) เพราะไม่มี STA แล้ว
 *  - ตั้งเวลาจากนาฬิกาของเครื่องที่เปิดหน้าเว็บโดยอัตโนมัติ (แทน NTP) และสำรองเวลาไว้ใน NVS
 *    ทุก 10 นาที เพื่อให้เวลาไม่หายเมื่อไฟดับ (แสดงเป็น "เวลาโดยประมาณ" จนกว่าจะซิงก์ใหม่)
 *  - หน้าเว็บ: เอาเมนูจัดการ Wi-Fi ออก เหลือหน้าต่างแสดงข้อมูลจุดเชื่อมต่อแบบอ่านอย่างเดียว
 *    และหยุด Poll ข้อมูลเมื่อสลับแท็บไปทำอย่างอื่น (ลดภาระเมื่อมีผู้ใช้หลายเครื่อง)
 *
 * ---------------------------------------------------------------------------
 * เพิ่มใน V4.6.0: แจ้งเตือนใกล้หมด (เตรียมถุงใหม่)
 *  - ตั้ง % ต่อเตียงจากหน้าเว็บ (50-95%, ค่าเริ่มต้น 80%) เก็บถาวรใน NVS
 *  - Host เสียงเตือนเบา 3 ครั้ง ซ้ำทุก 5 นาทีจนกว่าจะรับทราบ (ที่เตียง / ปุ่ม POWER / หน้าเว็บ)
 *  - รับทราบแล้วซิงก์ไปทุกจุด และล้างเมื่อกด "เริ่มถุงใหม่" หรือแก้ปริมาตร/เปอร์เซ็นต์
 *
 * สรุปการแก้ไขจาก V4.4.1 (โค้ดภายในระบุ 6.3.2)
 *  1) แก้จุดที่คอมไพล์ไม่ผ่าน (ฟิลด์ใน StationData / ชื่อฟังก์ชัน-ตัวแปรใน loop)
 *  2) โครงสร้างแพ็กเก็ต ESP-NOW ตรงกับ Station ทุกไบต์ (มี static_assert ตรวจขนาด)
 *  3) Station ส่งอัตราไหลที่คำนวณจากช่วงห่างระหว่างหยด + เวลาตั้งแต่หยดล่าสุด
 *     -> เลิกคำนวณ rate จากหน้าต่าง 2 วินาทีที่ทำให้แจ้งเตือนสายพับผิด
 *  4) Host เป็นผู้ตัดสินรหัสเตือนจุดเดียว เรียก evaluateClinicalAlerts() ทุก 1 วินาที
 *  5) ส่ง Sync ทุก 1 วินาที (เดิม 10 วินาที ขณะที่ Station ตัดสินว่าหลุดที่ 5 วินาที)
 *  6) เพิ่ม API /api/stations/config และ /api/stations/reset
 *     ค่า Target Rate / Plan Volume / Drop Factor / ชื่อผู้ป่วย เก็บที่ Host (NVS)
 *  7) Drop factor ใช้ค่าของแต่ละเตียง (10/15/20/60) แทนค่าคงที่ 20
 *  8) นับหยดรายนาทีจากผลต่าง totalDrops -> แพ็กเก็ตหายไม่ทำให้ข้อมูลหาย
 *  9) SoftAP ล็อก Channel 1
 * 10) Deep sleep ใช้ ext0 wakeup (esp_deep_sleep_enable_gpio_wakeup ไม่รองรับ ESP32-S3)
 * 11) ปุ่ม POWER กดสั้นขณะมีเสียงเตือน = พักเสียง 2 นาที (Snooze)
 * ============================================================================
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <time.h>
#include <sys/time.h>
#include <driver/rtc_io.h>

#define APP_VERSION         "4.7.0"
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
#define OLED_SDA_PIN        8   // ขา I2C SDA จอ OLED
#define OLED_SCL_PIN        9   // ขา I2C SCL จอ OLED

// ----------------------------------------------------------------------------
// ค่าคงที่ของระบบสื่อสารและการแจ้งเตือน (ต้องตรงกับ Station)
// ----------------------------------------------------------------------------
#define ESPNOW_CHANNEL          1        // ช่อง SoftAP / ESP-NOW เริ่มต้น
#define SYNC_INTERVAL_MS        1000     // ส่ง Sync ให้ Station ทุก 1 วินาที
#define ONLINE_TIMEOUT_MS       5000     // ไม่ได้รับข้อมูลเกิน 5 วินาที = OFFLINE
#define MIN_OCCLUSION_MS        8000     // เวลาต่ำสุดที่ไม่มีหยดก่อนถือว่าหยุดไหล
#define MAX_OCCLUSION_MS        300000   // เพดานเวลาตัดสินหยุดไหล (อัตราต่ำมาก)
#define RATE_DEVIATION_HOLD_MS  20000    // เร็ว/ช้าเกินต่อเนื่อง 20 วินาทีจึงเตือน
#define SNOOZE_MS               120000   // พักเสียง 2 นาที
#define NEAR_END_REMIND_MS      300000   // เตือนใกล้หมดซ้ำทุก 5 นาทีจนกว่าจะรับทราบ
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

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

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
uint8_t currentOledPage     = 0;
bool oledDisplaySleeping    = false;
bool oledScreensaverActive  = false;
bool isPowerOffProgressActive = false;
unsigned long lastUserActivityTime = 0;
bool isTimeSynced           = false;   // ตั้งเวลาจากเครื่องผู้ใช้ผ่านหน้าเว็บแล้ว
bool isTimeApprox           = false;   // กู้เวลาจากที่สำรองไว้ตอนบูต (ยังไม่ซิงก์ใหม่)
unsigned long lastTimeBackup = 0;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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

void updateHostOLED();

// ----------------------------------------------------------------------------
// ฟังก์ชันช่วย
// ----------------------------------------------------------------------------
uint8_t safeDropFactor(uint8_t df) {
  return (df == 10 || df == 15 || df == 20 || df == 60) ? df : 20;
}

bool isCriticalAlert(uint8_t code) {
  return code == ALERT_TOO_FAST || code == ALERT_TOO_SLOW ||
         code == ALERT_COMPLETE || code == ALERT_OCCLUSION;
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

const char* alertTextEn(uint8_t code) {
  switch (code) {
    case ALERT_TOO_FAST:  return "TOO FAST";
    case ALERT_TOO_SLOW:  return "TOO SLOW";
    case ALERT_NEAR_END:  return "NEXT BAG";
    case ALERT_COMPLETE:  return "BAG EMPTY";
    case ALERT_OCCLUSION: return "OCCLUSION";
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

String getOledClockStr() {
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
    syncMsg.alertCode    = stations[i].alertCode;
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
  rtc_gpio_pullup_en((gpio_num_t)POWER_BTN_PIN);
  rtc_gpio_pulldown_dis((gpio_num_t)POWER_BTN_PIN);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BTN_PIN, 0);
  esp_deep_sleep_start();
}

void powerOffSystem() {
  isPowerOffProgressActive = true;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(24, 24, "GOODBYE...");
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(12, 42, "Release BTN to OFF");
  u8g2.drawStr(8, 56, "Hold 2s to Power ON");
  u8g2.sendBuffer();

  playShutdownMelody();

  while (digitalRead(POWER_BTN_PIN) == LOW) delay(20);
  delay(200);

  u8g2.clearBuffer();
  u8g2.sendBuffer();
  u8g2.setPowerSave(1);

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
    if (!isStationOnline(i) || !s.isRunning) {
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

    if (holdDuration >= 450 && !oledDisplaySleeping) {
      isPowerOffProgressActive = true;

      if (millis() - lastProgressFrameTime >= 30) {
        lastProgressFrameTime = millis();
        int progressWidth = map(holdDuration, 450, 2000, 0, 100);
        if (progressWidth > 100) progressWidth = 100;

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(16, 20, "POWER OFF ?");
        u8g2.setFont(u8g2_font_5x8_tf);
        u8g2.drawStr(10, 35, "Keep holding to OFF");
        u8g2.drawFrame(14, 43, 100, 11);
        u8g2.drawBox(14, 43, progressWidth, 11);
        u8g2.sendBuffer();
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
        if (oledDisplaySleeping) {
          oledDisplaySleeping = false;
          u8g2.setPowerSave(0);
        }
        oledScreensaverActive = false;
        isPowerOffProgressActive = false;
        updateHostOLED();
      } else if (totalHoldTime < 400 && anyNearEndPending()) {
        // กดสั้นขณะมีเตือนใกล้หมดที่ยังไม่รับทราบ = รับทราบทุกเตียง
        acknowledgeAllNearEnd();
        tone(BUZZER_PIN, 2600, 40);
        if (oledDisplaySleeping) { oledDisplaySleeping = false; u8g2.setPowerSave(0); }
        oledScreensaverActive = false;
        isPowerOffProgressActive = false;
        updateHostOLED();
      } else if (oledScreensaverActive) {
        oledScreensaverActive = false;
        isPowerOffProgressActive = false;
        updateHostOLED();
      } else if (totalHoldTime < 400) {
        oledDisplaySleeping = !oledDisplaySleeping;
        u8g2.setPowerSave(oledDisplaySleeping ? 1 : 0);
        tone(BUZZER_PIN, 1800, 25);
        if (!oledDisplaySleeping) {
          isPowerOffProgressActive = false;
          updateHostOLED();
        }
      }
      else if (isPowerOffProgressActive && !oledDisplaySleeping) {
        isPowerOffProgressActive = false;
        updateHostOLED();
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
    if (oledScreensaverActive) {
      oledScreensaverActive = false;
    } else if (clickCount >= 2) {
      oledScreensaverActive = true;
    } else {
      currentOledPage = (currentOledPage + 1) % (activeStationCount + 1);
    }
    updateHostOLED();
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
void renderBlockStyleGrid() {
  int topY = 11;
  int bottomY = 53;
  int totalH = bottomY - topY;

  int onlineCount = 0;
  int alarmBed = 0;
  uint8_t alarmCode = ALERT_NONE;

  for (int i = 0; i < activeStationCount; i++) {
    if (isStationOnline(i)) onlineCount++;
    if (alarmBed == 0 && isCriticalAlert(stations[i].alertCode)) {
      alarmBed = i + 1;
      alarmCode = stations[i].alertCode;
    }
  }

  if (activeStationCount == 1) {
    u8g2.drawRFrame(0, topY + 1, 128, totalH - 1, 3);
    u8g2.drawHLine(0, topY + 13, 128);
    bool online = isStationOnline(0);

    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(6, topY + 9, "BED 01 BLOCK MONITOR");

    if (!online) {
      u8g2.setFont(u8g2_font_helvB10_tf);
      u8g2.drawStr(12, topY + 30, "OFFLINE");
    } else if (!stations[0].isRunning) {
      u8g2.setFont(u8g2_font_helvB10_tf);
      u8g2.drawStr(12, topY + 30, "PAUSED");
    } else {
      char rBuf[16];
      snprintf(rBuf, sizeof(rBuf), "%3.1f", stations[0].flowRate_ml_hr);
      u8g2.setFont(u8g2_font_logisoso16_tf);
      u8g2.drawStr(6, topY + 33, rBuf);
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(72, topY + 26, "mL/h");
    }

    char subBuf[40];
    snprintf(subBuf, sizeof(subBuf), "Vol:%3.1f/%.0fmL | %ddBm",
             stations[0].totalVolumeMl, stations[0].cfg.planVolumeMl, stations[0].rssi);
    u8g2.setFont(u8g2_font_4x6_tf);
    u8g2.drawStr(6, topY + 41, subBuf);
  }
  else if (activeStationCount == 2) {
    int cardW = 62;
    for (int i = 0; i < 2; i++) {
      int x = (i == 0) ? 0 : 66;
      u8g2.drawRBox(x, topY + 1, cardW, totalH - 1, 2);

      bool online = isStationOnline(i);

      char idStr[10];
      snprintf(idStr, sizeof(idStr), "BED %02d", i + 1);
      u8g2.setDrawColor(0);
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(x + 4, topY + 10, idStr);

      if (!online) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(x + 8, topY + 26, "OFFLINE");
      } else if (!stations[i].isRunning) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(x + 10, topY + 26, "PAUSED");
      } else {
        char rBuf[16];
        snprintf(rBuf, sizeof(rBuf), "%3.0f", stations[i].flowRate_ml_hr);
        u8g2.setFont(u8g2_font_helvB12_tf);
        u8g2.drawStr(x + 6, topY + 28, rBuf);
        u8g2.setFont(u8g2_font_4x6_tf);
        u8g2.drawStr(x + 38, topY + 28, "mh");
      }

      char vBuf[16];
      snprintf(vBuf, sizeof(vBuf), "V:%3.0fmL", stations[i].totalVolumeMl);
      u8g2.setFont(u8g2_font_4x6_tf);
      u8g2.drawStr(x + 4, topY + 38, vBuf);
      u8g2.setDrawColor(1);
    }
  }
  else {
    int cardW = 62;
    int rows = (activeStationCount + 1) / 2;
    int rowH = (rows <= 3) ? 14 : 10;   // 3-6 เตียง = 3 แถว, 7-8 เตียง = 4 แถว
    for (int i = 0; i < activeStationCount; i++) {
      int col = i % 2;
      int row = i / 2;
      int x = (col == 0) ? 0 : 65;
      int y = topY + row * rowH;
      int textY = y + ((rowH == 14) ? 9 : 7);

      bool online = isStationOnline(i);
      bool critical = online && stations[i].isRunning && isCriticalAlert(stations[i].alertCode);
      // เตียงที่มีเหตุวิกฤตแสดงเป็นกรอบทึบ (ตัวอักษรกลับสี) เพื่อให้เห็นชัดจากระยะไกล
      if (critical) { u8g2.drawRBox(x, y, cardW, rowH - 1, 2); u8g2.setDrawColor(0); }
      else u8g2.drawRFrame(x, y, cardW, rowH - 1, 2);

      char idStr[8];
      snprintf(idStr, sizeof(idStr), "B%d", i + 1);
      u8g2.setFont(u8g2_font_4x6_tf);
      u8g2.drawStr(x + 3, textY, idStr);

      if (!online) {
        u8g2.drawStr(x + 14, textY, "OFFLINE");
      } else if (!stations[i].isRunning) {
        u8g2.drawStr(x + 14, textY, "PAUSED");
      } else {
        char rBuf[24];
        snprintf(rBuf, sizeof(rBuf), "%3.0fmh %4.0fmL", stations[i].flowRate_ml_hr, stations[i].totalVolumeMl);
        u8g2.drawStr(x + 11, textY, rBuf);
      }
      u8g2.setDrawColor(1);
    }
  }

  u8g2.drawHLine(0, 53, 128);
  u8g2.setFont(u8g2_font_5x8_tf);
  if (alarmBed > 0) {
    char alertBuf[32];
    snprintf(alertBuf, sizeof(alertBuf), "%sB%d %s", isSnoozed() ? "(ZZ) " : "! ", alarmBed, alertTextEn(alarmCode));
    u8g2.drawBox(0, 54, 128, 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(2, 62, alertBuf);
    u8g2.setDrawColor(1);
  } else {
    int nearBed = 0;
    for (int i = 0; i < activeStationCount; i++) {
      if (stations[i].alertCode == ALERT_NEAR_END && !stations[i].nearEndAck) { nearBed = i + 1; break; }
    }
    char statBuf[32];
    if (nearBed > 0) {
      const StationData &ns = stations[nearBed - 1];
      int pct = (ns.cfg.planVolumeMl > 0) ? (int)(ns.totalVolumeMl * 100.0f / ns.cfg.planVolumeMl) : 0;
      snprintf(statBuf, sizeof(statBuf), "B%d NEXT BAG %d%% [PWR]", nearBed, pct);
      u8g2.drawFrame(0, 54, 128, 10);
      u8g2.drawStr(2, 62, statBuf);
    } else {
      snprintf(statBuf, sizeof(statBuf), "CH%d WiFi:%d | Bed:%d/%d", currentWifiChannel(),
               (int)WiFi.softAPgetStationNum(), onlineCount, activeStationCount);
      u8g2.drawStr(0, 62, statBuf);
    }
  }
}

void updateHostOLED() {
  if (oledDisplaySleeping || isPowerOffProgressActive) return;
  u8g2.clearBuffer();

  bool nearPending = anyNearEndPending();
  if (!oledScreensaverActive && !globalAlarmTriggered && !nearPending && (millis() - lastUserActivityTime >= 300000)) {
    oledScreensaverActive = true;
  }
  // มีเหตุวิกฤต (ยังไม่พักเสียง) หรือเตือนใกล้หมดที่ยังไม่รับทราบ -> ออกจาก Screensaver เพื่อแสดงเตือน
  if (oledScreensaverActive && ((globalAlarmTriggered && !isSnoozed()) || nearPending)) {
    oledScreensaverActive = false;
    currentOledPage = 0;
  }

  if (oledScreensaverActive) {
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(20, 10, "SMART IV CLOCK");
    u8g2.drawHLine(10, 13, 108);

    u8g2.setFont(u8g2_font_logisoso32_tf);
    String clockStr = getOledClockStr();
    u8g2.drawStr(4, 56, clockStr.c_str());
  }
  else if (currentOledPage == 0) {
    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(0, 7, "BLOCK MONITOR");
    char hBatBuf[16];
    snprintf(hBatBuf, sizeof(hBatBuf), "Beds:%d", activeStationCount);
    u8g2.drawStr(92, 7, hBatBuf);
    u8g2.drawHLine(0, 9, 128);

    renderBlockStyleGrid();
  }
  else if (currentOledPage >= 1 && currentOledPage <= activeStationCount) {
    int idx = currentOledPage - 1;
    const StationData &s = stations[idx];
    bool isOnline = isStationOnline(idx);
    bool isPaused = isOnline && !s.isRunning;

    u8g2.setFont(u8g2_font_5x8_tf);
    char headBuf[24];
    snprintf(headBuf, sizeof(headBuf), "BED %02d DETAIL  Df:%d", currentOledPage, safeDropFactor(s.cfg.dropFactor));
    u8g2.drawStr(0, 7, headBuf);
    u8g2.drawHLine(0, 9, 128);

    char rateBuf[28];
    if (!isOnline) snprintf(rateBuf, sizeof(rateBuf), "Rate: OFFLINE");
    else if (isPaused) snprintf(rateBuf, sizeof(rateBuf), "Rate: 0.0 (PAUSED)");
    else snprintf(rateBuf, sizeof(rateBuf), "Rate: %3.1f mL/h", s.flowRate_ml_hr);
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 21, rateBuf);

    u8g2.setFont(u8g2_font_5x8_tf);
    char volBuf[32];
    snprintf(volBuf, sizeof(volBuf), "Vol:%.0f/%.0fmL Set:%.0f", s.totalVolumeMl, s.cfg.planVolumeMl, s.cfg.targetRateHr);
    u8g2.drawStr(0, 32, volBuf);

    char statusBuf[32];
    if (!isOnline) snprintf(statusBuf, sizeof(statusBuf), "Stat: NO SIGNAL");
    else if (isPaused) snprintf(statusBuf, sizeof(statusBuf), "Stat: [PAUSED / STOP]");
    else if (s.alertCode == ALERT_NEAR_END) snprintf(statusBuf, sizeof(statusBuf), "Stat: NEXT BAG %s", s.nearEndAck ? "(ACK)" : "!");
    else if (s.alertCode != ALERT_NONE) snprintf(statusBuf, sizeof(statusBuf), "Stat: !%s!", alertTextEn(s.alertCode));
    else snprintf(statusBuf, sizeof(statusBuf), "Stat: NORMAL FLOW");
    u8g2.drawStr(0, 42, statusBuf);

    char nodeBatBuf[32];
    if (s.batteryVolts > 0.5) snprintf(nodeBatBuf, sizeof(nodeBatBuf), "RSSI:%ddBm | %1.2fV", s.rssi, s.batteryVolts);
    else snprintf(nodeBatBuf, sizeof(nodeBatBuf), "RSSI:%ddBm | Bat:N/A", s.rssi);
    u8g2.drawStr(0, 52, nodeBatBuf);

    u8g2.drawHLine(0, 55, 128);
    u8g2.drawStr(0, 63, "[PAGE:Next | Dbl:Clock]");
  }

  u8g2.sendBuffer();
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

      if (currentOledPage > activeStationCount) currentOledPage = 0;
      updateHostOLED();
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
      if (millis() - pressStart >= 1500) {
        validHoldToTurnOn = true;
        break;
      }
      delay(10);
    }

    if (!validHoldToTurnOn) {
      while (digitalRead(POWER_BTN_PIN) == LOW) delay(10);
      delay(100);
      enterDeepSleepWaitPowerButton();
    }

    while (digitalRead(POWER_BTN_PIN) == LOW) delay(10);
    delay(150);
  }

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(HOST_BAT_ADC_PIN, INPUT);

  pinMode(PAGE_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  u8g2.begin();
  u8g2.clearBuffer();

  u8g2.drawRFrame(0, 0, 128, 64, 4);
  u8g2.setFont(u8g2_font_7x14B_tf);
  u8g2.drawStr(14, 18, "SMART IV ALERT");
  u8g2.drawHLine(8, 23, 112);

  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(14, 35, "Central Host Gateway");
  u8g2.setFont(u8g2_font_4x6_tf);
  u8g2.drawStr(14, 45, "BCN Phrae & MCU Collab");
  u8g2.drawStr(14, 55, "Host v" APP_VERSION " / Proto v3");
  u8g2.sendBuffer();

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

  lastUserActivityTime = millis();
  playWelcomeMelody();
  delay(1400);

  unsigned long nowMs = millis();
  lastCalcTime = nowMs;
  lastMinuteLogTime = nowMs;
  lastSyncBroadcastTime = nowMs;
  updateHostOLED();
}

// ==========================================
//  Main Execution Loop
// ==========================================
void loop() {
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

  unsigned long oledRefreshInterval = (currentOledPage == 0 || oledDisplaySleeping) ? 1000 : 250;
  if (currentMillis - lastOledUpdateTime >= oledRefreshInterval) {
    updateHostOLED();
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
