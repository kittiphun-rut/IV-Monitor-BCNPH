/**
 * @file      Host-Touch24.ino
 * @brief     เฟิร์มแวร์เครื่องส่วนกลาง รุ่นจอสัมผัส 2.4 นิ้ว ตั้งค่าได้ที่หน้าเครื่อง
 * @version   4.10.0-TOUCH
 * @date      2026-09-23
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Hardware
 * ESP32-S3 Dev Module (N16R8) + จอสัมผัส 2.4" (ILI9341 + XPT2046 SPI 240x320)
 * นาฬิกา DS3231 และการ์ด SD สำหรับบันทึกข้อมูลย้อนหลัง
 *
 * @par Description
 * ทำหน้าที่เหมือนรุ่นจอ OLED ทุกอย่าง และเพิ่มการตั้งค่าทั้งหมดบนจอสัมผัส
 * ใช้งานคล้ายสมาร์ตโฟน มีแป้นตัวเลขสำหรับกรอกค่าตามแผนการรักษาที่หน้าเครื่อง
 * จึงตั้งค่าได้โดยไม่ต้องเปิดหน้าเว็บ
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 4.10.0-TOUCH | 2026-09-23 | เตียงที่หายไประหว่างให้น้ำเกลือมีเสียงเตือนแล้ว เพิ่มสุนัขเฝ้าบ้าน รายงานสาเหตุการรีบูต และปฏิเสธแพ็กเก็ตที่ค่าเป็นไปไม่ได้ |
 * | 4.9.3-TOUCH | 2026-09-23 | ย้ายโค้ดวาดจอออกไปไว้ที่ `HostScreen.h` ตรรกะไม่เปลี่ยนแม้แต่บรรทัดเดียว |
 * | 4.9.2-TOUCH | 2026-09-23 | พอร์ตคุณสมบัติและการแก้บั๊กทั้งหมดจากรุ่นจอ OLED v4.7.3-v4.7.5 มาครบ |
 * | 4.9.0-TOUCH | 2026-09-12 | สร้างรุ่นจอสัมผัส 2.4" ตั้งค่าได้บนจอ พร้อมนาฬิกา DS3231 และการ์ด SD |
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
 *                  + SD card (ช่องบนหลังจอ) + นาฬิกา DS3231 + Passive Buzzer + ปุ่ม multifunction
 *
 *
 *  เพิ่มใน V4.9.1: นาฬิกาสำรอง DS3231 และการบันทึกข้อมูลลง SD card
 *   - DS3231 (I2C) เก็บวันที่-เวลาไว้แม้ไฟดับ อ่านค่ามาตั้งเวลาระบบตอนบูตทันที
 *     และเมื่อเปิดหน้าเว็บจากมือถือ เวลาของเครื่องนั้นจะถูกเขียนลง DS3231 ให้ด้วย
 *     (เก็บเป็น UTC แล้วแปลงเป็นเวลาไทยตอนแสดงผล) ถ้าไม่มีโมดูลก็ยังใช้งานได้เหมือนเดิม
 *   - บันทึกข้อมูลรายนาทีของทุกเตียงลง SD card เป็นไฟล์ CSV วันละไฟล์
 *     พร้อมไฟล์บันทึกเหตุการณ์ (เกิดเหตุเตือน เริ่มถุงใหม่ แก้ค่า ตั้งเวลา) แยกอีกไฟล์
 *     ดาวน์โหลดไฟล์จากหน้าเว็บได้ และดูสถานะการ์ดได้จากหน้าตั้งค่าบนจอ
 *   - ถอด/ใส่การ์ดกลางคันได้ ระบบจะลองต่อการ์ดใหม่ให้เองทุก 30 วินาที
 *
 *  เปลี่ยนใน V4.9.0-TOUCH: ใช้จอสัมผัส 2.4" และทำหน้าจอให้ใช้งานเหมือนสมาร์ตโฟน
 *   - จอแนวตั้ง 240x320 พร้อมทัชสกรีนแบบความต้านทาน (XPT2046) ใช้ร่วมบัส SPI เดียวกับจอ
 *   - หน้าหลักแบบ FOCUS: เตียงที่ต้องดูตอนนี้ตัวใหญ่ + รายการเตียงอื่น แตะที่ใดก็เข้าดูเตียงนั้นได้
 *   - แตะเข้าไปในเตียงเพื่อสั่งงาน: เริ่มถุงใหม่ / รับทราบเตือนใกล้หมด / ตั้งค่าของเตียง
 *   - ตั้งค่าได้บนจอโดยตรง: อัตราเป้าหมาย ปริมาตรตามแผน Drop factor % เตือนใกล้หมด
 *     จำนวนเตียงที่ใช้งาน และความสว่างหน้าจอ (ปรับด้วยปุ่ม - + หรือแป้นตัวเลขบนจอ)
 *     ทุกค่ายังแก้จากหน้าเว็บได้เหมือนเดิม และบันทึกลง NVS ชุดเดียวกัน
 *   - มีหน้าปรับความแม่นของจอสัมผัส (Calibration) เก็บค่าไว้ใน NVS
 *   - ปุ่ม multifunction 1 ปุ่ม: คลิก = ย้อนกลับ/พักเสียง, ดับเบิลคลิก = ปิด/เปิดจอ,
 *     กดค้าง 2 วินาที = ปิดเครื่อง
 *   - หรี่จออัตโนมัติเมื่อไม่มีการใช้งาน 3 นาที แตะหน้าจอเพื่อกลับมาสว่าง
 *   - ตรรกะการวัด การแจ้งเตือน ESP-NOW และหน้าเว็บทั้งหมดเหมือน V4.8.0 ทุกประการ
 *
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
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <FS.h>
#include <SD.h>
#include <time.h>
#include <sys/time.h>
#include <driver/rtc_io.h>
#include <esp_task_wdt.h>          // [4.10.0-TOUCH] เพิ่ม: สุนัขเฝ้าบ้าน กันเครื่องค้างเงียบ
#include <esp_system.h>            // [4.10.0-TOUCH] เพิ่ม: อ่านสาเหตุการรีบูตครั้งล่าสุด

// ----------------------------------------------------------------------------
// สุนัขเฝ้าบ้านและสาเหตุการรีบูต ([4.10.0-TOUCH] เพิ่มทั้งหมด)
//
// เครื่องนี้ทำหน้าที่เตือนภัย การค้างแบบเงียบจึงอันตรายกว่าการรีบูต เพราะจอยัง
// ค้างภาพเดิมไว้ พยาบาลจึงเข้าใจว่าระบบยังเฝ้าอยู่ ทั้งที่หยุดไปแล้ว
// ตั้งไว้ 8 วินาที ซึ่งยาวกว่ารอบ loop() ปกติหลายเท่า จึงไม่รีบูตเพราะงานหนักชั่วคราว
// ----------------------------------------------------------------------------
#define LOOP_WDT_TIMEOUT_S      8

esp_reset_reason_t bootResetReason = ESP_RST_UNKNOWN;


#define APP_VERSION         "4.10.0-TOUCH"
#define DEV_NAME            "กิตติพันธ์ รัตนคร"
#define DEV_ROLE            "นักวิชาการคอมพิวเตอร์"
#define DEV_INSTITUTION     "มหาวิทยาลัยมหาจุฬาลงกรณราชวิทยาลัย วิทยาเขตแพร่"

// ----------------------------------------------------------------------------
// กำหนดขาเชื่อมต่อฮาร์ดแวร์ฝั่ง Host
// ----------------------------------------------------------------------------
#define HOST_BAT_ADC_PIN    1   // ขาอ่านแบตเตอรี่ Host
#define POWER_BTN_PIN       4   // ปุ่มเปิด-ปิดหน้าจอ / พักเสียงเตือน (Snooze) — ต้องเป็น RTC GPIO (0-21)
// เวอร์ชันนี้ใช้ปุ่มเดียว (POWER_BTN_PIN) ทำงานหลายหน้าที่ — ขา 5 จึงว่างไว้เผื่ออนาคต
#define BUZZER_PIN          7   // ขาต่อลำโพง Passive Buzzer

// ---- จอสัมผัส 2.4" (จอ 14 ขา + ทัช 4-5 ขา ใช้บัส SPI ร่วมกัน) ----
// ต่อสายจอ : VCC->3V3, GND->GND, SCL/SCK->TFT_SCLK, SDA/MOSI->TFT_MOSI,
//            RES->TFT_RST, DC->TFT_DC, CS->TFT_CS, BLK->TFT_BLK
// ต่อสายทัช: T_CLK->TFT_SCLK (ร่วมกัน), T_DIN->TFT_MOSI (ร่วมกัน),
//            T_DO->TFT_MISO, T_CS->TOUCH_CS_PIN, T_IRQ->TOUCH_IRQ_PIN (ต่อหรือไม่ต่อก็ได้)
// หมายเหตุ: บอร์ด N16R8 ใช้ GPIO33-37 กับ PSRAM ห้ามนำมาใช้
#define TFT_CS              10
#define TFT_DC              9
#define TFT_RST             8
#define TFT_MOSI            11
#define TFT_SCLK            12
#define TFT_MISO            16  // จำเป็นสำหรับอ่านค่าจากทัช (จอไม่ได้ใช้)
#define TFT_BLK             13  // -1 = ไม่ได้ต่อขาควบคุมไฟหน้าจอ
#define TFT_ROTATION        0   // 0 = แนวตั้ง 240x320 (ถ้าภาพกลับหัวให้ใช้ 2)

#define TOUCH_CS_PIN        14
#define TOUCH_IRQ_PIN       15  // ตั้งเป็น 255 ถ้าไม่ได้ต่อขา T_IRQ
#define TOUCH_MIN_PRESSURE  200 // แรงกดต่ำสุดที่ถือว่าเป็นการแตะจริง

// ปรับทิศทางของทัชให้ตรงกับภาพบนจอ (ถ้าแตะแล้วตำแหน่งสลับแกนหรือกลับด้าน ให้แก้ 3 ค่านี้)
#define TOUCH_SWAP_XY       0
#define TOUCH_FLIP_X        0
#define TOUCH_FLIP_Y        0

#define BACKLIGHT_DIM_MS    180000  // ไม่มีการใช้งาน 3 นาที -> หรี่จอ
#define BACKLIGHT_DIM_PCT   20

// ---- SD card บนหลังจอ (ใช้บัส SPI ร่วมกับจอและทัช ต่างกันที่ขา CS) ----
#define SD_CS_PIN           21
#define SD_SPI_HZ           20000000

// ---- นาฬิกาสำรอง DS3231 (I2C) ----
#define RTC_SDA_PIN         5
#define RTC_SCL_PIN         6

// ----------------------------------------------------------------------------
// ค่าคงที่ของระบบสื่อสารและการแจ้งเตือน (ต้องตรงกับ Station)
// ----------------------------------------------------------------------------
#define ESPNOW_CHANNEL          1        // ช่อง SoftAP / ESP-NOW เริ่มต้น
#define SYNC_INTERVAL_MS        1000     // ส่ง Sync ให้ Station ทุก 1 วินาที
#define ONLINE_TIMEOUT_MS       8000     // ไม่ได้รับข้อมูลเกิน 8 วินาที = OFFLINE
#define LOST_LINK_GRACE_MS      30000    // [4.10.0-TOUCH] เพิ่ม: เงียบต่อจาก OFFLINE อีกเท่านี้จึงถือว่าขาดการติดต่อจริง
#define MAX_PLAUSIBLE_RATE_HR   3000.0f  // [4.10.0-TOUCH] เพิ่ม: ชุดให้สารน้ำมาตรฐานไหลได้ไม่ถึงเท่านี้
#define MAX_PLAUSIBLE_BATT_V    6.0f     // [4.10.0-TOUCH] เพิ่ม: แบตลิเธียมเซลล์เดียว ไม่มีทางถึง 6 โวลต์
                                         // เดิม 5 วินาที สั้นเกินไปเมื่อมีหลายเตียง
                                         // ทำให้เตียงหายไปชั่วครู่ทั้งที่เครื่องยังทำงาน
#define LINK_WINDOW_MS          10000UL  // หน้าต่างวัดคุณภาพลิงก์ (คาดหวัง 10 ใบ)
#define LINK_WEAK_PCT           60       // ต่ำกว่านี้ = ลิงก์อ่อน เตือนก่อนหลุดจริง
#define ID_CONFLICT_WINDOW_MS   30000UL  // ช่วงเวลาที่นับการสลับ MAC ของเลขเตียงเดียวกัน
#define ID_CONFLICT_MIN_FLIPS   4        // สลับเกินเท่านี้ใน 1 ช่วง = มีบอร์ดตั้งเลขซ้ำกันแน่
#define HEARD_REMEMBER_MS       60000UL  // จำไว้ว่าเคยได้ยินเลขเตียงนี้นานเท่าใด
#define NO_SIGNAL_MS            90000UL  // ติดต่อกันได้นานเท่านี้แต่ไม่เคยมีหยดเลย
#define DROP_FALL_MS            240      // เวลาที่หยดหนึ่งหยดใช้ตกในแอนิเมชัน
#define SYNC_SLOT_MIN_MS        60       // ช่องเวลาขั้นต่ำระหว่างการส่ง Sync สองใบ
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
#define ALERT_NO_SIGNAL   6   // เซนเซอร์ยังไม่จับหยดเลย (คนละเรื่องกับสายพับ)
#define ALERT_LOST_LINK   7   // [4.10.0-TOUCH] เพิ่ม: เตียงที่กำลังให้น้ำเกลือหายไปจากอากาศ (ใช้ฝั่ง Host เท่านั้น)

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
bool isPowerOffProgressActive = false;

// ---- ชนิดข้อมูลที่ใช้เป็นพารามิเตอร์ของฟังก์ชันวาดจอ ----
// ต้องประกาศไว้ตอนต้นไฟล์ เพราะ Arduino IDE แทรก prototype ของฟังก์ชันไว้ก่อนส่วนแสดงผล
enum BedUiStatus { BU_NORMAL = 0, BU_FAST, BU_SLOW, BU_NOFLOW, BU_NEAREND, BU_DONE, BU_PAUSED, BU_OFFLINE };
struct TouchZone { int x, y, w, h; };

// ---- สถานะของแอปบนจอสัมผัส (ไฟล์ .ino สร้าง prototype ให้เฉพาะฟังก์ชัน ตัวแปรจึงต้องประกาศไว้ก่อน) ----
enum UiScreen {
  SCR_HOME = 0,   // หน้าหลักแบบ FOCUS
  SCR_BED,        // รายละเอียดเตียง
  SCR_BED_SET,    // ตั้งค่าของเตียง
  SCR_NUMPAD,     // แป้นตัวเลข
  SCR_SYS,        // ตั้งค่าระบบ
  SCR_ALARM,      // จอเตือนเต็มจอ
  SCR_CALIB,      // ปรับความแม่นจอสัมผัส
  SCR_POWER       // หน้าจอปิดเครื่อง
};

UiScreen uiScreen       = SCR_HOME;
UiScreen uiScreenDrawn  = (UiScreen)-1;
int  uiSelectedBed      = 0;       // เตียงที่กำลังเปิดดู (0-based)
int  uiEditField        = 0;       // ช่องที่กำลังแก้ค่า
bool uiNeedFramework    = true;    // ต้องวาดกรอบใหม่ทั้งหน้า
uint8_t screenBrightness = 100;    // ความสว่างหน้าจอ 25-100 %
bool backlightDimmed    = false;

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
  bool wasMonitoring = false;   // [4.10.0-TOUCH] เพิ่ม: เคยเห็นเตียงนี้ออนไลน์และกำลังนับอยู่จริง
  bool lostLinkAck = false;     // [4.10.0-TOUCH] เพิ่ม: พยาบาลรับทราบว่าเตียงนี้ขาดการติดต่อแล้ว
  unsigned long nearNextChime = 0;

  // ---- คุณภาพลิงก์: ได้รับจริงกี่ % ของที่ควรได้ใน 10 วินาทีล่าสุด ----
  // ต่างจาก RSSI: RSSI บอกว่าสัญญาณแรงแค่ไหน ค่านี้บอกว่าข้อมูลหายจริงหรือเปล่า
  uint16_t rxWindowCount = 0;
  uint32_t rxTotal       = 0;
  uint8_t  linkPct       = 0;
  unsigned long firstRecvTime = 0;   // ใช้จับกรณีติดต่อได้แต่ไม่เคยมีหยดเลย
  unsigned long lastDropAtMs  = 0;   // ใช้ล็อกเฟสแอนิเมชันหยดให้ตรงกับหยดจริง

  // ---- ตรวจว่ามีสองบอร์ดตั้งเลขเตียงเดียวกันหรือไม่ ----
  uint8_t  srcMac[6]      = {0};
  bool     macKnown       = false;
  uint8_t  idFlipCount    = 0;
  unsigned long idFlipWindowMs = 0;
  bool     idConflict     = false;

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

// ---- ตัวจัดคิวส่ง Sync แบบไม่บล็อก ----
// ของเดิมส่งครบทุกเตียงในรอบเดียวโดยมี delay() คั่น ทำให้ loop() หยุดนานถึงหลักร้อย ms
// ระหว่างนั้น Host รับข้อมูลจาก Station ไม่ได้เลย เป็นต้นเหตุหนึ่งของอาการขึ้น OFFLINE
uint16_t pendingSyncMask    = 0;    // บิตละเตียง = ขอให้ส่งในช่องเวลาถัดไป
uint8_t  syncRoundIdx       = 0;    // เตียงถัดไปในรอบปกติ
unsigned long lastSyncSlotTime  = 0;
uint32_t syncSendFail       = 0;
unsigned long lastLinkWindowTime = 0;

// จำไว้ว่าเคยได้ยินเลขเตียงใดบ้าง แม้เลขนั้นจะเกินจำนวนเตียงที่เปิดใช้
unsigned long heardIdAt[MAX_SUPPORTED_STATIONS] = {0};
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
void drawHomeScreen(bool force);
void drawBedScreen(bool force);
void drawBedSetScreen(bool force);
void drawNumpadScreen(bool force);
void drawSysScreen(bool force);
void drawAlarmScreen(bool force);
void drawCalibScreen(bool force);
void updateHostDisplay();
void setDisplaySleep(bool sleep);
void setBacklightRaw(int pct);
void setScreenBrightness(int pct);
int  currentFocusBed();
void uiGoBack();
void serviceTouch();
void drawPowerOffProgress(int pct);
void drawGoodbyeScreen();
void drawSettingRow(int y, const char* label, const String &value, const char* unit, bool force);
void stepEditedValue(int field, int dir);
void openNumpad(int field);

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
         code == ALERT_NO_SIGNAL ||
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

// [4.10.0-TOUCH] เพิ่ม: เตียงที่กำลังให้น้ำเกลืออยู่ แล้วเงียบหายไปจากอากาศ
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


// ============================================================================
// นาฬิกาสำรอง DS3231 (I2C) และการบันทึกข้อมูลลง SD card (ช่องบนหลังจอ)
// ----------------------------------------------------------------------------
//  DS3231 : เก็บวันที่-เวลาไว้แม้ไฟดับ (มีถ่านกระดุมในตัว) ความคลาดเคลื่อนต่ำมาก
//           เวลาที่เก็บเป็น UTC ส่วนการแสดงผลแปลงเป็นเวลาไทยด้วย TZ ของระบบ
//           เมื่อเปิดหน้าเว็บจากมือถือ เวลาจากเครื่องนั้นจะถูกเขียนลง DS3231 ให้ด้วย
//  SD card: บันทึกข้อมูลรายนาทีของทุกเตียงเป็นไฟล์ CSV วันละไฟล์ (/IVLOG/YYYYMMDD_data.csv)
//           และบันทึกเหตุการณ์สำคัญไว้อีกไฟล์ (/IVLOG/YYYYMMDD_event.csv)
//           ดาวน์โหลดไฟล์ได้จากหน้าเว็บ (แท็บ Log & Shift Report)
// ============================================================================
RTC_DS3231 rtc;
bool rtcPresent  = false;     // ตรวจพบโมดูล DS3231
bool rtcTimeOk   = false;     // เวลาใน DS3231 ใช้งานได้ (ไม่ได้ไฟหมด)
bool sdPresent   = false;     // ใส่ SD card และ mount ได้
uint32_t sdRowsWritten = 0;   // จำนวนแถวข้อมูลที่เขียนลง SD แล้ว (นับตั้งแต่เปิดเครื่อง)
char sdDateStr[12] = "nodate";
unsigned long lastSdRetry  = 0;
unsigned long lastRtcCheck = 0;
uint8_t lastLoggedAlert[MAX_SUPPORTED_STATIONS];

// ---------------------------------------------------------------- DS3231
void initRtc() {
  Wire.begin(RTC_SDA_PIN, RTC_SCL_PIN);
  rtcPresent = rtc.begin(&Wire);
  if (!rtcPresent) {
    Serial.println("[RTC] DS3231 not found");
    return;
  }
  rtcTimeOk = !rtc.lostPower();
  if (!rtcTimeOk) {
    Serial.println("[RTC] DS3231 lost power - waiting for time from web");
    return;
  }
  DateTime now = rtc.now();
  uint32_t epoch = now.unixtime();
  if (epoch > 1600000000UL) {
    struct timeval tv = { (time_t)epoch, 0 };
    settimeofday(&tv, NULL);
    isTimeSynced = true;      // DS3231 เป็นแหล่งเวลาที่เชื่อถือได้
    isTimeApprox = false;
  } else {
    rtcTimeOk = false;
  }
}

// เขียนเวลาปัจจุบันของระบบลง DS3231 (เรียกเมื่อหน้าเว็บส่งเวลามาให้)
void syncRtcFromSystem() {
  if (!rtcPresent) return;
  time_t now;
  time(&now);
  if (now <= 1600000000) return;
  rtc.adjust(DateTime((uint32_t)now));
  rtcTimeOk = true;
}

// อ่านเวลาจาก DS3231 มาแก้ค่าเวลาของระบบเป็นระยะ (กันนาฬิกาภายในชิปเดินคลาด)
void serviceRtc(unsigned long nowMs) {
  if (!rtcPresent || !rtcTimeOk) return;
  if (nowMs - lastRtcCheck < 3600000UL) return;    // ทุก 1 ชั่วโมง
  lastRtcCheck = nowMs;
  DateTime t = rtc.now();
  uint32_t epoch = t.unixtime();
  if (epoch <= 1600000000UL) return;
  time_t sys;
  time(&sys);
  if (abs((long)(epoch - (uint32_t)sys)) >= 2) {   // ต่างกันเกิน 2 วินาทีจึงปรับ
    struct timeval tv = { (time_t)epoch, 0 };
    settimeofday(&tv, NULL);
  }
}

String rtcStatusText() {
  if (!rtcPresent) return String("not found");
  return rtcTimeOk ? String("OK") : String("set time on web");
}

// ---------------------------------------------------------------- SD card
void sdUpdateDateStr() {
  struct tm ti;
  if (!getLocalTime(&ti, 10)) {
    snprintf(sdDateStr, sizeof(sdDateStr), "nodate");
    return;
  }
  strftime(sdDateStr, sizeof(sdDateStr), "%Y%m%d", &ti);
}

String sdDataPath()  { return "/IVLOG/" + String(sdDateStr) + "_data.csv"; }
String sdEventPath() { return "/IVLOG/" + String(sdDateStr) + "_event.csv"; }

bool initSdCard() {
  sdPresent = SD.begin(SD_CS_PIN, SPI_TFT, SD_SPI_HZ);
  if (!sdPresent) {
    Serial.println("[SD] card not found");
    return false;
  }
  if (!SD.exists("/IVLOG")) SD.mkdir("/IVLOG");
  sdUpdateDateStr();
  Serial.printf("[SD] ready, logging to %s\n", sdDataPath().c_str());
  return true;
}

// บันทึกเหตุการณ์ (เริ่มถุงใหม่ แก้ค่า เกิดเหตุเตือน ฯลฯ) — bedIdx = -1 คือเหตุการณ์ของระบบ
void sdLogEvent(const char* type, int bedIdx, const String &detail) {
  if (!sdPresent) return;
  sdUpdateDateStr();
  String path = sdEventPath();
  bool isNew = !SD.exists(path);
  File f = SD.open(path, FILE_APPEND);
  if (!f) { sdPresent = false; return; }
  if (isNew) f.println(F("datetime,type,bed,detail"));
  f.print(getFormattedDateTime());
  f.print(',');
  f.print(type);
  f.print(',');
  if (bedIdx >= 0) f.print(bedIdx + 1); else f.print('-');
  f.print(',');
  f.println(detail);
  f.close();
}

// บันทึกข้อมูลรายนาทีของทุกเตียงลงไฟล์ CSV (เรียกจากรอบบันทึก log รายนาทีใน loop)
void sdLogMinute(const String &timestamp) {
  if (!sdPresent) return;
  sdUpdateDateStr();
  String path = sdDataPath();
  bool isNew = !SD.exists(path);
  File f = SD.open(path, FILE_APPEND);
  if (!f) {
    sdPresent = false;                 // ถอดการ์ดออกกลางคัน -> หยุดบันทึกและลองใหม่ภายหลัง
    return;
  }
  if (isNew) f.println(F("datetime,minute,bed,patient,drops,volume_ml,rate_mlhr,target_mlhr,alert,rssi,battery_v"));

  for (int i = 0; i < activeStationCount; i++) {
    const StationData &s = stations[i];
    if (s.logCount == 0) continue;
    const LogEntry &e = s.logs[s.logCount - 1];
    f.print(timestamp);            f.print(',');
    f.print(e.minuteIndex);        f.print(',');
    f.print(i + 1);                f.print(',');
    f.print(s.cfg.patientName);    f.print(',');
    f.print(e.drops);              f.print(',');
    f.print(e.volume, 1);          f.print(',');
    f.print(e.rateHr, 1);          f.print(',');
    f.print(e.targetRate, 0);      f.print(',');
    f.print(alertTextEn(e.alertCode)); f.print(',');
    f.print(e.rssi);               f.print(',');
    f.println(e.battery, 2);
    sdRowsWritten++;
  }
  f.close();
}

// ตรวจการ์ดซ้ำเป็นระยะ เผื่อเพิ่งเสียบเข้าไป หรือหลุดกลางทาง
void serviceSdCard(unsigned long nowMs) {
  if (sdPresent) return;
  if (nowMs - lastSdRetry < 30000UL) return;
  lastSdRetry = nowMs;
  SD.end();
  initSdCard();
}

String sdStatusText() {
  if (!sdPresent) return String("no card");
  return String(sdRowsWritten) + " rows";
}

// บันทึกเมื่อรหัสเตือนของเตียงเปลี่ยน (เรียกทุกวินาทีหลังประเมินการแจ้งเตือน)
void sdLogAlertChanges() {
  if (!sdPresent) return;
  for (int i = 0; i < activeStationCount; i++) {
    uint8_t code = stations[i].alertCode;
    if (code == lastLoggedAlert[i]) continue;
    lastLoggedAlert[i] = code;
    if (code == ALERT_NONE) sdLogEvent("ALERT_CLEAR", i, String("back to normal"));
    else                    sdLogEvent("ALERT", i, String(alertTextEn(code)));
  }
}

// ----------------------------------------------------------------------------
// การส่ง Sync แบบไม่บล็อก (พอร์ตมาจาก Host OLED v4.7.3)
// ----------------------------------------------------------------------------
// ของเดิมส่งครบทุกเตียงในรอบเดียว โดยมี delay(4) คั่นทุกใบและ delay(10) เมื่อส่งไม่ผ่าน
// แปดเตียงจึงทำให้ loop() หยุดนานได้ถึงราว 112 ms ระหว่างนั้น Host รับข้อมูลจาก
// Station ไม่ได้เลย เป็นต้นเหตุหนึ่งของอาการเตียงขึ้น OFFLINE ทั้งที่เครื่องยังทำงาน
//
// ของใหม่แบ่งเป็น "ช่องเวลา" ส่งได้มากสุดหนึ่งใบต่อช่อง จึงไม่มี delay() ในลูปเลย
uint16_t syncSlotIntervalMs() {
  uint8_t n = (activeStationCount < 1) ? 1 : activeStationCount;
  uint16_t slot = SYNC_INTERVAL_MS / n;
  if (slot < SYNC_SLOT_MIN_MS) slot = SYNC_SLOT_MIN_MS;
  return slot;
}

// รหัสที่ส่งออกไปให้ Station — Station รุ่นเก่าไม่รู้จัก ALERT_NO_SIGNAL
// จึงต้องแปลงเป็น ALERT_NONE เสมอ เพื่อให้เข้ากันได้กับทุกรุ่นตั้งแต่ 7.4.x ขึ้นไป
uint8_t alertCodeForStation(uint8_t code) {
  // [4.10.0-TOUCH] แก้: กันรหัส 7 ไม่ให้หลุดออกไป Station รุ่นเดิมไม่รู้จัก
  return (code == ALERT_NO_SIGNAL || code == ALERT_LOST_LINK) ? ALERT_NONE : code;
}

void sendSyncForStation(int i) {
  if (i < 0 || i >= MAX_SUPPORTED_STATIONS) return;

  time_t now;
  time(&now);

  struct_host_sync syncMsg = {};
  syncMsg.epochTime    = (now > 1600000000) ? (uint32_t)now : 0;
  syncMsg.isSynced     = (now > 1600000000) ? 1 : 0;
  syncMsg.stationId    = i + 1;
  syncMsg.targetRateHr = stations[i].cfg.targetRateHr;
  syncMsg.totalPlanMl  = stations[i].cfg.planVolumeMl;
  syncMsg.alertCode    = alertCodeForStation(stations[i].alertCode);
  syncMsg.dropFactor   = safeDropFactor(stations[i].cfg.dropFactor);
  syncMsg.resetSeq     = stations[i].cfg.resetSeq;
  syncMsg.hostChannel  = currentWifiChannel();
  syncMsg.nearEndPct   = safeNearPct(stations[i].nearEndPct);
  syncMsg.flags        = stations[i].nearEndAck ? 0x01 : 0x00;

  // ไม่ส่งซ้ำและไม่ delay() เมื่อคิวส่งเต็ม เพราะรอบถัดไปมาถึงในอีกไม่กี่สิบมิลลิวินาทีอยู่แล้ว
  if (esp_now_send(broadcastAddress, (uint8_t *)&syncMsg, sizeof(syncMsg)) != ESP_OK) syncSendFail++;
}

// ขอให้ส่ง Sync ของเตียงนี้ในช่องเวลาถัดไป (ใช้ตอนค่าตั้งเปลี่ยนหรือรหัสเตือนเปลี่ยน)
void requestSyncNow(int idx) {
  if (idx < 0 || idx >= MAX_SUPPORTED_STATIONS) return;
  pendingSyncMask |= (uint16_t)(1u << idx);
}

void requestSyncAll() {
  for (int i = 0; i < activeStationCount; i++) requestSyncNow(i);
}

// เรียกทุกรอบของ loop() — ส่งได้มากสุด 1 ใบต่อหนึ่งช่องเวลา จึงไม่กินเวลาในลูปเลย
void serviceSyncScheduler() {
  unsigned long now = millis();
  if (now - lastSyncSlotTime < syncSlotIntervalMs()) return;
  lastSyncSlotTime = now;

  if (pendingSyncMask != 0) {                    // งานด่วนมาก่อน
    for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
      if (pendingSyncMask & (1u << i)) {
        pendingSyncMask &= (uint16_t)~(1u << i);
        sendSyncForStation(i);
        return;
      }
    }
  }

  if (activeStationCount < 1) return;
  if (syncRoundIdx >= activeStationCount) syncRoundIdx = 0;
  sendSyncForStation(syncRoundIdx);
  syncRoundIdx = (uint8_t)((syncRoundIdx + 1) % activeStationCount);
}

// ปรับคุณภาพลิงก์ทุก 10 วินาที: ได้รับกี่ใบจาก 10 ใบที่ควรได้
void serviceLinkQuality() {
  unsigned long now = millis();
  if (now - lastLinkWindowTime < LINK_WINDOW_MS) return;
  lastLinkWindowTime = now;

  const uint16_t expected = (uint16_t)(LINK_WINDOW_MS / SYNC_INTERVAL_MS);
  portENTER_CRITICAL(&dataMux);
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) {
    uint16_t got = stations[i].rxWindowCount;
    stations[i].rxWindowCount = 0;
    uint16_t pct = (uint16_t)((got * 100UL) / expected);
    stations[i].linkPct = (uint8_t)((pct > 100) ? 100 : pct);
  }
  portEXIT_CRITICAL(&dataMux);
}

// ---- ตัวช่วยวินิจฉัยปัญหาการเชื่อมต่อ (พอร์ตมาจาก Host OLED v4.7.4) ----

// เตียงที่ออนไลน์และกำลังนับ แต่ไม่เคยมีหยดเลยตั้งแต่ติดต่อกันได้
// แยกจากสายพับ เพราะสายพับคือ "เคยไหลแล้วหยุด" ส่วนอันนี้คือ "ไม่เคยเริ่มเลย"
bool isStationNoSignal(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i) || !s.isRunning) return false;
  if (s.totalDrops > 0) return false;
  if (s.firstRecvTime == 0) return false;
  return (millis() - s.firstRecvTime) > NO_SIGNAL_MS;
}

bool hasIdConflict(int i) { return stations[i].idConflict; }

bool anyIdConflict() {
  for (int i = 0; i < activeStationCount; i++) if (stations[i].idConflict) return true;
  return false;
}

bool heardStationId(int i) {
  unsigned long t = heardIdAt[i];
  return t != 0 && (millis() - t) < HEARD_REMEMBER_MS;
}

// เลขเตียงที่ได้ยินอยู่แต่ยังไม่ได้เปิดใช้ (0 = ไม่มี)
uint8_t heardBeyondBedCount() {
  for (int i = activeStationCount; i < MAX_SUPPORTED_STATIONS; i++) {
    if (heardStationId(i)) return (uint8_t)(i + 1);
  }
  return 0;
}

String macTail(const StationData &s) {
  if (!s.macKnown) return String("-");
  char b[12];
  snprintf(b, sizeof(b), "%02X:%02X:%02X", s.srcMac[3], s.srcMac[4], s.srcMac[5]);
  return String(b);
}

// ---- ควบคุมความสว่างไฟหน้าจอด้วย PWM ----
void setBacklightRaw(int pct) {
#if TFT_BLK >= 0
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(TFT_BLK, (pct * 255) / 100);
  #else
    ledcWrite(0, (pct * 255) / 100);
  #endif
#else
  (void)pct;
#endif
}

void setScreenBrightness(int pct) {
  if (pct < 25) pct = 25;
  if (pct > 100) pct = 100;
  screenBrightness = (uint8_t)pct;
  backlightDimmed = false;
  setBacklightRaw(screenBrightness);
  preferences.begin("sys-config", false);
  preferences.putUChar("bright", screenBrightness);
  preferences.end();
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
  stopLoopWatchdog();   // [4.10.0-TOUCH] เพิ่ม: เลิกเฝ้าก่อนหลับ ไม่งั้นถูกนับว่าค้าง
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
    // [4.10.0-TOUCH] แก้: แยก "ติดต่อเตียงไม่ได้" ออกจาก "พยาบาลสั่งหยุด" ของเดิมรวมสองกรณีนี้
    // ไว้ด้วยกันแล้วล้างรหัสเตือนทิ้งทั้งคู่ เตียงที่กำลังให้น้ำเกลืออยู่แล้วหายไป
    // จากอากาศจึงไม่มีเสียงใด ๆ ทั้งที่ระบบเลิกเฝ้าเตียงนั้นไปแล้ว
    if (!isStationOnline(i)) {
      uint8_t lost = isStationLostLink(i) ? ALERT_LOST_LINK : ALERT_NONE;
      if (s.alertCode != lost) requestSyncNow(i);   // ยกเลิกเตือนที่ Station ทันที
      s.alertCode = lost;
      s.deviationSince = 0;
      if (lost != ALERT_NONE) anyCritical = true;   // รหัสนี้ตัดสินตรงนี้ เพราะข้ามส่วนท้ายไป
      continue;
    }

    // ติดต่อได้ตามปกติ = ล้างสถานะขาดการติดต่อทิ้ง พร้อมรับเหตุการณ์ครั้งใหม่
    s.lostLinkAck = false;
    s.wasMonitoring = s.isRunning;        // เฝ้าอยู่จริงเฉพาะตอนที่กำลังนับ

    if (!s.isRunning) {
      if (s.alertCode != ALERT_NONE) requestSyncNow(i);   // ยกเลิกเตือนที่ Station ทันที
      s.alertCode = ALERT_NONE;
      s.deviationSince = 0;
      continue;
    }

    float tr   = s.cfg.targetRateHr;
    float act  = s.flowRate_ml_hr;
    float vol  = s.totalVolumeMl;
    float plan = s.cfg.planVolumeMl;
    uint8_t code = ALERT_NONE;

    if (isStationNoSignal(i)) {
      code = ALERT_NO_SIGNAL;         // ตรวจก่อนสายพับ เพราะยังไม่เคยมีหยดให้พับเลย
      s.deviationSince = 0;
    } else if (isStationOccluded(i)) {
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

    // รหัสเตือนเปลี่ยน = ส่งให้ Station ในช่องเวลาถัดไป แทนที่จะรอครบรอบ
    if (s.alertCode != code) requestSyncNow(i);
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

// ============================================================================
// ปุ่ม multifunction ปุ่มเดียว (ขา POWER_BTN_PIN)
//   คลิก 1 ครั้ง : มีเสียงเตือน = พักเสียง / มีเตือนใกล้หมด = รับทราบทุกเตียง
//                  จอดับอยู่ = ปลุกจอ / อยู่หน้าย่อย = ย้อนกลับ
//   ดับเบิลคลิก  : ปิด/เปิดหน้าจอ (ระบบยังทำงานและยังเตือนตามปกติ)
//   กดค้าง 2 วิ  : ปิดเครื่อง (ปลุกด้วยการกดค้าง 2 วินาทีอีกครั้ง)
// ============================================================================
void handleShortClick() {
  lastUserActivityTime = millis();

  if (globalAlarmTriggered && !isSnoozed()) {
    globalSnoozeUntil = millis() + SNOOZE_MS;
    noTone(BUZZER_PIN);
    tone(BUZZER_PIN, 2200, 40);
    if (displaySleeping) setDisplaySleep(false);
    uiScreen = SCR_HOME;
    uiNeedFramework = true;
    return;
  }
  if (anyNearEndPending()) {
    acknowledgeAllNearEnd();
    tone(BUZZER_PIN, 2600, 40);
    if (displaySleeping) setDisplaySleep(false);
    uiNeedFramework = true;
    return;
  }
  if (displaySleeping) {
    setDisplaySleep(false);
    return;
  }
  if (uiScreen != SCR_HOME) {
    uiGoBack();
    tone(BUZZER_PIN, 2000, 15);
    return;
  }
  tone(BUZZER_PIN, 1600, 15);          // อยู่หน้าหลักอยู่แล้ว
}

void checkMultifunctionButton() {
  static unsigned long btnPressStart = 0;
  static unsigned long lastProgressFrameTime = 0;
  static unsigned long lastReleaseTime = 0;
  static bool isHolding = false;
  static int clickCount = 0;

  unsigned long now = millis();
  int btnState = digitalRead(POWER_BTN_PIN);

  if (btnState == LOW) {
    if (!isHolding) {
      btnPressStart = now;
      isHolding = true;
    }
    unsigned long holdDuration = now - btnPressStart;

    if (holdDuration >= 450 && !displaySleeping) {
      isPowerOffProgressActive = true;
      if (now - lastProgressFrameTime >= 60) {
        lastProgressFrameTime = now;
        drawPowerOffProgress(map(holdDuration, 450, 2000, 0, 100));
      }
    }
    if (holdDuration >= 2000) powerOffSystem();
  }
  else if (isHolding) {
    unsigned long totalHoldTime = now - btnPressStart;
    isHolding = false;
    btnPressStart = 0;

    if (isPowerOffProgressActive) {          // กดค้างแล้วปล่อยก่อนครบ = ยกเลิก
      isPowerOffProgressActive = false;
      uiScreenDrawn = (UiScreen)-1;
      uiNeedFramework = true;
      updateHostDisplay();
    } else if (totalHoldTime < 400) {
      clickCount++;                          // รอดูว่าจะมีคลิกที่สองหรือไม่
      lastReleaseTime = now;
    }
  }

  // ตัดสินว่าเป็นคลิกเดียวหรือดับเบิลคลิก เมื่อไม่มีการกดเพิ่มภายใน 280 ms
  if (clickCount > 0 && !isHolding && (now - lastReleaseTime > 280)) {
    if (clickCount >= 2) {
      lastUserActivityTime = now;
      setDisplaySleep(!displaySleeping);     // ดับเบิลคลิก = ปิด/เปิดจอ
      tone(BUZZER_PIN, 1800, 25);
    } else {
      handleShortClick();
    }
    clickCount = 0;
    if (!displaySleeping) updateHostDisplay();
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

  // [4.10.0-TOUCH] เพิ่ม: ปฏิเสธค่าที่เป็นไปไม่ได้ทางกายภาพ
  // แพ็กเก็ต ESP-NOW เป็นการกระจายเปล่า ไม่มีลายเซ็นและไม่มีเลขตรวจสอบ การตรวจเดิม
  // มีแค่ความยาวกับช่วงของเลขเตียง อุปกรณ์อื่นที่บังเอิญส่งขนาด 23 ไบต์ในช่องเดียวกัน
  // จึงถูกตีความเป็นข้อมูลเตียงได้ ทำให้ตัวเลขบนจอกระโดดและอาจปลุกเสียงเตือนผิด
  // เขียนกลับด้านเพื่อให้ดัก NaN ไปด้วยในตัว (การเทียบใด ๆ กับ NaN เป็นเท็จเสมอ)
  if (incoming.isRunning > 1) return;
  if (!(incoming.flowRateHr   >= 0.0f && incoming.flowRateHr   <= MAX_PLAUSIBLE_RATE_HR)) return;
  if (!(incoming.batteryVolts >= 0.0f && incoming.batteryVolts <= MAX_PLAUSIBLE_BATT_V)) return;

  int idx = incoming.stationId - 1;

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  const uint8_t *src = (info) ? info->src_addr : nullptr;
#else
  const uint8_t *src = mac;
#endif

  portENTER_CRITICAL(&dataMux);
  StationData &s = stations[idx];

  heardIdAt[idx] = millis();

  // ---- ตรวจว่ามีสองบอร์ดตั้งเลขเตียงเดียวกันหรือไม่ ----
  // Host แยกเตียงจากเลขที่เครื่องส่งมาเท่านั้น สองเครื่องที่ตั้งเลขซ้ำกันจะเขียนทับ
  // ช่องเดียวกัน เตียงที่เหลือจึงขึ้น OFFLINE ทั้งที่เครื่องทำงานปกติ
  // จับได้จากการที่ MAC ต้นทางของเลขเตียงเดียวกันสลับไปมาถี่ผิดปกติ
  if (src != nullptr) {
    if (!s.macKnown) {
      memcpy(s.srcMac, src, 6);
      s.macKnown = true;
      s.idFlipWindowMs = heardIdAt[idx];
    } else if (memcmp(s.srcMac, src, 6) != 0) {
      memcpy(s.srcMac, src, 6);                 // จำตัวล่าสุดไว้
      if (heardIdAt[idx] - s.idFlipWindowMs > ID_CONFLICT_WINDOW_MS) {
        s.idFlipWindowMs = heardIdAt[idx];      // เริ่มนับช่วงใหม่
        s.idFlipCount = 0;
      }
      if (s.idFlipCount < 255) s.idFlipCount++;
      if (s.idFlipCount >= ID_CONFLICT_MIN_FLIPS) s.idConflict = true;
    } else if (heardIdAt[idx] - s.idFlipWindowMs > ID_CONFLICT_WINDOW_MS) {
      s.idFlipWindowMs = heardIdAt[idx];        // เงียบมานาน = ล้างสถานะเดิม
      s.idFlipCount = 0;
      s.idConflict = false;
    }
  }

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
  if (s.rxWindowCount < 0xFFFF) s.rxWindowCount++;
  s.rxTotal++;

  // ---- ล็อกเฟสแอนิเมชันหยด ----
  // Station บอกมาว่า "หยดล่าสุดผ่านมาแล้วกี่มิลลิวินาที" จึงย้อนกลับไปหาเวลาที่หยดนั้นตก
  // ในฐานเวลาของ Host ได้ แอนิเมชันบนจอจึงเดินตรงจังหวะกับหยดจริงที่เตียงนั้น
  if (incoming.msSinceLastDrop > 0 && incoming.msSinceLastDrop < 600000UL) {
    s.lastDropAtMs = s.lastRecvTime - incoming.msSinceLastDrop;
  }

  if (s.firstRecvTime == 0) s.firstRecvTime = s.lastRecvTime;
  // ตราบใดที่ยังมีหยดเข้ามา ให้เลื่อนเวลาอ้างอิงตาม เมื่อกด "เริ่มถุงใหม่" แล้วตัวนับกลับเป็น 0
  // ตัวจับเวลา 90 วินาทีจึงเริ่มนับจากจังหวะรีเซ็ต ไม่ใช่จากตอนเปิดเครื่อง
  if (s.totalDrops > 0) s.firstRecvTime = s.lastRecvTime;
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
// ส่วนแสดงผลและระบบสัมผัส — จอ 2.4" (ST7789V/ILI9341 240x320 แนวตั้ง) + ทัช XPT2046
// ใช้งานเหมือนสมาร์ตโฟน: แตะการ์ดเพื่อเข้าดู แตะปุ่มเพื่อสั่งงาน แก้ค่าได้บนจอโดยตรง
//
//   HOME     : เตียงที่ต้องดูตอนนี้ตัวใหญ่ (FOCUS) + รายการเตียงอื่นแบบแตะได้
//   BED      : รายละเอียดเตียง + ปุ่มสั่งงาน (ถุงใหม่ / รับทราบ / หยุดพัก / ตั้งค่า)
//   BED SET  : ตั้งค่าของเตียงด้วยปุ่ม - + หรือแตะที่ค่าเพื่อพิมพ์ตัวเลข
//   NUMPAD   : แป้นตัวเลขบนจอ
//   SYSTEM   : ตั้งค่าระบบ (จำนวนเตียง ความสว่าง ปรับจอสัมผัส ข้อมูล Wi-Fi)
//   ALARM    : จอเตือนเต็มจอ แตะที่ไหนก็ได้เพื่อพักเสียง
//   CALIB    : ปรับความแม่นจอสัมผัส (เก็บค่าไว้ใน NVS)
// ============================================================================

// ---- สีที่ใช้บนจอ (RGB565) ----
#define C_BG        0x0000
#define C_CARD      0x18C5
#define C_CARD2     0x10A2
#define C_BAR       0x2104
#define C_TOPBAR    0x0A49
#define C_BTN       0x2945
#define C_LINE      0x39E7
#define C_DIM       0x8410
#define C_WHITE     0xFFFF
#define C_GREEN     0x2FEB
#define C_CYAN      0x07FF
#define C_YELLOW    0xFFE0
#define C_ORANGE    0xFD20
#define C_RED       0xF800
#define C_SKY       0x5DFF

// ---- ผังหน้าจอแนวตั้ง 240x320 ----
#define SCR_W       240
#define SCR_H       320
#define TOPBAR_H    22

// ค่าที่กำลังแก้ชั่วคราว (ยังไม่บันทึกจนกว่าจะกด SAVE)
float editTargetRate = 0, editPlanVolume = 0;
uint8_t editDropFactor = 20, editNearPct = DEFAULT_NEAR_END_PCT;
char editNumBuf[8] = "";           // ตัวเลขที่พิมพ์ในหน้า NUMPAD
int  editNumMax = 999;
const char* editNumLabel = "";
const char* editNumUnit  = "";

// ---- แคชข้อความ เพื่อวาดเฉพาะส่วนที่เปลี่ยน ----
String cacheClock = "", cacheTopRight = "", cacheFocusKey = "", cacheStatusWord = "";
String cacheRate = "", cacheSetRate = "", cacheSummary = "", cacheBottom = "";
String cacheSide[MAX_SUPPORTED_STATIONS];
String cacheBedKey = "", cacheAlarmRate = "", cacheNumValue = "";
int    cachePct = -999;

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

void textRight(const String &s, int xr, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, xr - (int)s.length() * 6 * size, y, size, col, bg);
}
void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, x + (w - (int)s.length() * 6 * size) / 2, y, size, col, bg);
}
String padTo(const String &s, unsigned int width) {
  String out = s;
  while (out.length() < width) out += ' ';
  return out;
}

// ----------------------------------------------------------------------------
// แอนิเมชันหยดน้ำเกลือ (พอร์ตจาก Host OLED v4.7.5)
// ----------------------------------------------------------------------------
// ไม่ได้ "เดา" จังหวะหยดเอง แต่ล็อกเฟสกับหยดจริงที่ Station วัดได้
//   Station ส่ง msSinceLastDrop มาทุกวินาที -> ย้อนกลับไปได้ว่าหยดล่าสุดตกเมื่อใด
//   คาบการหยดคำนวณจากอัตราไหลจริง: 3600000 / (mL/h x หยดต่อ mL)
// หยดที่เห็นบนจอจึงตรงจังหวะกับหยดที่ตกจริงในกระเปาะของเตียงนั้น
uint32_t dripPeriodMs(int i) {
  const StationData &s = stations[i];
  float dropsPerHr = s.flowRate_ml_hr * (float)safeDropFactor(s.cfg.dropFactor);
  if (dropsPerHr < 1.0f) return 0;
  uint32_t p = (uint32_t)(3600000.0f / dropsPerHr);
  if (p < 150) p = 150;                 // เร็วกว่านี้ตาคนก็แยกไม่ออกอยู่ดี
  // เพดานผูกกับเกณฑ์สายพับ เลยจากนี้เตียงถูกตีเป็นเหตุเตือนอยู่แล้ว
  // เพดานที่สั้นเกินไปจะทำให้ % period ไปวาด "หยดปลอม" คั่นระหว่างหยดจริง
  if (p > MAX_OCCLUSION_MS) p = MAX_OCCLUSION_MS;
  return p;
}

// เฟสของแอนิเมชัน: 0..100 = กำลังตก (% ของระยะทาง), -1 = ยังไม่ถึงหยดถัดไป
int dripAnimPhase(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i) || !s.isRunning) return -1;
  if (s.alertCode == ALERT_OCCLUSION || s.alertCode == ALERT_NO_SIGNAL) return -1;
  uint32_t period = dripPeriodMs(i);
  if (period == 0 || s.lastDropAtMs == 0) return -1;

  uint32_t fall = DROP_FALL_MS;
  if (fall > period * 3 / 4) fall = period * 3 / 4;   // หยดถี่ = ตกเร็วขึ้น ไม่ให้ซ้อนกัน
  if (fall < 40) fall = 40;

  // เลยเวลาหยดถัดไปไปมากแล้ว = การไหลเปลี่ยนหรือหยุด แต่ยังไม่ถึงเกณฑ์สายพับ
  // ไม่วาดหยดปลอมต่อ รอให้หยดจริงส่ง msSinceLastDrop มา resync เฟสก่อน
  uint32_t elapsed = (uint32_t)(millis() - s.lastDropAtMs);
  if (elapsed > period * 3) return -1;
  uint32_t phase = elapsed % period;
  if (phase < fall) return (int)((phase * 100UL) / fall);
  return -1;
}





// ---- ปุ่มบนจอ (ความสูงอย่างน้อย 34 px เพื่อให้แตะง่าย) — struct TouchZone ประกาศไว้ตอนต้นไฟล์ ----
bool zoneHit(const TouchZone &z, int tx, int ty) {
  return tx >= z.x && tx < z.x + z.w && ty >= z.y && ty < z.y + z.h;
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
  if (s.alertCode == ALERT_OCCLUSION) return -1;
  float left = s.cfg.planVolumeMl - s.totalVolumeMl;
  if (left < 0) left = 0;
  float rate = (s.flowRate_ml_hr > 5.0f) ? s.flowRate_ml_hr : s.cfg.targetRateHr;
  if (rate <= 0) return -1;
  return (int)((left / rate) * 60.0f);
}

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

// ============================================================================
// ระบบจอสัมผัส XPT2046 — อ่านค่าดิบ แปลงเป็นพิกัดจอ และปรับความแม่น (Calibration)
// ============================================================================
XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);

// ค่าปรับความแม่น (ค่าดิบที่มุมจอ) — ค่าเริ่มต้นใช้ได้กับโมดูลส่วนใหญ่ ปรับใหม่ได้จากหน้าตั้งค่า
int touchRawX0 = 300, touchRawX1 = 3800, touchRawY0 = 300, touchRawY1 = 3800;
bool touchCalibrated = false;

unsigned long lastTouchEventMs = 0;
bool touchWasDown = false;

void loadTouchCalibration() {
  preferences.begin("sys-config", true);
  touchRawX0 = preferences.getInt("tx0", 300);
  touchRawX1 = preferences.getInt("tx1", 3800);
  touchRawY0 = preferences.getInt("ty0", 300);
  touchRawY1 = preferences.getInt("ty1", 3800);
  touchCalibrated = preferences.getUChar("tcal", 0) == 1;
  preferences.end();
}

void saveTouchCalibration() {
  preferences.begin("sys-config", false);
  preferences.putInt("tx0", touchRawX0);
  preferences.putInt("tx1", touchRawX1);
  preferences.putInt("ty0", touchRawY0);
  preferences.putInt("ty1", touchRawY1);
  preferences.putUChar("tcal", 1);
  preferences.end();
  touchCalibrated = true;
}

// แปลงค่าดิบของ XPT2046 เป็นพิกัดบนจอ (คิดการหมุนจอด้วย)
void mapTouch(int rawX, int rawY, int &sx, int &sy) {
  long x = map(rawX, touchRawX0, touchRawX1, 0, SCR_W - 1);
  long y = map(rawY, touchRawY0, touchRawY1, 0, SCR_H - 1);
  if (x < 0) x = 0;
  if (x > SCR_W - 1) x = SCR_W - 1;
  if (y < 0) y = 0;
  if (y > SCR_H - 1) y = SCR_H - 1;
#if TOUCH_SWAP_XY
  long t = x; x = y; y = t;
#endif
#if TOUCH_FLIP_X
  x = SCR_W - 1 - x;
#endif
#if TOUCH_FLIP_Y
  y = SCR_H - 1 - y;
#endif
  sx = (int)x;
  sy = (int)y;
}

// คืนค่า true หนึ่งครั้งต่อการแตะหนึ่งครั้ง (กันการแตะรัวและการสั่นของจอความต้านทาน)
bool readTap(int &tx, int &ty) {
  bool down = ts.touched();
  if (!down) {
    touchWasDown = false;
    return false;
  }
  if (touchWasDown) return false;                       // ยังกดค้างอยู่จากครั้งก่อน
  unsigned long now = millis();
  if (now - lastTouchEventMs < 180) return false;       // กันแตะซ้ำเร็วเกินไป

  TS_Point p = ts.getPoint();
  if (p.z < TOUCH_MIN_PRESSURE) return false;           // แรงกดน้อยเกินไป ถือว่าไม่ได้ตั้งใจ
  touchWasDown = true;
  lastTouchEventMs = now;
  mapTouch(p.x, p.y, tx, ty);
  lastUserActivityTime = now;
  return true;
}


// แถบหัวเรื่องของหน้าย่อย: ปุ่มย้อนกลับ (ซ้าย) + ชื่อหน้า
const TouchZone ZONE_BACK = { 4, 25, 44, 30 };


// ============================================================================
// หน้า HOME — เตียงที่ต้องดูตอนนี้ตัวใหญ่ + รายการเตียงอื่น (แตะได้ทุกส่วน)
// ============================================================================
#define FOCUS_X   6
#define FOCUS_Y   26
#define FOCUS_W   228
#define FOCUS_H   122
#define LIST_Y    154
#define LIST_H    128
#define BOTBAR_Y  288

const TouchZone ZONE_FOCUS = { FOCUS_X, FOCUS_Y, FOCUS_W, FOCUS_H };
const TouchZone ZONE_GEAR  = { 190, 290, 44, 28 };
const TouchZone ZONE_ALERT = { 6, 290, 180, 28 };

int homeRowH = 30;          // ความสูงแถวรายการเตียง (ปรับตามจำนวนเตียง)
int homeRowBed[MAX_SUPPORTED_STATIONS];   // เตียงที่อยู่ในแต่ละแถว (ใช้ตอนแตะ)
int homeRowCount = 0;






// จัดการการแตะบนหน้า HOME
void handleHomeTap(int tx, int ty) {
  if (zoneHit(ZONE_GEAR, tx, ty)) {
    uiScreen = SCR_SYS;
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_FOCUS, tx, ty)) {
    uiSelectedBed = currentFocusBed();
    uiScreen = SCR_BED;
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_ALERT, tx, ty)) {
    int a = autoFocusBed();
    if (a >= 0) {
      uiSelectedBed = a;
      uiScreen = SCR_BED;
      uiNeedFramework = true;
    }
    return;
  }
  if (ty >= LIST_Y && ty < LIST_Y + LIST_H) {
    int gap = 4;
    int slot = (ty - LIST_Y) / (homeRowH + gap);
    if (slot >= 0 && slot < homeRowCount) {
      uiSelectedBed = homeRowBed[slot];
      uiScreen = SCR_BED;
      uiNeedFramework = true;
    }
  }
}

// ============================================================================
// หน้า BED — รายละเอียดเตียงและปุ่มสั่งงาน
// ============================================================================
const TouchZone ZONE_BED_NEWBAG = {   6, 244,  74, 46 };
const TouchZone ZONE_BED_ACK    = {  84, 244,  72, 46 };
const TouchZone ZONE_BED_SET    = { 160, 244,  74, 46 };


void handleBedTap(int tx, int ty) {
  if (zoneHit(ZONE_BACK, tx, ty)) {
    uiScreen = SCR_HOME;
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_BED_NEWBAG, tx, ty)) {
    // เริ่มถุงใหม่: เปลี่ยน resetSeq ให้เครื่องประจำเตียงล้างตัวนับ (เหมือนปุ่มบนหน้าเว็บ)
    StationData &s = stations[uiSelectedBed];
    s.cfg.resetSeq++;
    s.totalDrops = 0;
    s.lastTotalDrops = 0;
    s.hasBaseline = false;
    s.totalVolumeMl = 0;
    s.nearEndAck = false;
    s.alertCode = ALERT_NONE;
    saveBedConfig(uiSelectedBed);
    sdLogEvent("NEW_BAG", uiSelectedBed, String("counter reset from screen"));
    tone(BUZZER_PIN, 2400, 60);
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_BED_ACK, tx, ty)) {
    stations[uiSelectedBed].nearEndAck = true;
    stations[uiSelectedBed].nearNextChime = 0;
    sdLogEvent("ACK_NEAR_END", uiSelectedBed, String("acknowledged on screen"));
    tone(BUZZER_PIN, 2600, 40);
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_BED_SET, tx, ty)) {
    const BedConfig &c = stations[uiSelectedBed].cfg;
    editTargetRate  = c.targetRateHr;
    editPlanVolume  = c.planVolumeMl;
    editDropFactor  = safeDropFactor(c.dropFactor);
    editNearPct     = safeNearPct(stations[uiSelectedBed].nearEndPct);
    uiScreen = SCR_BED_SET;
    uiNeedFramework = true;
  }
}

// ============================================================================
// หน้า BED SET — ปรับค่าด้วยปุ่ม - + หรือแตะที่ค่าเพื่อพิมพ์ตัวเลข
// ============================================================================
const int SET_ROW_Y[4] = { 62, 112, 162, 212 };
const TouchZone ZONE_SET_SAVE   = {   6, 266, 111, 46 };
const TouchZone ZONE_SET_CANCEL = { 123, 266, 111, 46 };



// เปิดแป้นตัวเลขสำหรับช่องที่เลือก
void openNumpad(int field) {
  uiEditField = field;
  editNumBuf[0] = '\0';
  switch (field) {
    case 0: editNumLabel = "TARGET RATE"; editNumUnit = "mL/h";   editNumMax = 999; break;
    case 1: editNumLabel = "PLAN VOLUME"; editNumUnit = "mL";     editNumMax = 9999; break;
    case 2: editNumLabel = "DROP FACTOR"; editNumUnit = "gtt/mL"; editNumMax = 60;  break;
    case 3: editNumLabel = "NEXT BAG AT"; editNumUnit = "%";      editNumMax = 95;  break;
    default: editNumLabel = "ACTIVE BEDS"; editNumUnit = "beds";  editNumMax = MAX_SUPPORTED_STATIONS; break;
  }
  uiScreen = SCR_NUMPAD;
  uiNeedFramework = true;
}

void applyEditedValue(int field, int v) {
  switch (field) {
    case 0: editTargetRate = v; break;
    case 1: editPlanVolume = v; break;
    case 2: editDropFactor = safeDropFactor(v); break;
    case 3: editNearPct = safeNearPct(v); break;
    case 4:
      if (v >= 1 && v <= MAX_SUPPORTED_STATIONS) {
        activeStationCount = v;
        preferences.begin("sys-config", false);
        preferences.putUChar("bedCount", activeStationCount);
        preferences.end();
        if (manualFocusBed >= activeStationCount) manualFocusBed = -1;
      }
      break;
  }
}

void stepEditedValue(int field, int dir) {
  switch (field) {
    case 0: editTargetRate += dir * 5;  if (editTargetRate < 0) editTargetRate = 0; if (editTargetRate > 999) editTargetRate = 999; break;
    case 1: editPlanVolume += dir * 50; if (editPlanVolume < 0) editPlanVolume = 0; if (editPlanVolume > 9999) editPlanVolume = 9999; break;
    case 2: {
      const uint8_t df[4] = {10, 15, 20, 60};
      int idx = 2;
      for (int k = 0; k < 4; k++) if (df[k] == editDropFactor) idx = k;
      idx += dir;
      if (idx < 0) idx = 0;
      if (idx > 3) idx = 3;
      editDropFactor = df[idx];
      break;
    }
    case 3: {
      int v = editNearPct + dir * 5;
      if (v < 50) v = 50;
      if (v > 95) v = 95;
      editNearPct = v;
      break;
    }
    case 4: {
      int v = activeStationCount + dir;
      if (v >= 1 && v <= MAX_SUPPORTED_STATIONS) applyEditedValue(4, v);
      break;
    }
  }
}

void handleBedSetTap(int tx, int ty) {
  if (zoneHit(ZONE_BACK, tx, ty) || zoneHit(ZONE_SET_CANCEL, tx, ty)) {
    uiScreen = SCR_BED;
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_SET_SAVE, tx, ty)) {
    StationData &s = stations[uiSelectedBed];
    s.cfg.targetRateHr = editTargetRate;
    s.cfg.planVolumeMl = editPlanVolume;
    s.cfg.dropFactor   = editDropFactor;
    s.nearEndPct       = editNearPct;
    s.nearEndAck       = false;              // เปลี่ยนแผนแล้ว เริ่มนับเตือนใกล้หมดใหม่
    saveBedConfig(uiSelectedBed);
    {
      char d[64];
      snprintf(d, sizeof(d), "target %d mL/h  plan %d mL  df %d  near %d%%",
               (int)editTargetRate, (int)editPlanVolume, (int)editDropFactor, (int)editNearPct);
      sdLogEvent("BED_CONFIG", uiSelectedBed, String(d));
    }
    tone(BUZZER_PIN, 2600, 80);
    uiScreen = SCR_BED;
    uiNeedFramework = true;
    return;
  }
  for (int f = 0; f < 4; f++) {
    int y = SET_ROW_Y[f];
    if (ty < y || ty >= y + 44) continue;
    if (tx >= 140 && tx < 184)      stepEditedValue(f, -1);   // ปุ่ม -
    else if (tx >= 188 && tx < 230) stepEditedValue(f, +1);   // ปุ่ม +
    else if (tx < 136)              openNumpad(f);            // แตะที่ค่า = พิมพ์เอง
    return;
  }
}


void handleNumpadTap(int tx, int ty) {
  if (zoneHit(ZONE_BACK, tx, ty)) {
    uiScreen = (uiEditField == 4) ? SCR_SYS : SCR_BED_SET;
    uiNeedFramework = true;
    return;
  }
  for (int i = 0; i < 12; i++) {
    int col = i % 3, row = i / 3;
    TouchZone z = { 6 + col * 78, 116 + row * 50, 72, 44 };
    if (!zoneHit(z, tx, ty)) continue;
    if (i == 9) {                                   // ปุ่ม C = ล้าง
      editNumBuf[0] = '\0';
    } else if (i == 11) {                           // ปุ่ม OK = ยืนยัน
      if (editNumBuf[0] != '\0') {
        int v = atoi(editNumBuf);
        if (v > editNumMax) v = editNumMax;
        applyEditedValue(uiEditField, v);
      }
      uiScreen = (uiEditField == 4) ? SCR_SYS : SCR_BED_SET;
      uiNeedFramework = true;
      tone(BUZZER_PIN, 2600, 40);
      return;
    } else {
      size_t len = strlen(editNumBuf);
      if (len < sizeof(editNumBuf) - 1) {
        editNumBuf[len] = (i == 10) ? '0' : (char)('1' + i);
        editNumBuf[len + 1] = '\0';
      }
    }
    tone(BUZZER_PIN, 3000, 10);
    return;
  }
}

// ============================================================================
// หน้า SYSTEM — ตั้งค่าระบบ (ทุกค่ายังแก้จากหน้าเว็บได้เหมือนเดิม)
// ============================================================================
const TouchZone ZONE_SYS_CALIB = { 6, 162, 228, 40 };


void handleSysTap(int tx, int ty) {
  if (zoneHit(ZONE_BACK, tx, ty)) {
    uiScreen = SCR_HOME;
    uiNeedFramework = true;
    return;
  }
  if (zoneHit(ZONE_SYS_CALIB, tx, ty)) {
    uiScreen = SCR_CALIB;
    uiNeedFramework = true;
    return;
  }
  if (ty >= 62 && ty < 106) {                        // แถวจำนวนเตียง
    if (tx >= 140 && tx < 184)      stepEditedValue(4, -1);
    else if (tx >= 188 && tx < 230) stepEditedValue(4, +1);
    else if (tx < 136)              openNumpad(4);
    return;
  }
  if (ty >= 112 && ty < 156) {                       // แถวความสว่างหน้าจอ
    if (tx >= 140 && tx < 184)      setScreenBrightness(screenBrightness - 25);
    else if (tx >= 188 && tx < 230) setScreenBrightness(screenBrightness + 25);
    return;
  }
}


void handleAlarmTap(int tx, int ty) {
  (void)tx; (void)ty;                       // แตะที่ไหนก็ได้ = พักเสียง
  globalSnoozeUntil = millis() + SNOOZE_MS;
  noTone(BUZZER_PIN);
  tone(BUZZER_PIN, 2200, 40);
  uiScreen = SCR_HOME;
  uiNeedFramework = true;
}

// ============================================================================
// หน้า CALIB — ปรับความแม่นของจอสัมผัส (แตะกากบาท 2 จุด)
// ============================================================================
int calibStep = 0;
int calibRawX[2], calibRawY[2];
const int CALIB_PT[2][2] = { { 24, 40 }, { SCR_W - 24, SCR_H - 40 } };



// หน้านี้อ่านค่าดิบเอง ไม่ผ่าน mapTouch เพราะกำลังหาค่าปรับอยู่
void serviceCalibScreen() {
  if (!ts.touched()) { touchWasDown = false; return; }
  if (touchWasDown) return;
  TS_Point p = ts.getPoint();
  if (p.z < TOUCH_MIN_PRESSURE) return;
  touchWasDown = true;
  lastUserActivityTime = millis();

  calibRawX[calibStep] = p.x;
  calibRawY[calibStep] = p.y;
  tone(BUZZER_PIN, 2600, 40);
  calibStep++;

  if (calibStep < 2) {
    uiNeedFramework = true;
    return;
  }

  // คำนวณค่าดิบที่ขอบจอจากจุดที่แตะสองจุด (เชิงเส้น)
  int dxScreen = CALIB_PT[1][0] - CALIB_PT[0][0];
  int dyScreen = CALIB_PT[1][1] - CALIB_PT[0][1];
  if (dxScreen != 0 && dyScreen != 0) {
    long slopeX = ((long)(calibRawX[1] - calibRawX[0]) * 1000) / dxScreen;
    long slopeY = ((long)(calibRawY[1] - calibRawY[0]) * 1000) / dyScreen;
    touchRawX0 = calibRawX[0] - (int)((slopeX * CALIB_PT[0][0]) / 1000);
    touchRawX1 = touchRawX0 + (int)((slopeX * (SCR_W - 1)) / 1000);
    touchRawY0 = calibRawY[0] - (int)((slopeY * CALIB_PT[0][1]) / 1000);
    touchRawY1 = touchRawY0 + (int)((slopeY * (SCR_H - 1)) / 1000);
    saveTouchCalibration();
  }

  calibStep = 0;
  tft.fillScreen(C_BG);
  textCenterIn("CALIBRATION SAVED", 0, SCR_W, 140, 2, C_GREEN, C_BG);
  delay(900);
  uiScreen = SCR_SYS;
  uiNeedFramework = true;
}

// ============================================================================
// ตัวควบคุมการแสดงผลและการแตะทั้งหมด
// ============================================================================
void resetUiCaches() {
  cacheClock = ""; cacheTopRight = ""; cacheFocusKey = ""; cacheStatusWord = "";
  cacheRate = ""; cacheSetRate = ""; cacheSummary = ""; cacheBottom = "";
  cacheBedKey = ""; cacheAlarmRate = ""; cacheNumValue = "";
  cachePct = -999;
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) cacheSide[i] = "";
}

void updateHostDisplay() {
  if (displaySleeping || isPowerOffProgressActive) return;

  // เหตุวิกฤตที่ยังไม่พักเสียง -> บังคับขึ้นจอเตือนเต็มจอ (ยกเว้นระหว่างปรับจอสัมผัส)
  bool alarmNow = globalAlarmTriggered && !isSnoozed();
  if (alarmNow && uiScreen != SCR_ALARM && uiScreen != SCR_CALIB) {
    uiScreen = SCR_ALARM;
    uiNeedFramework = true;
  } else if (!alarmNow && uiScreen == SCR_ALARM) {
    uiScreen = SCR_HOME;
    uiNeedFramework = true;
  }

  bool force = uiNeedFramework || (uiScreen != uiScreenDrawn);
  if (force) {
    resetUiCaches();
    uiScreenDrawn = uiScreen;
    uiNeedFramework = false;
  }

  switch (uiScreen) {
    case SCR_HOME:    drawHomeScreen(force);   break;
    case SCR_BED:     drawBedScreen(force);    break;
    case SCR_BED_SET: drawBedSetScreen(force); break;
    case SCR_NUMPAD:  drawNumpadScreen(force); break;
    case SCR_SYS:     drawSysScreen(force);    break;
    case SCR_ALARM:   drawAlarmScreen(force);  break;
    case SCR_CALIB:   drawCalibScreen(force);  break;
    default: break;
  }
}

// เรียกทุกรอบ loop เพื่ออ่านการแตะและส่งให้หน้าที่กำลังแสดงอยู่
void serviceTouch() {
  if (isPowerOffProgressActive) return;
  if (uiScreen == SCR_CALIB) { serviceCalibScreen(); return; }

  int tx, ty;
  if (!readTap(tx, ty)) return;

  if (displaySleeping) {                 // จอดับอยู่: แตะหนึ่งครั้งเพื่อปลุก
    setDisplaySleep(false);
    return;
  }

  switch (uiScreen) {
    case SCR_HOME:    handleHomeTap(tx, ty);   break;
    case SCR_BED:     handleBedTap(tx, ty);    break;
    case SCR_BED_SET: handleBedSetTap(tx, ty); break;
    case SCR_NUMPAD:  handleNumpadTap(tx, ty); break;
    case SCR_SYS:     handleSysTap(tx, ty);    break;
    case SCR_ALARM:   handleAlarmTap(tx, ty);  break;
    default: break;
  }
  if (uiNeedFramework) updateHostDisplay();    // ตอบสนองทันทีที่แตะ ไม่ต้องรอรอบวาดถัดไป
}

// ย้อนกลับหนึ่งขั้น (ใช้กับปุ่ม multifunction)
void uiGoBack() {
  switch (uiScreen) {
    case SCR_BED:     uiScreen = SCR_HOME;    break;
    case SCR_BED_SET: uiScreen = SCR_BED;     break;
    case SCR_NUMPAD:  uiScreen = (uiEditField == 4) ? SCR_SYS : SCR_BED_SET; break;
    case SCR_SYS:     uiScreen = SCR_HOME;    break;
    case SCR_CALIB:   uiScreen = SCR_SYS; calibStep = 0; break;
    default:          uiScreen = SCR_HOME;    break;
  }
  uiNeedFramework = true;
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
  // [4.10.0-TOUCH] เพิ่ม: เครื่องรีบูตเองแล้วไม่มีใครรู้สาเหตุ = หาต้นตอในวอร์ดจริงไม่ได้
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
  json += "\"linkWeakPct\":" + String(LINK_WEAK_PCT) + ",";
  json += "\"syncFail\":" + String(syncSendFail) + ",";
  json += "\"heardBeyond\":" + String(heardBeyondBedCount()) + ",";
  json += "\"rtc\":\"" + jsonEscape(rtcStatusText().c_str()) + "\",";
  json += "\"sd\":\"" + jsonEscape(sdStatusText().c_str()) + "\",";
  json += "\"sdRows\":" + String(sdRowsWritten) + ",";
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
    json += "\"link\":" + String(isOnline ? s.linkPct : 0) + ",";
    json += "\"rxTotal\":" + String(s.rxTotal) + ",";
    json += "\"mac\":\"" + macTail(s) + "\",";
    json += "\"idConflict\":" + String(s.idConflict ? "true" : "false") + ",";
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
  // ต้องจำค่าเดิมไว้ "ก่อน" เขียนทับ เดิมประกาศ oldPlan ไว้ใต้บล็อกนี้ ทำให้ได้ค่าใหม่เสมอ
  // เงื่อนไขเทียบจึงเป็นเท็จตลอด และการเปลี่ยนปริมาตรถุงอย่างเดียวไม่เคยรีเซ็ตเสียงเตือนใกล้หมด
  float oldPlan = c.planVolumeMl;
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
  requestSyncNow(idx);          // ส่งเฉพาะเตียงนี้ในช่องเวลาถัดไป (ไม่บล็อกหน้าเว็บ)
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
  requestSyncNow(idx);
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/stations/ack  station — รับทราบเตือนใกล้หมดจากหน้าเว็บ
void handleStationAck() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int idx = parseStationArg();
  if (idx < 0) { server.send(400, "text/plain", "Invalid station"); return; }
  stations[idx].nearEndAck = true;
  stations[idx].lostLinkAck = true;   // [4.10.0-TOUCH] เพิ่ม: ปุ่มรับทราบเดิมใช้ปิดเสียงเตียงที่ขาดการติดต่อได้ด้วย
  requestSyncNow(idx);
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
      if (uiSelectedBed >= activeStationCount) uiSelectedBed = 0;
      uiNeedFramework = true;      // จำนวนเตียงเปลี่ยน -> วาดหน้าจอใหม่ทั้งหมด
      requestSyncAll();            // เตียงที่เพิ่งเปิดใช้จะได้ค่าตั้งทันที
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
// ไฟล์บันทึกบน SD card — รายการไฟล์ และดาวน์โหลดผ่านหน้าเว็บ
// ----------------------------------------------------------------------------

// อนุญาตเฉพาะชื่อไฟล์ธรรมดาในโฟลเดอร์ /IVLOG (กันการอ่านไฟล์นอกโฟลเดอร์)
bool sdSafeName(const String &name) {
  if (name.length() == 0 || name.length() > 40) return false;
  for (unsigned int i = 0; i < name.length(); i++) {
    char c = name.c_str()[i];
    bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
    if (!ok) return false;
  }
  if (name == "." || name == "..") return false;
  return true;
}

// GET /api/sd/list -> { "present":true, "rows":123, "files":[{"name":"...","size":123}] }
void handleSdList() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  String json = "{\"present\":";
  json += sdPresent ? "true" : "false";
  json += ",\"rows\":" + String(sdRowsWritten);
  json += ",\"files\":[";
  if (sdPresent) {
    File dir = SD.open("/IVLOG");
    if (dir && dir.isDirectory()) {
      bool first = true;
      File f = dir.openNextFile();
      while (f) {
        if (!f.isDirectory()) {
          String nm = String(f.name());
          int slash = -1;
          for (unsigned int i = 0; i < nm.length(); i++) if (nm.c_str()[i] == '/') slash = (int)i;
          if (slash >= 0) nm = nm.substring(slash + 1);
          if (!first) json += ",";
          first = false;
          json += "{\"name\":\"" + jsonEscape(nm.c_str()) + "\",\"size\":" + String((unsigned long)f.size()) + "}";
        }
        f.close();
        f = dir.openNextFile();
      }
    }
    if (dir) dir.close();
  }
  json += "]}";
  server.send(200, "application/json", json);
}

// GET /api/sd/download?file=20260916_data.csv
void handleSdDownload() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  if (!sdPresent) { server.send(503, "text/plain", "No SD card"); return; }
  if (!server.hasArg("file")) { server.send(400, "text/plain", "Missing file"); return; }
  String name = server.arg("file");
  if (!sdSafeName(name)) { server.send(400, "text/plain", "Bad file name"); return; }
  String path = "/IVLOG/" + name;
  if (!SD.exists(path)) { server.send(404, "text/plain", "Not found"); return; }
  File f = SD.open(path, FILE_READ);
  if (!f) { server.send(500, "text/plain", "Open failed"); return; }
  server.sendHeader("Content-Disposition", "attachment; filename=" + name);
  server.streamFile(f, "text/csv; charset=utf-8");
  f.close();
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
  syncRtcFromSystem();                 // เก็บลง DS3231 ให้เวลาคงอยู่แม้ไฟดับ
  backupClockToNvs();
  lastTimeBackup = millis();
  sdLogEvent("TIME_SET", -1, getFormattedDateTime());
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
  
  bootResetReason = esp_reset_reason();   // [4.10.0-TOUCH] เพิ่ม: จำไว้ว่ารีบูตครั้งล่าสุดเพราะอะไร
  Serial.printf("[boot] reset reason: %s\n", resetReasonText(bootResetReason));
  setupLoopWatchdog();
}

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(HOST_BAT_ADC_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

#if TFT_BLK >= 0
  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(TFT_BLK, 5000, 8);        // ไฟหน้าจอควบคุมด้วย PWM เพื่อปรับความสว่างได้
  #else
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BLK, 0);
  #endif
#endif
  SPI_TFT.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);   // MISO จำเป็นสำหรับอ่านค่าทัช
  tft.init(240, 320);              // ความละเอียดจริงของแผง
  tft.setSPISpeed(40000000);
  tft.setRotation(TFT_ROTATION);   // 0 = แนวตั้ง 240x320
  tft.setTextWrap(false);
  tft.fillScreen(0x0000);

  ts.begin(SPI_TFT);               // ทัชใช้บัส SPI เดียวกับจอ
  ts.setRotation(TFT_ROTATION);
  loadTouchCalibration();

  hostBatteryVolts = readHostBattery();
  hostBatteryPct = calculateBatteryPct(hostBatteryVolts);

  preferences.begin("sys-config", true);
  activeStationCount = preferences.getUChar("bedCount", 5);
  screenBrightness   = preferences.getUChar("bright", 100);
  uint32_t savedEpoch = preferences.getUInt("lastEpoch", 0);
  preferences.end();
  if (screenBrightness < 25 || screenBrightness > 100) screenBrightness = 100;
  setBacklightRaw(screenBrightness);

  // เขตเวลาไทยถาวร
  setenv("TZ", TZ_THAILAND, 1);
  tzset();

  // ลำดับแหล่งเวลา: DS3231 (แม่นที่สุด) -> ค่าที่สำรองไว้ใน NVS -> รอหน้าเว็บส่งเวลามาให้
  initRtc();
  if (!isTimeSynced && savedEpoch > 1600000000UL) {
    struct timeval tv = { (time_t)savedEpoch, 0 };
    settimeofday(&tv, NULL);
    isTimeApprox = true;
  }

  initSdCard();
  for (int i = 0; i < MAX_SUPPORTED_STATIONS; i++) lastLoggedAlert[i] = ALERT_NONE;
  sdLogEvent("BOOT", -1, String("Host v" APP_VERSION " started"));
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
  server.on("/api/sd/list", handleSdList);
  server.on("/api/sd/download", handleSdDownload);
  server.begin();

  drawSplashScreen();
  lastUserActivityTime = millis();
  playWelcomeMelody();
  delay(1400);
  uiScreen = touchCalibrated ? SCR_HOME : SCR_CALIB;   // ยังไม่เคยปรับจอสัมผัส -> ปรับก่อนใช้งาน
  uiScreenDrawn = (UiScreen)-1;
  uiNeedFramework = true;

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
  feedWatchdog();   // [4.10.0-TOUCH] เพิ่ม: บอกสุนัขเฝ้าบ้านว่ายังเดินอยู่
  server.handleClient();

  checkMultifunctionButton();
  serviceTouch();                 // อ่านการแตะหน้าจอทุกรอบ เพื่อให้ตอบสนองทันที
  handleBuzzerAlarm();

  unsigned long currentMillis = millis();

  // สำรองเวลาลง NVS เป็นระยะ เพื่อให้เวลายังใกล้เคียงเดิมหลังไฟดับ
  if (currentMillis - lastTimeBackup >= TIME_BACKUP_INTERVAL_MS) {
    lastTimeBackup = currentMillis;
    backupClockToNvs();
  }
  serviceRtc(currentMillis);           // ปรับเวลาระบบตาม DS3231 ทุก 1 ชั่วโมง
  serviceSdCard(currentMillis);        // ลองต่อ SD card ใหม่ถ้าเพิ่งเสียบ/หลุด

  // ---- ประเมินเตือนทุก 1 วินาที (การส่ง Sync แยกไปที่ตัวจัดคิวด้านล่าง) ----
  if (currentMillis - lastSyncBroadcastTime >= SYNC_INTERVAL_MS) {
    lastSyncBroadcastTime = currentMillis;
    evaluateClinicalAlerts();
    sdLogAlertChanges();               // รหัสเตือนเปลี่ยน -> บันทึกลงไฟล์เหตุการณ์
  }

  // ส่ง Sync ทีละใบตามช่องเวลา ไม่มี delay() จึงไม่ขวางการรับข้อมูลจาก Station
  serviceSyncScheduler();
  serviceLinkQuality();

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
  if (manualFocusBed >= 0 && (long)(currentMillis - manualFocusUntil) >= 0) {   // [4.10.0-TOUCH] แก้: เทียบผลต่างแบบมีเครื่องหมาย จึงทนการวนรอบของ millis()
    manualFocusBed = -1;
    uiNeedFramework = true;
  }

  // หรี่จอเมื่อไม่มีการใช้งาน และกลับมาสว่างเมื่อมีการแตะหรือกดปุ่ม
  if (!displaySleeping) {
    bool shouldDim = (currentMillis - lastUserActivityTime >= BACKLIGHT_DIM_MS) &&
                     !(globalAlarmTriggered && !isSnoozed()) && !anyNearEndPending();
    if (shouldDim != backlightDimmed) {
      backlightDimmed = shouldDim;
      setBacklightRaw(shouldDim ? BACKLIGHT_DIM_PCT : screenBrightness);
    }
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
    sdLogMinute(currentTimestamp);     // เขียนข้อมูลนาทีนี้ของทุกเตียงลง SD card
    lastMinuteLogTime += 60000;
  }
}
