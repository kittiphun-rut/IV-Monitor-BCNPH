/**
 * ============================================================================
 * โครงการวิจัย: ผลของการใช้นวัตกรรม Smart IV Alert ต่อความแม่นยำในการแจ้งเตือนและปริมาณสารน้ำที่ได้รับ
 * หน่วยงาน: หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * สถาบัน: วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 *
 * ระบบ: Central Host Gateway (เครื่องควบคุมและติดตามศูนย์กลาง)
 * เวอร์ชัน: 4.7.6-OLED (Protocol v3 — ใช้คู่กับ Bed Station 7.4.x / 7.5.x / 7.7.x)
 * บอร์ดประมวลผล: ESP32-S3 Dev Module (N16R8) + จอ 1.3" OLED (SH1106 I2C) + Passive Buzzer
 *
 * ผู้พัฒนาระบบ: นายกิตติพันธ์ รัตนคร (นักวิชาการคอมพิวเตอร์ มจร. วิทยาเขตแพร่)
 * อาจารย์ที่ปรึกษา: ดร.กรรณิการ์ กาศสมบูรณ์ (วิทยาลัยพยาบาลบรมราชชนนี แพร่)
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V4.7.6-OLED: นำการเชื่อมต่อเราเตอร์กลับมา เพื่อลดภาระซีพียู
 *
 *  ปัญหา: เมื่อมีสมาร์ตโฟนหลายเครื่องเปิดหน้าเว็บพร้อมกัน เครื่องต้องทำหน้าที่
 *  Access Point ให้ทุกเครื่องเอง ทั้งส่ง beacon จัดการการเข้าร่วม และกันบัฟเฟอร์
 *  ไว้ต่อเครื่อง งานเหล่านี้แย่งเวลาจากการรับสัญญาณ ESP-NOW ของเตียง
 *
 *  1) โหมดเครือข่ายใหม่: เกาะเราเตอร์เป็นหลัก ถ้าต่อไม่ได้ค่อยเปิด AP สำรอง
 *     ต่อเราเตอร์สำเร็จ = ปิด AP ทิ้ง ภาระส่วนนี้หายหมด ไม่ใช่ WIFI_AP_STA
 *     ที่เปิดทั้งสองอย่างค้างไว้เหมือน v4.6.0 ซึ่งไม่ได้ลดภาระเลย
 *  2) ต่อไม่ได้ใน 15 วินาที หรือเราเตอร์หลุดเกิน 20 วินาที -> เปิด AP กลับมาเอง
 *     พยาบาลต่อ SSID เดิมได้เหมือนทุกวัน ไม่มีช่วงที่เข้าหน้าเว็บไม่ได้
 *  3) ขณะใช้ AP สำรอง จะลองกลับไปเกาะเราเตอร์ทุก 10 นาที แต่เฉพาะตอนไม่มีใคร
 *     ต่อ AP อยู่ เพราะการสลับโหมดจะตัดการเชื่อมต่อของผู้ใช้
 *  4) ESP-NOW ใช้ช่องของเราเตอร์ได้เลย ไม่ต้องบังคับกลับเป็นช่อง 1
 *     เพราะ Station มี manageChannelHunting() ไล่หาช่องเอง เจอภายใน ~33 วินาที
 *     (v4.6.0 บังคับช่องกลับหลังเกาะเราเตอร์ ซึ่งทำให้ลิงก์เราเตอร์พังทันที)
 *  5) การค้นหาเครือข่ายใช้แบบไม่บล็อก หน้าเว็บถามผลซ้ำจนกว่าจะเสร็จ
 *     (v4.6.0 ใช้ WiFi.scanNetworks() แบบบล็อก ทำให้แพ็กเก็ตจากเตียงหายช่วงสแกน)
 *  6) สั่งปิดโหมดประหยัดพลังงานของวิทยุซ้ำสามจุดรอบการเข้าโหมด STA
 *     เพราะ Arduino core เรียก esp_wifi_set_ps() ทับตอน STA_START ด้วยค่าปริยาย
 *     WIFI_PS_MIN_MODEM ของ ESP32-S3 ซึ่งทำให้ตัวรับหลับและแพ็กเก็ตมาถึงช้าเป็นวินาที
 *  7) หน้าเว็บปรับจังหวะรีเฟรชตามจำนวนผู้ใช้จริง Host วัดอัตราเรียก /api/data
 *     แล้วบอก pollMs กลับไป ยืดได้ถึง 4 วินาทีเมื่อคนเปิดพร้อมกันมาก
 *     และเปลี่ยนจาก setInterval เป็น setTimeout ต่อทอด คำขอจึงไม่ทับถมกันเป็นคิว
 *  8) จอ OLED แสดงที่อยู่ที่ต้องพิมพ์ทั้งบนหน้าพักจอและหน้า LINK DIAGNOSTIC
 *     เพราะที่อยู่เปลี่ยนตามเราเตอร์ ไม่ใช่ 192.168.4.1 คงที่เหมือนเดิม
 *
 *  ตรรกะการวัด การแจ้งเตือน และโครงสร้างแพ็กเก็ต Protocol v3 ไม่เปลี่ยนแม้แต่ไบต์เดียว
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V4.7.2-OLED: ปรับหน้าเว็บ Dashboard ตามที่พยาบาลขอมา
 *  1) สถานะปกติมี "กรอบสีเขียว" รอบการ์ดเตียง มองแวบเดียวรู้ว่าเตียงไหนเรียบร้อย
 *     (เดิมปกติไม่มีกรอบ ทำให้แยกจากเตียงออฟไลน์ด้วยสายตายาก)
 *  2) การเตือนใกล้หมดบอกเป็น "% ของสารน้ำ" แทนตัวเลข mL ที่ตั้งไว้
 *     และเพิ่มบรรทัด "เหลือในกระปุก ... mL" ในการ์ดทุกใบ (เดิมมีแต่ยอดที่ให้ไปแล้ว)
 *  3) เพิ่มแถบแจ้งเตือนรวมบนหน้า Live Monitor บอกว่าเตียงไหนเป็นอะไร
 *     ครอบคลุมทั้ง ไหลช้า / ไหลเร็ว / ไม่ไหล / ให้ครบแล้ว / เซนเซอร์ไม่จับหยด
 *     พร้อมแถบสีส้มแยกต่างหากสำหรับเตียงที่ใกล้หมด (เฝ้าดู ไม่ใช่เหตุวิกฤต)
 *  4) เสียงเตือนบนหน้าเว็บกระตุ้นความสนใจขึ้น — จากเสียงไซน์ 880 Hz ครั้งเดียว
 *     เป็นชุด 4 พัลส์สลับสองความถี่แบบรถพยาบาล ใช้คลื่นสี่เหลี่ยมที่มีฮาร์มอนิกมาก
 *     จึงแทรกผ่านเสียงรบกวนในหอผู้ป่วยได้ดีกว่า และปล่อยซ้ำทุก 2 วินาทีจนกว่าเหตุจะหาย
 *  5) ป้ายเวอร์ชันบนหัวหน้าเว็บอ่านจาก APP_VERSION จริง (เดิมค้างที่ v4.6.0)
 *
 *  ตรรกะการวัด การแจ้งเตือน และโปรโตคอล ESP-NOW ไม่เปลี่ยนแม้แต่ไบต์เดียว
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V4.7.1-OLED: รองรับเครื่องประจำเตียงรุ่นใหม่ (v7.5.1 และ v7.7.0)
 *
 *  โครงสร้างแพ็กเก็ต ESP-NOW ของ Station v7.5.1 และ v7.7.0 เหมือน v7.4.x ทุกไบต์
 *  (struct_message 23 ไบต์ / struct_host_sync 20 ไบต์) และสูตรตัดสินสายพับก็สูตรเดียวกัน
 *  จึงใช้งานร่วมกันได้อยู่แล้วโดยไม่ต้องแก้โปรโตคอล — ตรวจซ้ำได้ด้วย tools/protocol-test
 *
 *  แต่มีช่องโหว่ที่เห็นชัดขึ้นเมื่อใช้กับ v7.7.0 ซึ่งมีสถานะ "เซนเซอร์ยังจับหยดไม่ได้"
 *  ในตัวเครื่องเอง ขณะที่ Host เดิมไม่มีสถานะนี้เลย:
 *
 *   เตียงที่ออนไลน์อยู่ กำลังนับอยู่ แต่เซนเซอร์ไม่เคยจับหยดได้สักหยดเดียว
 *   (วางเซนเซอร์ผิดตำแหน่ง สายหลุด เลนส์สกปรก) Host เดิมจะขึ้นว่า "ปกติ" ตลอดไป
 *   เพราะเงื่อนไขสายพับกำหนดว่าต้องเคยมีหยดมาก่อน (totalDrops == 0 -> ไม่เข้าเงื่อนไข)
 *   นี่คือความล้มเหลวแบบเงียบ ซึ่งอันตรายกว่าการแจ้งเตือนผิด เพราะพยาบาลเข้าใจว่า
 *   ระบบกำลังเฝ้าอยู่ ทั้งที่ไม่ได้นับอะไรเลย
 *
 *  สิ่งที่เพิ่ม
 *   - รหัสเตือนใหม่ ALERT_NO_SIGNAL (6) = "เซนเซอร์ยังไม่จับหยด" แยกจากสายพับชัดเจน
 *     เพราะสองอย่างนี้ให้พยาบาลไปดูคนละจุด (สายพับ = ดูสายน้ำเกลือ, ไม่มีสัญญาณ = ดูเซนเซอร์)
 *   - เงื่อนไข: ออนไลน์ + กำลังนับ + ยังไม่เคยมีหยดเลย + ต่อเนื่องเกิน NO_SIGNAL_MS
 *   - ถือเป็นเหตุวิกฤต จึงมีเสียงเตือนที่ Host และขึ้นบนจอ OLED / หน้าเว็บ / ไฟล์ CSV
 *   - **ไม่ส่งรหัสนี้ออกไปที่ Station** เพื่อความเข้ากันได้ย้อนหลังอย่างสมบูรณ์
 *     (Station รุ่นเก่าไม่รู้จักรหัส 6) ตอนส่ง Sync จะแปลงเป็น ALERT_NONE เสมอ
 *     เครื่องที่เตียงรุ่น v7.7.0 มีสถานะ CHECK SENSOR ของตัวเองอยู่แล้ว
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V4.7.0: ตัดระบบจัดการ Wi-Fi (Router/อินเทอร์เน็ต) ออกทั้งหมด
 *  - Host เกาะเราเตอร์เป็นหลัก (WIFI_STA) ถ้าต่อไม่ได้เปิด Access Point สำรอง (WIFI_AP)
 *    -> ไม่มีการสแกนช่อง/รีคอนเนกต์มาแย่งเวลาคลื่นวิทยุ ESP-NOW และ Web Server อีก
 *  - รองรับสมาร์ตโฟน/แท็บเล็ตพร้อมกันได้ถึง AP_MAX_CLIENTS เครื่อง (เดิมค่าปริยาย 4
 *    และในโหมด AP+STA เหลือใช้งานจริงเพียง 2-3 เครื่อง)
 *  - ปิดโหมดประหยัดพลังงานของ Wi-Fi (WIFI_PS_NONE) ให้ตอบสนองหลายเครื่องพร้อมกันได้นิ่ง
 *  - ESP-NOW ส่งผ่านอินเทอร์เฟซที่ใช้อยู่จริง (WIFI_IF_STA หรือ WIFI_IF_AP)
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

#define APP_VERSION         "4.7.7-OLED"
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
#define SYNC_INTERVAL_MS        1000     // แต่ละเตียงได้รับ Sync ครบ 1 ใบทุก 1 วินาที
#define SYNC_SLOT_MIN_MS        60       // ช่องเวลาต่ำสุดระหว่างแพ็กเก็ต Sync สองใบ
#define ONLINE_TIMEOUT_MS       8000     // ไม่ได้รับข้อมูลเกิน 8 วินาที = OFFLINE
                                         // Station ส่งวินาทีละใบ ค่านี้จึงยอมให้หายติดกัน 8 ใบ
                                         // (เดิม 5 วินาที = 5 ใบ ซึ่งน้อยเกินไปเมื่อมีหลายเตียง)
#define LINK_WINDOW_MS          10000UL  // หน้าต่างวัดคุณภาพลิงก์ (คาดหวัง 10 ใบ)
#define LINK_WEAK_PCT           60       // ต่ำกว่านี้ = ลิงก์อ่อน เตือนก่อนหลุดจริง
#define ID_CONFLICT_WINDOW_MS   30000UL  // ช่วงเวลาที่นับการสลับ MAC ของเลขเตียงเดียวกัน
#define ID_CONFLICT_MIN_FLIPS   4        // สลับเกินเท่านี้ใน 1 ช่วง = มีบอร์ดตั้งเลขซ้ำกันแน่
#define HEARD_REMEMBER_MS       60000UL  // จำไว้ว่าเคยได้ยินเลขเตียงนี้นานเท่าใด
#define OLED_ANIM_INTERVAL_MS   90       // ~11 เฟรม/วินาที ขณะมีหยดให้แสดง
#define OLED_IDLE_INTERVAL_MS   400      // ไม่มีหยดให้แสดง
#define OLED_I2C_HZ             400000   // 400 kHz = ตามสเปก SH1106 (เดิมใช้ค่าปริยาย 100 kHz)
#define DROP_FALL_MS            240      // เวลาที่หยดหนึ่งหยดใช้ตกในแอนิเมชัน
#define MIN_OCCLUSION_MS        8000     // เวลาต่ำสุดที่ไม่มีหยดก่อนถือว่าหยุดไหล
#define NO_SIGNAL_MS            90000UL  // ติดต่อกันได้นานเท่านี้แต่ไม่เคยมีหยดเลย
                                         // = เซนเซอร์ยังจับหยดไม่ได้ (ไม่ใช่สายพับ)
                                         // 90 วินาที เผื่อเวลาพยาบาลจัดตำแหน่งเซนเซอร์เสร็จ
#define MAX_OCCLUSION_MS        300000   // เพดานเวลาตัดสินหยุดไหล (อัตราต่ำมาก)
#define RATE_DEVIATION_HOLD_MS  20000    // เร็ว/ช้าเกินต่อเนื่อง 20 วินาทีจึงเตือน
#define SNOOZE_MS               120000   // พักเสียง 2 นาที
#define NEAR_END_REMIND_MS      300000   // เตือนใกล้หมดซ้ำทุก 5 นาทีจนกว่าจะรับทราบ
#define DEFAULT_NEAR_END_PCT    80       // เตือนเมื่อให้ไปแล้ว 80% ของปริมาตรตามแผน
#define AP_MAX_CLIENTS          8        // จำนวนสมาร์ตโฟน/แท็บเล็ตที่ต่อพร้อมกันได้ (สูงสุด 10)
#define TIME_BACKUP_INTERVAL_MS 600000   // สำรองเวลาปัจจุบันลง NVS ทุก 10 นาที

// ---- โหมดเครือข่าย: เกาะเราเตอร์เป็นหลัก ถ้าต่อไม่ได้ค่อยเปิด AP สำรอง ----
// เหตุผล: เมื่อสมาร์ตโฟนต่อผ่านเราเตอร์ Host ไม่ต้องทำหน้าที่ Access Point
// จึงไม่มีภาระ beacon / การจัดการ client / บัฟเฟอร์ต่อเครื่อง
#define STA_CONNECT_TIMEOUT_MS  15000    // รอเกาะเราเตอร์นานสุด 15 วินาที
#define STA_LOST_GRACE_MS       20000    // หลุดจากเราเตอร์เกินเท่านี้ -> เปิด AP สำรอง
#define STA_RETRY_INTERVAL_MS   600000   // ขณะใช้ AP สำรอง ลองกลับไปเกาะเราเตอร์ทุก 10 นาที
                                         // (ลองเฉพาะตอนไม่มีใครต่อ AP อยู่ จะได้ไม่กวนผู้ใช้)

// ---- ปรับจังหวะ poll ของหน้าเว็บตามจำนวนผู้ใช้จริง ----
#define API_RATE_WINDOW_MS      5000     // หน้าต่างนับอัตราการเรียก /api/data
#define POLL_MS_MIN             1000     // ผู้ใช้น้อย = รีเฟรชทุก 1 วินาทีเหมือนเดิม
#define POLL_MS_MAX             4000     // ผู้ใช้มาก = ยืดถึง 4 วินาที กันซีพียูตัน

// รหัสแจ้งเตือน (ใช้ร่วมกันทั้ง Host / Station / Web)
#define ALERT_NONE        0
#define ALERT_TOO_FAST    1
#define ALERT_TOO_SLOW    2
#define ALERT_NEAR_END    3   // ให้ไปแล้วถึง % ที่ตั้ง (เตือนเบา ไม่ใช่เหตุวิกฤต)
#define ALERT_COMPLETE    4   // ให้ครบตามแผนแล้ว
#define ALERT_OCCLUSION   5   // สายพับ / หยุดไหล
#define ALERT_NO_SIGNAL   6   // เซนเซอร์ยังไม่จับหยดเลย (คนละเรื่องกับสายพับ)
                              // ใช้ภายใน Host เท่านั้น ไม่ส่งออกไปที่ Station

// โหมดภาพของกระเปาะหยดบนจอ Host — ให้ตรงกับที่จอ Station แสดงในหน้า 1 IV BAG
// (ประกาศไว้ตอนต้นไฟล์ เพราะ Arduino แทรก prototype ของฟังก์ชันไว้ก่อนส่วนแสดงผล)
enum DripMode : uint8_t {
  DRIP_FLOW = 0,     // กำลังไหล — มีหยดวิ่งลงมา
  DRIP_PAUSED = 1,   // พยาบาลสั่งหยุดนับ — สัญลักษณ์ Pause เหมือนจอ Station
  DRIP_ALERT = 2,    // สายพับ / เซนเซอร์ยังไม่จับหยด — เครื่องหมายตกใจ
  DRIP_OFFLINE = 3   // ติดต่อเตียงไม่ได้ — กากบาท
};

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

// ---- สถานะเครือข่าย ----
enum NetMode : uint8_t { NET_MODE_AP = 0, NET_MODE_STA = 1 };
NetMode  netMode          = NET_MODE_AP;   // โหมดที่ใช้อยู่จริงในขณะนี้
String   staSsid          = "";
String   staPass          = "";
bool     staConfigured    = false;         // มีรหัสเราเตอร์บันทึกไว้หรือไม่
bool     staConnecting    = false;         // กำลังรอเกาะเราเตอร์อยู่
uint32_t staAttemptStart  = 0;
uint32_t staLostSince     = 0;             // เวลาที่เริ่มหลุดจากเราเตอร์ (0 = ไม่ได้หลุด)
uint32_t lastStaRetry     = 0;
int8_t   lastStaRssi      = 0;

// ---- ตัวนับอัตราการเรียก /api/data เพื่อปรับจังหวะ poll ให้หน้าเว็บ ----
uint16_t apiDataHits       = 0;
uint32_t apiRateWindowAt   = 0;
uint16_t currentPollMs     = POLL_MS_MIN;
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
  unsigned long firstRecvTime = 0;   // ครั้งแรกที่ติดต่อกันได้ (ใช้ตรวจว่าเซนเซอร์ยังไม่จับหยด)

  // ---- คุณภาพลิงก์: นับแพ็กเก็ตที่รับได้จริงเทียบกับที่ควรได้ ----
  uint16_t rxWindowCount = 0;        // จำนวนใบที่รับได้ในหน้าต่างปัจจุบัน
  uint8_t  linkPct = 0;              // 0-100 % ของแพ็กเก็ตที่ควรได้รับ
  uint32_t rxTotal = 0;              // สะสมตั้งแต่เปิดเครื่อง (ไว้วินิจฉัย)

  // ---- ล็อกเฟสแอนิเมชันหยดให้ตรงกับหยดจริงที่ Station วัดได้ ----
  unsigned long lastDropAtMs = 0;    // เวลาโดยประมาณ (ฐานเวลา Host) ที่หยดล่าสุดตกลงมา

  // ---- ตรวจจับ "สองบอร์ดตั้งเลขเตียงซ้ำกัน" ----
  // Host แยกเตียงจาก stationId ในแพ็กเก็ตเท่านั้น ถ้าสองบอร์ดตั้งเลขเดียวกัน
  // มันจะเขียนทับช่องเดียวกัน เตียงที่เหลือจึงขึ้น OFFLINE ทั้งที่บอร์ดทำงานปกติ
  // อาการนี้มองจากจอ Station ไม่เห็นเลย เพราะทุกบอร์ดรับ Sync ของเลขนั้นได้เหมือนกัน
  uint8_t  srcMac[6] = {0};
  bool     macKnown = false;
  uint8_t  idFlipCount = 0;          // จำนวนครั้งที่ MAC ต้นทางสลับในช่วงที่กำลังนับ
  unsigned long idFlipWindowMs = 0;
  bool     idConflict = false;
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
unsigned long lastSyncSlotTime       = 0;
unsigned long lastLinkWindowTime     = 0;
uint16_t      pendingSyncMask        = 0;   // บิตที่ 0 = เตียง 1 ... ต้องส่ง Sync ทันที
uint8_t       syncRoundIdx           = 0;   // ตัวชี้ของการวนส่งตามรอบ
uint32_t      syncSendFail           = 0;   // จำนวนครั้งที่ esp_now_send ไม่สำเร็จ (ไว้วินิจฉัย)

// เลขเตียงที่ "เคยได้ยินจริง" ล่าสุด ใช้บอกว่ามีบอร์ดส่งเลขเกินจำนวนเตียงที่เปิดใช้อยู่หรือไม่
unsigned long heardIdAt[MAX_SUPPORTED_STATIONS] = {0};
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
         code == ALERT_COMPLETE || code == ALERT_OCCLUSION ||
         code == ALERT_NO_SIGNAL;
}

// รหัสที่ส่งออกไปให้ Station — Station รุ่นเก่าไม่รู้จัก ALERT_NO_SIGNAL
// จึงต้องแปลงเป็น ALERT_NONE เสมอ เพื่อให้เข้ากันได้กับทุกรุ่นตั้งแต่ 7.4.x ขึ้นไป
uint8_t alertCodeForStation(uint8_t code) {
  return (code == ALERT_NO_SIGNAL) ? ALERT_NONE : code;
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

// เตียงที่ออนไลน์และกำลังนับ แต่ไม่เคยมีหยดเลยตั้งแต่ติดต่อกันได้
// แยกจากสายพับ เพราะสายพับคือ "เคยไหลแล้วหยุด" ส่วนอันนี้คือ "ไม่เคยเริ่มเลย"
bool isStationNoSignal(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i) || !s.isRunning) return false;
  if (s.totalDrops > 0) return false;                 // เคยจับหยดได้แล้ว = ไม่ใช่กรณีนี้
  if (s.firstRecvTime == 0) return false;
  return (millis() - s.firstRecvTime) > NO_SIGNAL_MS;
}

// เตียงนี้มีมากกว่าหนึ่งบอร์ดส่งมาด้วยเลขเดียวกันหรือไม่
bool hasIdConflict(int i) {
  return stations[i].idConflict;
}

bool anyIdConflict() {
  for (int i = 0; i < activeStationCount; i++) if (stations[i].idConflict) return true;
  return false;
}

// เคยได้ยินเลขเตียงนี้ภายใน 1 นาทีที่ผ่านมาไหม (ใช้ได้แม้เลขเกินจำนวนเตียงที่เปิดใช้)
bool heardStationId(int i) {
  unsigned long t = heardIdAt[i];
  return t != 0 && (millis() - t) < HEARD_REMEMBER_MS;
}

// มีบอร์ดส่งเลขเตียงเกินจำนวนที่เปิดใช้อยู่หรือไม่ — คืนเลขเตียงตัวแรกที่เจอ (0 = ไม่มี)
uint8_t heardBeyondBedCount() {
  for (int i = activeStationCount; i < MAX_SUPPORTED_STATIONS; i++) {
    if (heardStationId(i)) return (uint8_t)(i + 1);
  }
  return 0;
}

// MAC ท้าย 3 ไบต์ พอให้แยกบอร์ดออกจากกันได้โดยไม่กินพื้นที่จอ
String macTail(const StationData &s) {
  if (!s.macKnown) return String("-");
  char b[12];
  snprintf(b, sizeof(b), "%02X:%02X:%02X", s.srcMac[3], s.srcMac[4], s.srcMac[5]);
  return String(b);
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
    case ALERT_NO_SIGNAL: return "NO SIGNAL";
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
// การจัดการ Wi-Fi: เกาะเราเตอร์เป็นหลัก ถ้าต่อไม่ได้ค่อยเปิด AP สำรอง
//
// ข้อควรระวังที่เคยทำให้รุ่น v4.6.0 พัง และห้ามทำซ้ำ
//   1) ห้ามเรียก esp_wifi_set_channel() ขณะอยู่โหมด STA
//      ช่องถูกกำหนดโดยเราเตอร์ ถ้าไปบังคับกลับเป็นช่อง 1 ลิงก์เราเตอร์จะพังทันที
//      ESP-NOW ไม่ต้องใช้ช่อง 1 เพราะ Station มี manageChannelHunting() ไล่หาช่องเอง
//   2) ห้ามใช้ WiFi.scanNetworks() แบบบล็อก ระหว่างสแกนวิทยุจะกระโดดข้ามช่อง
//      แพ็กเก็ต ESP-NOW หายหมด ใช้แบบ async แล้วมาเก็บผลทีหลังเท่านั้น
//   3) ห้ามวนรอเกาะเราเตอร์แบบบล็อกใน loop() ทุกอย่างต้องเช็กแล้วออกทันที
//   4) ต้องสั่งปิดโหมดประหยัดพลังงานซ้ำหลังเข้าโหมด STA
//      เพราะ Arduino core เรียก esp_wifi_set_ps(WiFi.getSleep()) ทับตอน STA_START
//      ซึ่งค่าปริยายของ ESP32-S3 คือ WIFI_PS_MIN_MODEM ทำให้ตัวรับหลับ
//      อาการคือแพ็กเก็ตจาก Station มาถึงช้าเป็นวินาที
// ----------------------------------------------------------------------------

// ผูก peer กระจายเสียงของ ESP-NOW เข้ากับอินเทอร์เฟซที่ใช้งานอยู่จริง
void attachEspNowPeer(wifi_interface_t ifidx) {
  esp_now_del_peer(broadcastAddress);          // ไม่เป็นไรถ้ายังไม่มี
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;                        // 0 = ใช้ช่องปัจจุบันของอินเทอร์เฟซ
  peerInfo.ifidx   = ifidx;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

// บังคับปิดโหมดประหยัดพลังงานของวิทยุ (ดูข้อ 4 ด้านบน)
void forceRadioAwake() {
  WiFi.setSleep(false);
  esp_wifi_set_ps(WIFI_PS_NONE);
}

void loadWifiConfig() {
  preferences.begin("wifi-config", true);
  staSsid = preferences.getString("ssid", "");
  staPass = preferences.getString("pass", "");
  preferences.end();
  staConfigured = (staSsid.length() > 0);
}

void saveWifiConfig(const String &ssid, const String &pass) {
  preferences.begin("wifi-config", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();
  staSsid = ssid;
  staPass = pass;
  staConfigured = (staSsid.length() > 0);
}

void clearWifiConfig() {
  preferences.begin("wifi-config", false);
  preferences.clear();
  preferences.end();
  staSsid = "";
  staPass = "";
  staConfigured = false;
}

// เปิด Access Point ของตัวเอง (โหมดสำรอง หรือเมื่อยังไม่ได้ตั้งรหัสเราเตอร์)
void startApMode() {
  staConnecting = false;
  WiFi.disconnect(false, false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(default_ap_ssid, default_ap_pass, ESPNOW_CHANNEL, 0, AP_MAX_CLIENTS);
  forceRadioAwake();
  netMode = NET_MODE_AP;
  lastStaRssi = 0;
  attachEspNowPeer(WIFI_IF_AP);
  Serial.printf("[HOST] โหมด AP: ช่อง %d รองรับ %d เครื่อง\n", currentWifiChannel(), AP_MAX_CLIENTS);
}

// เริ่มเกาะเราเตอร์ แบบไม่บล็อก (ผลลัพธ์ไปเช็กใน serviceWifi)
void startStaMode() {
  if (!staConfigured) { startApMode(); return; }
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  forceRadioAwake();                 // ครั้งที่ 1 ก่อน begin
  WiFi.begin(staSsid.c_str(), staPass.c_str());
  forceRadioAwake();                 // ครั้งที่ 2 หลัง begin เพราะ core ทับค่าตอน STA_START
  staConnecting   = true;
  staAttemptStart = millis();
  Serial.printf("[HOST] กำลังเกาะเราเตอร์ \"%s\"\n", staSsid.c_str());
}

void onStaConnected() {
  staConnecting = false;
  staLostSince  = 0;
  netMode       = NET_MODE_STA;
  forceRadioAwake();                 // ครั้งที่ 3 หลังเชื่อมต่อสำเร็จ กันไว้อีกชั้น
  attachEspNowPeer(WIFI_IF_STA);     // ไม่มี AP แล้ว ต้องส่งผ่าน STA
  lastStaRssi = WiFi.RSSI();
  Serial.printf("[HOST] เกาะเราเตอร์แล้ว IP %s ช่อง %d RSSI %d\n",
                WiFi.localIP().toString().c_str(), currentWifiChannel(), lastStaRssi);
  Serial.println("[HOST] ปิด AP แล้ว Station จะไล่หาช่องใหม่เองภายใน ~33 วินาที");
}

// เรียกทุกรอบของ loop() ห้ามบล็อก
void serviceWifi(uint32_t now) {
  // --- กำลังรอเกาะเราเตอร์ ---
  if (staConnecting) {
    if (WiFi.status() == WL_CONNECTED) { onStaConnected(); return; }
    if (now - staAttemptStart >= STA_CONNECT_TIMEOUT_MS) {
      Serial.println("[HOST] เกาะเราเตอร์ไม่สำเร็จ เปิด AP สำรอง");
      startApMode();
    }
    return;
  }

  // --- ใช้งานผ่านเราเตอร์อยู่: เฝ้าดูว่าหลุดหรือยัง ---
  if (netMode == NET_MODE_STA) {
    if (WiFi.status() == WL_CONNECTED) {
      staLostSince = 0;
      lastStaRssi  = WiFi.RSSI();
    } else {
      if (staLostSince == 0) staLostSince = now;
      else if (now - staLostSince >= STA_LOST_GRACE_MS) {
        Serial.println("[HOST] เราเตอร์หลุดเกินเวลาผ่อนผัน เปิด AP สำรอง");
        startApMode();
        lastStaRetry = now;
      }
    }
    return;
  }

  // --- ใช้ AP สำรองอยู่: ลองกลับไปเกาะเราเตอร์เป็นระยะ ---
  // ลองเฉพาะตอนไม่มีใครต่อ AP อยู่ เพราะการสลับโหมดจะตัดการเชื่อมต่อของผู้ใช้
  if (staConfigured && now - lastStaRetry >= STA_RETRY_INTERVAL_MS) {
    lastStaRetry = now;
    if (WiFi.softAPgetStationNum() == 0) {
      Serial.println("[HOST] ไม่มีใครต่อ AP อยู่ ลองกลับไปเกาะเราเตอร์");
      startStaMode();
    }
  }
}

// ชื่อเครือข่ายและที่อยู่ที่ผู้ใช้ต้องพิมพ์ เพื่อแสดงบนจอและหน้าเว็บ
String netModeLabel() { return netMode == NET_MODE_STA ? "ROUTER" : "AP"; }
IPAddress activeIP()  { return netMode == NET_MODE_STA ? WiFi.localIP() : WiFi.softAPIP(); }

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

// ----------------------------------------------------------------------------
// การส่ง Sync ให้ Station — แบบเฉลี่ยช่องเวลา ไม่มี delay() (ใหม่ใน v4.7.3)
// ----------------------------------------------------------------------------
// ของเดิมส่งรวดเดียว N ใบติดกัน คั่นด้วย delay(4) อยู่ใน loop() ทุก 1 วินาที
// ผลคือ (1) loop() ถูกบล็อก 32-50 ms ทุกวินาที และ (2) คลื่นวิทยุถูกใช้เป็นช่วงกระชาก
// ทับจังหวะที่ Station กำลังส่งข้อมูลกลับมาพอดี ยิ่งมีหลายเตียงยิ่งชนบ่อย
// ของใหม่ส่งทีละใบ ห่างกันเท่า ๆ กันตลอดวินาที แต่ละเตียงยังได้รับครบ 1 ใบต่อวินาทีเหมือนเดิม
uint16_t syncSlotIntervalMs() {
  uint8_t n = (activeStationCount < 1) ? 1 : activeStationCount;
  uint16_t slot = SYNC_INTERVAL_MS / n;
  if (slot < SYNC_SLOT_MIN_MS) slot = SYNC_SLOT_MIN_MS;
  return slot;
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
  syncMsg.alertCode    = alertCodeForStation(stations[i].alertCode);  // กันรหัสที่ Station รุ่นเก่าไม่รู้จัก
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

    // รหัสเตือนเปลี่ยน = ส่งให้ Station ในช่องเวลาถัดไป (<=125 ms) แทนที่จะรอครบรอบ
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
      currentOledPage = (currentOledPage + 1) % (activeStationCount + 2);   // +1 = หน้า LINK DIAG
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

#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  const uint8_t *src = (info && info->src_addr) ? info->src_addr : nullptr;
#else
  const uint8_t *src = mac;
#endif

  portENTER_CRITICAL(&dataMux);
  StationData &s = stations[idx];

  heardIdAt[idx] = millis();

  // ---- ตรวจว่ามีสองบอร์ดตั้งเลขเตียงเดียวกันหรือไม่ ----
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
  // ในฐานเวลาของ Host ได้ แอนิเมชันบนจอ Host จึงเดินตรงจังหวะกับหยดจริงที่เตียงนั้น
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
// ----------------------------------------------------------------------------
// แอนิเมชันหยดน้ำเกลือบนจอ OLED (ใหม่ใน v4.7.3)
// ----------------------------------------------------------------------------
// หลักคิด: ไม่ได้ "เดา" จังหวะหยดเอง แต่ล็อกเฟสกับหยดจริงที่ Station วัดได้
//   - Station ส่ง msSinceLastDrop มาทุกวินาที -> ย้อนกลับไปได้ว่าหยดล่าสุดตกเมื่อใด
//   - คาบการหยดคำนวณจากอัตราไหลจริง: 3600000 / (mL/h x หยดต่อ mL)
// ผลคือหยดที่เห็นบนจอ Host ตรงจังหวะกับหยดที่ตกจริงในกระเปาะของเตียงนั้น
// และเมื่อ Station หยุด/สายพับ ภาพจะเปลี่ยนเป็นสัญลักษณ์ชุดเดียวกับที่จอ Station ใช้

DripMode dripModeOf(int i) {
  if (!isStationOnline(i)) return DRIP_OFFLINE;
  const StationData &s = stations[i];
  if (!s.isRunning) return DRIP_PAUSED;
  if (s.alertCode == ALERT_OCCLUSION || s.alertCode == ALERT_NO_SIGNAL) return DRIP_ALERT;
  return DRIP_FLOW;
}

// คาบการหยดที่วัดได้จริง (มิลลิวินาทีต่อหนึ่งหยด) — 0 = ยังคำนวณไม่ได้
uint32_t dripPeriodMs(int i) {
  const StationData &s = stations[i];
  float dropsPerHr = s.flowRate_ml_hr * (float)safeDropFactor(s.cfg.dropFactor);
  if (dropsPerHr < 1.0f) return 0;
  uint32_t p = (uint32_t)(3600000.0f / dropsPerHr);
  if (p < 150) p = 150;        // เร็วกว่านี้ตามนุษย์ก็แยกไม่ออกอยู่ดี
  // v4.7.5: เพดานเดิม 20 วินาที ทำให้อัตราต่ำถูกบีบคาบลง แล้ว % period ไปวาด
  // "หยดปลอม" คั่นระหว่างหยดจริง (3 mL/h คาบจริง 60 วิ ถูกวาดทุก 20 วิ = ปลอม 2 หยด)
  // เพดานใหม่ผูกกับเกณฑ์สายพับ เลยจากนี้เตียงจะถูกตีเป็น DRIP_ALERT อยู่แล้ว
  if (p > MAX_OCCLUSION_MS) p = MAX_OCCLUSION_MS;
  return p;
}

// เฟสของแอนิเมชัน:  0..100 = กำลังตก (% ของระยะทาง)
//                   -1     = ยังไม่ถึงหยดถัดไป
//                   -2     = เพิ่งตกถึงผิวน้ำ (วาดคลื่นกระเพื่อม)
int dripAnimPhase(int i) {
  if (dripModeOf(i) != DRIP_FLOW) return -1;
  const StationData &s = stations[i];
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
  if (phase < fall + 120) return -2;
  return -1;
}

// มีเตียงไหนกำลังหยดอยู่บ้างไหม — ใช้ตัดสินว่าต้องรีเฟรชจอถี่หรือไม่
// v4.7.4: กระเปาะหยดเหลืออยู่เฉพาะหน้ารายละเอียดของแต่ละเตียง
// หน้ารวมจึงไม่ต้องรีเฟรชถี่อีกต่อไป ประหยัดทั้งบัส I2C และเวลาในลูป
bool anyBedDripping() {
  int idx = currentOledPage - 1;
  if (idx < 0 || idx >= activeStationCount) return false;
  return dripModeOf(idx) == DRIP_FLOW && dripPeriodMs(idx) > 0;
}



// คำสถานะสั้น ๆ ที่พยาบาลอ่านจบในแวบเดียว (อังกฤษ เพราะฟอนต์ในจอไม่มีตัวไทย)
const char* shortStatusOf(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i))      return "OFFLINE";
  if (s.idConflict)             return "ID DUP";
  if (!s.isRunning)             return "PAUSE";
  switch (s.alertCode) {
    case ALERT_TOO_FAST:  return "FAST";
    case ALERT_TOO_SLOW:  return "SLOW";
    case ALERT_OCCLUSION: return "STOP";
    case ALERT_NO_SIGNAL: return "NO DRIP";
    case ALERT_COMPLETE:  return "DONE";
    case ALERT_NEAR_END:  return s.nearEndAck ? "NEAR" : "NEW BAG";
    default:              return "OK";
  }
}

// เวอร์ชันสั้นไม่เกิน 4 ตัวอักษร สำหรับการ์ดในหน้ารวมที่มีที่จำกัด
// ต้องไม่เกิน 4 ตัว ไม่เช่นนั้นจะไปทับตัวเลขอัตราไหล
const char* gridStatusOf(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i))      return "OFF";
  if (s.idConflict)             return "DUP!";
  if (!s.isRunning)             return "PAUS";
  switch (s.alertCode) {
    case ALERT_TOO_FAST:  return "FAST";
    case ALERT_TOO_SLOW:  return "SLOW";
    case ALERT_OCCLUSION: return "STOP";
    case ALERT_NO_SIGNAL: return "SENS";
    case ALERT_COMPLETE:  return "DONE";
    case ALERT_NEAR_END:  return s.nearEndAck ? "near" : "BAG!";
    default:              return "OK";
  }
}

// เตียงนี้ต้องให้คนไปดูหรือไม่ (ใช้ตัดสินว่าจะทำการ์ดเป็นแถบทึบ)
bool bedNeedsAttention(int i) {
  const StationData &s = stations[i];
  if (!isStationOnline(i)) return true;
  if (s.idConflict) return true;
  return s.isRunning && isCriticalAlert(s.alertCode);
}



// ----------------------------------------------------------------------------
// REST APIs & Handlers
// ----------------------------------------------------------------------------
void handleApiData() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();

  // นับว่ามีการเรียกถี่แค่ไหน แล้วบอกจังหวะ poll ที่เหมาะสมกลับไปให้หน้าเว็บ
  // ผู้ใช้ยิ่งมาก ยิ่งยืดจังหวะ เพื่อไม่ให้ WebServer แย่งเวลาจากการรับ ESP-NOW
  uint32_t nowMs = millis();
  if (apiRateWindowAt == 0) apiRateWindowAt = nowMs;
  apiDataHits++;
  if (nowMs - apiRateWindowAt >= API_RATE_WINDOW_MS) {
    // จำนวนหน้าเว็บที่เปิดอยู่โดยประมาณ = อัตราเรียกต่อวินาที x จังหวะ poll ปัจจุบัน
    float perSec  = (float)apiDataHits * 1000.0f / (float)(nowMs - apiRateWindowAt);
    float viewers = perSec * (float)currentPollMs / 1000.0f;
    uint16_t want = (uint16_t)(POLL_MS_MIN * (viewers < 2.0f ? 1.0f : viewers / 2.0f));
    if (want < POLL_MS_MIN) want = POLL_MS_MIN;
    if (want > POLL_MS_MAX) want = POLL_MS_MAX;
    currentPollMs   = want;
    apiDataHits     = 0;
    apiRateWindowAt = nowMs;
  }

  String json;
  json.reserve(3000);
  json = "{";
  json += "\"version\":\"" APP_VERSION "\",";
  json += "\"pollMs\":" + String(currentPollMs) + ",";
  json += "\"netMode\":\"" + netModeLabel() + "\",";
  json += "\"netIP\":\"" + activeIP().toString() + "\",";
  json += "\"staRssi\":" + String(lastStaRssi) + ",";
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
    json += "\"link\":" + String(s.linkPct) + ",";
    json += "\"idConflict\":" + String(s.idConflict ? "true" : "false") + ",";
    json += "\"mac\":\"" + macTail(s) + "\",";
    json += "\"rxTotal\":" + String(s.rxTotal) + ",";
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
  // v4.7.5: ต้องจำค่าเดิมไว้ "ก่อน" เขียนทับ เดิมประกาศ oldPlan ไว้ใต้บล็อกนี้
  // ทำให้ oldPlan ได้ค่าใหม่เสมอ เงื่อนไขเทียบจึงเป็นเท็จตลอด และการเปลี่ยน
  // ปริมาตรถุงอย่างเดียวไม่เคยรีเซ็ตเสียงเตือนใกล้หมดให้ถุงใหม่
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
  requestSyncNow(idx);          // Station ต้องเห็น resetSeq ใหม่ทันที
  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/stations/ack  station — รับทราบเตือนใกล้หมดจากหน้าเว็บ
void handleStationAck() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  int idx = parseStationArg();
  if (idx < 0) { server.send(400, "text/plain", "Invalid station"); return; }
  stations[idx].nearEndAck = true;
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

      if (currentOledPage > activeStationCount + 1) currentOledPage = 0;
      if (syncRoundIdx >= activeStationCount) syncRoundIdx = 0;
      requestSyncAll();           // เตียงที่เพิ่งเปิดใช้จะได้ค่าตั้งทันที
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
  json += "\"mode\":\"" + netModeLabel() + "\",";
  json += "\"ip\":\"" + activeIP().toString() + "\",";
  json += "\"staSsid\":\"" + jsonEscape(staSsid.c_str()) + "\",";
  json += "\"staConfigured\":" + String(staConfigured ? "true" : "false") + ",";
  json += "\"staRssi\":" + String(lastStaRssi) + ",";
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

// ---------------------------------------------------------------------------
// ตั้งค่าเราเตอร์
// ---------------------------------------------------------------------------
bool scanBorrowedStaIface = false;   // ยืมอินเทอร์เฟซ STA มาสแกนชั่วคราวขณะอยู่โหมด AP

void handleWifiConfig() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  if (!server.hasArg("ssid")) { server.send(400, "text/plain", "Missing ssid"); return; }
  String ssid = server.arg("ssid");
  String pass = server.hasArg("pass") ? server.arg("pass") : "";
  if (ssid.length() == 0 || ssid.length() > 32 || pass.length() > 63) {
    server.send(400, "text/plain", "Bad ssid/pass");
    return;
  }
  saveWifiConfig(ssid, pass);
  // ตอบกลับก่อน แล้วค่อยสลับโหมด เพราะการสลับจะตัดการเชื่อมต่อของเครื่องที่ส่งคำสั่งมา
  server.send(200, "application/json", "{\"ok\":true,\"willSwitch\":true}");
  delay(150);                        // ให้แพ็กเก็ตตอบกลับออกไปให้พ้นก่อน
  startStaMode();
}

void handleWifiForget() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();
  clearWifiConfig();
  server.send(200, "application/json", "{\"ok\":true}");
  delay(150);
  startApMode();
}

// สแกนแบบไม่บล็อก: เรียกครั้งแรกเริ่มสแกนแล้วตอบ scanning:true
// หน้าเว็บเรียกซ้ำจนกว่าจะได้รายการ
void handleWifiScan() {
  if (!server.authenticate(web_username, web_password)) return server.requestAuthentication();

  int n = WiFi.scanComplete();

  if (n == WIFI_SCAN_FAILED) {           // ยังไม่เริ่ม -> เริ่มสแกน
    if (netMode == NET_MODE_AP && !scanBorrowedStaIface) {
      // โหมด AP ล้วนสแกนไม่ได้ ต้องเปิดอินเทอร์เฟซ STA ร่วมชั่วคราว
      WiFi.mode(WIFI_AP_STA);
      forceRadioAwake();
      scanBorrowedStaIface = true;
    }
    WiFi.scanNetworks(true);             // true = ไม่บล็อก
    server.send(200, "application/json", "{\"scanning\":true}");
    return;
  }
  if (n == WIFI_SCAN_RUNNING) {
    server.send(200, "application/json", "{\"scanning\":true}");
    return;
  }

  String json = "{\"scanning\":false,\"nets\":[";
  int shown = 0;
  for (int i = 0; i < n && shown < 20; i++) {
    if (WiFi.SSID(i).length() == 0) continue;
    if (shown) json += ",";
    json += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i).c_str()) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"lock\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true") + "}";
    shown++;
  }
  json += "]}";
  WiFi.scanDelete();

  if (scanBorrowedStaIface) {            // คืนโหมดเดิม
    scanBorrowedStaIface = false;
    WiFi.mode(WIFI_AP);
    forceRadioAwake();
    attachEspNowPeer(WIFI_IF_AP);
  }
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
  // 100 kHz (ค่าปริยาย) ทำให้ส่งภาพทั้งจอใช้เวลาราว 92 ms ต่อเฟรม ซึ่งช้าเกินกว่าจะทำ
  // แอนิเมชันได้ และยังหน่วง loop() ด้วย 400 kHz อยู่ในสเปกของ SH1106 และเหลือราว 23 ms
  u8g2.setBusClock(OLED_I2C_HZ);
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

  // ---------------- Wi-Fi: เกาะเราเตอร์เป็นหลัก ถ้าต่อไม่ได้ค่อยเปิด AP สำรอง ----------------
  // เมื่อสมาร์ตโฟนต่อผ่านเราเตอร์ Host ไม่ต้องทำหน้าที่ Access Point
  // จึงไม่มีภาระ beacon การจัดการ client และบัฟเฟอร์ต่อเครื่อง
  // ESP-NOW ใช้ช่องของเราเตอร์ได้เลย เพราะ Station ไล่หาช่องเองอัตโนมัติ
  WiFi.persistent(false);
  loadWifiConfig();

  // เปิด ESP-NOW ก่อน แล้วค่อยเลือกโหมด เพราะ attachEspNowPeer() ต้องใช้
  WiFi.mode(staConfigured ? WIFI_STA : WIFI_AP);
  forceRadioAwake();

  MDNS.begin("iv-monitor");

  // ---------------- ESP-NOW ----------------
  if (esp_now_init() != ESP_OK) {
    Serial.println("[HOST] ESP-NOW init FAILED");
  } else {
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  }

  // เลือกโหมดเครือข่าย (ทั้งสองทางจะผูก peer ของ ESP-NOW ให้ตรงอินเทอร์เฟซเอง)
  if (staConfigured) startStaMode();
  else               startApMode();

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
  server.on("/api/wifi/scan", handleWifiScan);
  server.on("/api/wifi/config", HTTP_POST, handleWifiConfig);
  server.on("/api/wifi/forget", HTTP_POST, handleWifiForget);
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

  // ---- ดูแลการเชื่อมต่อเครือข่าย (ไม่บล็อก) ----
  serviceWifi(currentMillis);

  // ---- ประเมินเตือนทุก 1 วินาที (การส่ง Sync แยกไปอยู่ที่ตัวจัดคิวด้านล่าง) ----
  if (currentMillis - lastSyncBroadcastTime >= SYNC_INTERVAL_MS) {
    lastSyncBroadcastTime = currentMillis;
    evaluateClinicalAlerts();
  }

  // ---- ส่ง Sync ทีละใบแบบเฉลี่ยช่องเวลา + วัดคุณภาพลิงก์ ----
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

  // รีเฟรชถี่เฉพาะตอนที่มีหยดให้แสดงจริง ๆ เพื่อไม่ให้เปลือง I2C และเวลาในลูปโดยเปล่าประโยชน์
  unsigned long oledRefreshInterval;
  if (oledDisplaySleeping || oledScreensaverActive) oledRefreshInterval = 1000;
  else if (anyBedDripping())                        oledRefreshInterval = OLED_ANIM_INTERVAL_MS;
  else                                              oledRefreshInterval = OLED_IDLE_INTERVAL_MS;
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
