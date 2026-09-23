/**
 * ============================================================================
 * โครงการวิจัย: ผลของการใช้นวัตกรรม Smart IV Alert ต่อความแม่นยำในการแจ้งเตือนและปริมาณสารน้ำที่ได้รับ
 * หน่วยงาน: หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น อำเภอสูงเม่น จังหวัดแพร่
 * สถาบัน: วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 *
 * ระบบ: Bed Station Node (เครื่องตรวจวัดและแจ้งเตือนประจำเตียง)
 * เวอร์ชัน: 7.5.4 (Protocol v3 — ใช้คู่กับ Central Host Firmware 4.6.0 / 4.7.0)
 * บอร์ดประมวลผล: ESP32-S3 Super Mini + จอสี TFT 1.47" (ST7789 SPI 172x320) + Passive Buzzer
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V7.5.4: แก้สูตรคำนวณอัตราการไหลให้ตรงกับวิธีที่พยาบาลวัดจริง
 *   ของเดิมเฉลี่ย "อัตราขณะนั้น" (60000/dt) แบบถ่วงน้ำหนัก 0.75 ซึ่งเป็นการเฉลี่ย
 *   ส่วนกลับของเวลา มีปัญหาสองข้อพร้อมกัน
 *     1) เอนสูงเสมอ — ตามอสมการเจนเซน ค่าเฉลี่ยของส่วนกลับ >= ส่วนกลับของค่าเฉลี่ย
 *     2) แกว่งแรง — น้ำหนัก 0.75 เกาะค่าขณะนั้นเกือบเต็ม
 *   ผลคือหยดสลับ 1.0/1.4 วินาที (ของจริง 150 mL/h ที่ DF 20) ทำให้จอแกว่ง
 *   138.9-169.7 mL/h ซึ่งหลุดกรอบ +-10% ที่คู่มือใช้ตัดสินการคาลิเบรต
 *
 *   ใหม่: นับหยดที่ตกจริงในหน้าต่าง 8 หยด แล้วหารด้วยเวลาที่ใช้ไปทั้งหน้าต่าง
 *     mL/h = (N-1) / dropFactor * 3600000 / (t_ใหม่สุด - t_เก่าสุด)
 *   เป็นวิธีเดียวกับที่พยาบาลนับหยด 1 นาที และเดียวกับที่ Host บันทึกลง Log
 *   รายนาที ตัวเลขทั้งสามจุดจึงตรงกันโดยนิยาม (ทดสอบแล้วต่างกัน 0.000%)
 *
 *   โครงสร้างแพ็กเก็ต Protocol v3 ไม่เปลี่ยนแม้แต่ไบต์เดียว
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V7.5.3: แก้อาการ "Host ขึ้น OFFLINE ทั้งที่ Station ยังออนไลน์"
 *   อาการนี้เกิดเมื่อมีหลายเตียงพร้อมกัน และสาเหตุอยู่ที่ "จังหวะส่ง" ไม่ใช่สัญญาณอ่อน
 *
 *   ของเดิมทุกสเตชันส่งข้อมูลทุก 1000 ms เป๊ะ ไม่มีการสุ่มเลย คริสตัลของบอร์ดสองตัว
 *   ต่างกันราว 40 ppm = คาบส่งเลื่อนหากันเพียง 0.04 ms ต่อวินาที เมื่อใดที่สองเตียง
 *   ส่งห่างกันไม่ถึงความยาวเฟรม (~0.8 ms) มันจะชนกันแบบนั้น "ค้างอยู่ราว 40 วินาที"
 *   กว่าจะเลื่อนพ้นกัน ระหว่างนั้น Host ไม่ได้รับของเตียงนั้นเลยจึงขึ้น OFFLINE
 *
 *   ทำไม Station ถึงยังขึ้นว่าออนไลน์ตลอด — เพราะ Host ส่ง Sync เตียงละใบ
 *   มีกี่เตียงก็ส่งเท่านั้นใบต่อวินาที และ Station รับทุกใบไม่ว่าจะจ่าหน้าถึงเตียงใด
 *   ฝั่ง Station จึงมีข้อมูลสำรองมากกว่าฝั่ง Host ถึง N เท่า (8 เตียง = 8 เท่า)
 *
 *   แก้โดย
 *    - สุ่มคาบส่งใหม่ทุกครั้งในช่วง 940-1060 ms สองเตียงที่บังเอิญชนกันจะแยกกัน
 *      ในรอบถัดไปทันที ไม่ใช่รอ 40 วินาที
 *    - รอบแรกหลังบูตเหลื่อมกันตามเลขเตียง (เตียงละ 111 ms) แพ็กเก็ตจึงกระจายกัน
 *      ทั้งวินาทีตั้งแต่ต้น
 *    - เมล็ดสุ่มมาจาก MAC ของบอร์ด จึงไม่ซ้ำกันแม้แฟลชเฟิร์มแวร์เดียวกันทุกตัว
 *
 *   อัตราการส่งเฉลี่ยยังเท่าเดิม (1 ใบ/วินาที) โครงสร้างแพ็กเก็ตไม่เปลี่ยน
 *   ใช้กับ Host รุ่นใดก็ได้ และใช้ปนกับสเตชันรุ่นเก่าได้
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V7.5.2: หมุนจอ 180 องศาได้ เพื่อให้ติดตั้งจอได้ทุกทิศทาง
 *   (พอร์ตมาจาก V7.7.1 — ระบบคัดกรองหยดและหน้าคาลิเบรตของ V7.5.1 ไม่เปลี่ยนเลย)
 *
 *   วิธีสลับ: คลิก 3 ครั้ง = เข้าหน้า SET BED ID -> ในหน้านั้น "ดับเบิลคลิก"
 *   = สลับ NORMAL <-> FLIP 180 จอพลิกทันทีให้เห็นผล แล้วบันทึกลง NVS ทันที
 *
 *   ทางเทคนิค: จอ ST7789 172x320 ใช้ setRotation(0) กับ (2) ซึ่งเป็นแนวตั้ง
 *   ขนาดเท่ากันทั้งคู่ (172x320) ต่างกันแค่บิต MX/MY ใน MADCTL พิกัดทุกจุดใน
 *   โค้ดวาดภาพจึงใช้ได้เหมือนเดิมทั้งหมด ไม่ต้องแก้เลย์เอาต์แม้แต่บรรทัดเดียว
 *   และไลบรารีตั้งออฟเซ็ตคอลัมน์ (_colstart/_colstart2 = 34) ไว้เท่ากันทั้งสองทิศ
 *   ภาพจึงไม่เลื่อนออกนอกจอ
 *
 *   ตั้งค่าเริ่มต้นตอนคอมไพล์ได้ที่ TFT_ROTATION_DEFAULT (ถ้าทั้งวอร์ดติดตั้งกลับหัว
 *   เหมือนกันหมด ตั้งเป็น TFT_ROT_FLIP180 แล้วแฟลชทีเดียวจบ)
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V7.5.1: รื้อระบบคัดกรองหยดและหน้าคาลิเบรตใหม่ทั้งหมด
 *   หน้าจอ การสื่อสาร และการแจ้งเตือนเหมือน V7.5.0 ทุกประการ
 *
 *  ปัญหา 1 "นับเบิ้ล" — ของเดิมเมื่อค่าต่ำกว่าเกณฑ์แต่ยังไม่พ้นเวลากันรัว 55 ms
 *   จะไม่นับ แต่ก็ไม่เปลี่ยนสถานะ ตัวจับจึงยังง้างค้างอยู่ พอพ้น 55 ms ขณะที่
 *   สัญญาณยังต่ำอยู่ ก็นับซ้ำจากหยดเดิม (และนับอีกทุก 55 ms ที่ยังค้าง)
 *   -> ของใหม่นับตอน "พัลส์จบ" ที่จุดเดียว แล้วต้องกลับมานิ่งใต้เกณฑ์ปลด 20 ms
 *      ก่อนจึงรับพัลส์ถัดไป จึงได้ 1 พัลส์ = 1 หยด เสมอ (แก้อาการเหมือนสวิตช์ลั่น)
 *
 *  ปัญหา 2 "หาช่วงต่างยาก" — ของเดิมตั้งเกณฑ์จากค่ากึ่งกลางระหว่างกระเปาะมีน้ำ
 *   กับกระเปาะเปล่า ซึ่งเป็นค่านิ่ง 2 ค่า แต่หยดที่ตกผ่านลำแสงเป็นสัญญาณชั่วขณะ
 *   ที่เบนจากค่าปกติเพียงบางส่วน เกณฑ์กึ่งกลางจึงสูงเกินกว่าจะจับหยดได้จริง
 *   -> ของใหม่วัด "ระยะเบนจากเส้นฐาน" โดยเส้นฐานไล่ตามค่าปกติเองตลอดเวลา
 *      (กันการดริฟต์จากอุณหภูมิ แสงรอบข้าง และแรงดันไฟ) และตั้งเกณฑ์จาก
 *      หยดจริงที่วัดได้ในขั้นตอนคาลิเบรต ไม่ต้องหาค่ากึ่งกลางเองอีก
 *
 *  สิ่งที่เพิ่มในสายสัญญาณ
 *   - อ่าน ADC ที่คาบคงที่ 2 kHz (เดิมอ่านรอบละครั้งตามจังหวะ loop)
 *   - มีเดียน 5 จุด ตัดสไปก์เดี่ยวจาก ADC และจากจังหวะที่ ESP-NOW ส่งข้อมูล
 *   - เส้นฐานปรับอัตโนมัติ และหยุดนิ่งระหว่างที่พัลส์กำลังดำเนินอยู่
 *   - กรองตามความกว้างพัลส์ 2-150 ms (แคบไป = สัญญาณรบกวน, กว้างไป = ไม่ใช่หยด)
 *   - รู้ทิศสัญญาณเอง (หยดทำให้ค่าลดลงหรือเพิ่มขึ้น) ไม่ต้องตั้งค่าเอง
 *   - ถ้าการอ่านขาดช่วงเกิน 25 ms (เช่นเพิ่งวาดจอทั้งหน้า) จะตั้งต้นใหม่
 *     แทนที่จะเดาพัลส์ที่มองเห็นไม่ครบ
 *
 *  หน้าคาลิเบรตใหม่ 3 ขั้น + ตรวจสอบ (กดปุ่มค้าง 3 วินาที) พร้อมจอคลื่นสด
 *   1 NOISE  วัดพื้นสัญญาณรบกวนของเซ็นเซอร์ 3 วินาที (ขีดล่างของฮาร์ดแวร์)
 *   2 DROPS  เรียนรู้จากหยดจริง 10 หยด เก็บความสูงและความกว้างพัลส์
 *   3 RESULT ตั้งเกณฑ์ที่จุดกึ่งกลางระหว่าง "พ้นสัญญาณรบกวน 3 เท่า" กับ
 *            "ต่ำกว่าหยดที่จางที่สุด 55%" แล้วรายงาน SNR ให้เห็นขีดความสามารถจริง
 *   VERIFY   นับหยดสดให้เทียบกับตาตัวเองว่า 1 หยดขึ้น 1 จริง
 *
 * ---------------------------------------------------------------------------
 * เปลี่ยนใน V7.5.0: ออกแบบหน้าจอ TFT ใหม่ทั้งหมด (UX/UI สำหรับใช้งานจริงที่เตียง)
 *  - แนวคิด "IV BAG": หน้าหลักแสดงภาพถุงน้ำเกลือที่ระดับน้ำลดลงจริงตามปริมาณที่ให้ไปแล้ว
 *    พร้อมกระเปาะหยดที่มีภาพเคลื่อนไหวตามหยดจริง และตัวเลขสำคัญเพียง 4 ค่า
 *    (อัตราไหล / ปริมาณคงเหลือ / เวลาที่เหลือ / เวลาที่จะหมด)
 *  - ลดข้อความบนจอลงเหลือเท่าที่จำเป็น ใช้สีและตำแหน่งแทนคำอธิบาย
 *    แถบสถานะสีที่ขอบบนบอกภาวะของเตียงตั้งแต่แรกเห็น (เขียว/เหลือง/ส้ม/แดง/ฟ้า)
 *  - เพิ่มหน้าที่ 4: กราฟแท่งแนวโน้มอัตราไหลย้อนหลัง 30 นาที พร้อมเส้นอัตราเป้าหมาย
 *    ใช้ดูว่าการไหลสม่ำเสมอหรือมีช่วงที่ตกไป โดยไม่ต้องเปิดหน้าเว็บ
 *  - หน้า PLAN และ LINK จัดใหม่เป็นการ์ดตัวเลขใหญ่ อ่านได้จากปลายเตียง
 *  - จอเตือนวิกฤต/ใกล้หมดถุง/Screensaver ออกแบบใหม่ให้เหลือข้อความเท่าที่ต้องอ่าน
 *  - ตรรกะการวัด การสื่อสาร และการแจ้งเตือนทั้งหมดเหมือน V7.4.0 ทุกประการ
 *    (โครงสร้างแพ็กเก็ต Protocol v3 ไม่เปลี่ยน ใช้กับ Host เดิมได้ทันที)
 *
 * ---------------------------------------------------------------------------
 * เพิ่มใน V7.4.0: แจ้งเตือนใกล้หมด (เตรียมถุงใหม่)
 *  - เตือนเมื่อให้สารน้ำไปแล้วถึง % ที่ตั้งจากหน้าเว็บ (ค่าเริ่มต้น 80% ของปริมาตรตามแผน)
 *  - แสดงจอสีส้ม "PREPARE NEXT BAG" + เสียงเตือนเบา 3 ครั้ง ซ้ำทุก 5 นาทีจนกว่าจะกดรับทราบ
 *  - คลิกปุ่มที่เตียง หรือกดรับทราบที่ Host/หน้าเว็บ = รับทราบ (ซิงก์กันทั้งระบบ)
 *  - ไม่นับเป็นเหตุวิกฤต ไม่เข้าจอ EMERGENCY สีแดง
 *
 * สรุปการแก้ไขจาก V7.2.3
 *  1) โครงสร้างแพ็กเก็ต ESP-NOW ตรงกับ Host ทุกไบต์ (มี static_assert ตรวจขนาด)
 *     ส่งอัตราไหล + เวลาตั้งแต่หยดล่าสุด + แรงดันแบตเตอรี่ (ถ้ามีวงจร)
 *  2) รับ Drop factor จาก Host แทนค่าคงที่ 20 gtt/mL
 *  3) แก้แจ้งเตือนสายพับผิด: เดิมเข้า EMERGENCY ทันทีหลังหยดแรก และค้างเมื่อกดหยุดชั่วคราว
 *     -> ใช้เกณฑ์ "ไม่มีหยดนานเกิน 2.5 เท่าของช่วงหยดตามเป้าหมาย (ขั้นต่ำ 8 วินาที)" สูตรเดียวกับ Host
 *  4) รหัสเตือน เร็ว/ช้าเกิน/ครบแผน มาจาก Host (มีหน่วงยืนยัน 20 วินาที) — สายพับตัดสินในเครื่องทันที
 *  5) ค้นหาช่องสัญญาณของ Host อัตโนมัติ (Channel 1-13) เมื่อขาดการเชื่อมต่อ และจำช่องล่าสุด
 *  6) ลดการวาดจอ TFT ทุกรอบ loop -> วาดเฉพาะเมื่อสถานะเปลี่ยน ไม่แย่งเวลาอ่าน ADC ตรวจจับหยด
 *  7) รองรับคำสั่ง "เริ่มถุงใหม่" จากหน้าเว็บ (รีเซ็ตตัวนับ)
 *  8) ในจอ EMERGENCY: คลิก 1 ครั้ง = พักเสียง 2 นาที, ดับเบิลคลิก = หยุด/เริ่มนับ
 *  9) Screensaver ทำงานจริงเมื่อไม่มีการกดปุ่ม 3 นาที (คลิกเพื่อปลุก)
 * 10) ไฟ RGB แสดงสถานะ: แดง=เตือน, เหลือง=หยุดชั่วคราว, น้ำเงิน=ไม่พบ Host, เขียว=ปกติ
 *
 * การใช้ปุ่ม (ปุ่มเดียว)
 *  - คลิก 1 ครั้ง      : เปลี่ยนหน้า 1/2/3/4  (ในจอเตือน = พักเสียง 2 นาที)
 *  - ดับเบิลคลิก       : หยุด/เริ่มนับหยด (PAUSE)
 *  - คลิก 3 ครั้ง      : ตั้งหมายเลขเตียง 1-8
 *  - กดค้าง 3 วินาที   : Calibration Wizard (วัดสัญญาณรบกวน + เรียนรู้จากหยดจริง)
 *  - กดค้าง 5 วินาที   : หน้าผู้พัฒนา
 * ============================================================================
 */

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <time.h>
#include <sys/time.h>

#define APP_VERSION         "7.5.5"

#define TFT_CS              9
#define TFT_DC              10
#define TFT_RST             11
#define TFT_MOSI            12
#define TFT_SCLK            13
#define TFT_BLK             8

// ---- ทิศการแสดงผลของจอ ----
// จอ ST7789 172x320 หมุนได้ 2 ทิศที่ยังเป็นแนวตั้งขนาดเดิม จึงใช้พิกัดชุดเดียวกัน
#define TFT_ROT_NORMAL        0   // สาย USB อยู่ด้านล่าง (ค่ามาตรฐาน)
#define TFT_ROT_FLIP180       2   // กลับหัว 180 องศา สาย USB อยู่ด้านบน
#define TFT_ROTATION_DEFAULT  TFT_ROT_NORMAL   // ค่าเริ่มต้นเมื่อยังไม่เคยตั้งที่หน้าเครื่อง

#define SENSOR_AO_PIN       4
#define BUZZER_PIN          3
#define BTN_PIN             2
#define RGB_LED_PIN         48

// วงจรวัดแบตเตอรี่ (ถ้ามี): ใส่หมายเลขขา ADC ที่ต่อผ่านตัวต้านทานแบ่งแรงดัน 1:1 เช่น 1
// -1 = ไม่มีวงจร -> ส่งค่า 0 และหน้าเว็บ/Host แสดง "N/A"
#define STATION_BAT_ADC_PIN -1
#define BAT_DIVIDER_RATIO   2.0f

// ----------------------------------------------------------------------------
// ตัวตรวจจับหยด (v7.5.1) — ทำงานกับ "ระยะเบนจากเส้นฐาน" ไม่ใช่ค่า ADC สัมบูรณ์
// ----------------------------------------------------------------------------
#define SENSOR_SAMPLE_US        500    // อ่าน ADC ทุก 0.5 ms (2 kHz) พัลส์ 10 ms จึงได้ ~20 จุด
#define SENSOR_MEDIAN_N         5      // มีเดียน 5 จุด ตัดสไปก์เดี่ยวของ ADC/ตอนส่ง ESP-NOW
#define SENSOR_GAP_RESYNC_MS    25     // อ่านขาดช่วงนานกว่านี้ = ตั้งต้นใหม่ ไม่เดาพัลส์ที่เห็นไม่ครบ
// ค่าคงที่เวลาของตัวไล่ตาม คิดจากเวลาจริง (หน่วย 100 ไมโครวินาที) ไม่ใช่จำนวนตัวอย่าง
// เพื่อให้พฤติกรรมคงที่ไม่ว่าลูปหลักจะยุ่งแค่ไหน
#define BASELINE_TAU            5000   // 500 ms — เส้นฐานไล่ตามการดริฟต์ช้า ๆ
#define BASELINE_TAU_FAST       1250   // 125 ms — ตอนถุงแกว่งหรือค่าค้างเหนือเกณฑ์ปลด
#define DELTA_SLOW_TAU          300    // 30 ms  — ตัวตัดองค์ประกอบช้าในสภาวะปกติ
#define DELTA_SLOW_TAU_MOTION   100    // 10 ms  — ตอนถุงแกว่ง ตัดให้แรงขึ้น
#define NOISE_WINDOW_MS         1000   // คาบวัดสัญญาณรบกวนยอดถึงยอด
#define DROP_MIN_WIDTH_MS       2      // พัลส์แคบกว่านี้ = สัญญาณรบกวน ไม่ใช่หยด
#define DROP_MAX_WIDTH_MS       250    // กว้างกว่านี้ = ระดับน้ำเปลี่ยน/มีอะไรบัง ไม่ใช่หยด
#define DROP_REARM_MS           20     // ต้องกลับมานิ่งใต้เกณฑ์ปลดเท่านี้ ก่อนรับหยดถัดไป
#define DROP_REARM_TIMEOUT_MS   300    // กันค้าง: อยู่ในช่วงรอนิ่งนานเกินนี้ = ยึดเส้นฐานใหม่

#define CAL_DEFAULT_TRIGGER     120    // ใช้เมื่อยังไม่เคยคาลิเบรต
#define CAL_MIN_TRIGGER         18
#define CAL_NOISE_MS            3000
#define CAL_POLARITY_MS         6000
#define CAL_LEARN_TIMEOUT_MS    60000
#define CAL_TARGET_DROPS        6      // เก็บให้ครบเท่านี้แล้วจบ (เดิม 10 ทำให้ขั้นที่ 2 นาน)
#define CAL_MIN_DROPS           3      // ขั้นต่ำที่ยอมรับได้
#define CAL_MAX_SAMPLES         8
#define CAL_QUICK_DROPS         3      // ได้เท่านี้และความสูงใกล้เคียงกัน = จบทันที
#define CAL_QUICK_SPREAD_PCT    40

// ---- การเรียนรู้รูปร่างพัลส์ต่อเนื่องหลังคาลิเบรต ----
#define LEARN_DIV               8      // ความเร็วที่ค่าที่เรียนรู้ไล่ตามหยดใหม่ (มาก = ช้า)
#define LEARN_MIN_AMP_PCT       35     // พัลส์ที่เตี้ยกว่านี้ (% ของที่เรียนรู้) = ไม่ใช่หยด
#define LEARN_MAX_WIDTH_MUL     3      // พัลส์ที่กว้างกว่าที่เรียนรู้เกินเท่านี้ = ไม่ใช่หยด
#define LEARN_MAX_RISE_MUL      2      // ขอบขาขึ้นช้ากว่าที่เรียนรู้เกินเท่านี้ = ถุงแกว่ง ไม่ใช่หยด
#define MOTION_WINDOW_MS        200    // คาบตรวจว่าเส้นฐานกำลังเคลื่อน (ถุงแกว่ง/คนไข้เดิน)
#define MOTION_HOLD_MS          2000   // ถือว่ายังเคลื่อนอยู่อีกเท่านี้หลังตรวจพบ
#define CAL_VERIFY_MS           20000
#define SCOPE_TOP               82

// ----------------------------------------------------------------------------
// ค่าคงที่ของระบบสื่อสารและการแจ้งเตือน (ต้องตรงกับ Host)
// ----------------------------------------------------------------------------
#define DEFAULT_ESPNOW_CHANNEL  1
#define MAX_WIFI_CHANNEL        13
#define HOST_TIMEOUT_MS         5000
#define CHANNEL_SCAN_DWELL_MS   2500     // Host ส่ง Sync ทุก 1 วินาที -> รอแต่ละช่อง 2.5 วินาที
#define CHANNEL_SCAN_START_MS   6000     // หลังบูตรอช่องที่จำไว้ก่อน 6 วินาที
#define SEND_INTERVAL_MS        1000     // คาบพื้นฐานของการส่งข้อมูลให้ Host
#define SEND_JITTER_MS          60       // สุ่มบวกลบรอบคาบ กันสองสเตชันส่งชนกันค้างนาน
                                         // (ดูคำอธิบายเหตุผลในหัวไฟล์ V7.5.3)
#define MIN_OCCLUSION_MS        8000
#define MAX_OCCLUSION_MS        300000
#define SCREENSAVER_IDLE_MS     180000
#define SNOOZE_MS               120000
#define NEAR_END_REMIND_MS      300000   // เตือนใกล้หมดซ้ำทุก 5 นาทีจนกว่าจะรับทราบ
#define DEFAULT_NEAR_END_PCT    80

#define ALERT_NONE        0
#define ALERT_TOO_FAST    1
#define ALERT_TOO_SLOW    2
#define ALERT_NEAR_END    3   // ให้ไปแล้วถึง % ที่ตั้ง (เตรียมถุงใหม่)
#define ALERT_COMPLETE    4
#define ALERT_OCCLUSION   5

#define THEME_BG            0x0000
#define THEME_SKYBLUE       0x5DFF
#define THEME_CYAN          0x07FF
#define THEME_PINK          0xFD79
#define THEME_NAVY          0x09CD
#define THEME_DARKBOX       0x18C5
#define THEME_MAROON        0x6000

#define COLOR_WHITE         0xFFFF
#define COLOR_GREEN         0x07E0
#define COLOR_RED           0xF800
#define COLOR_ORANGE        0xFD20
#define COLOR_YELLOW        0xFFE0

// ชุดสีของหน้าจอรุ่นใหม่ (พื้นดำ การ์ดเทาเข้ม ตัวอักษรรองสีเทา)
#define UI_CARD             0x18C5   // พื้นการ์ด
#define UI_BAR              0x0A49   // แถบบนสุด
#define UI_LINE             0x39E7   // เส้นคั่น/แถบว่าง
#define UI_DIM              0x8410   // ข้อความรอง
#define UI_GREEN            0x2FEB   // เขียวอ่านง่ายบนพื้นดำ

#ifndef ST77XX_LIGHTGREY
  #define ST77XX_LIGHTGREY  0xC618
#endif
#ifndef ST77XX_DARKGREY
  #define ST77XX_DARKGREY   0x39E7
#endif

SPIClass SPI_TFT(FSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(&SPI_TFT, TFT_CS, TFT_DC, TFT_RST);
Preferences stationPrefs;

enum AppState {
  STATE_NORMAL_VIEW,
  STATE_EMERGENCY,
  STATE_SCREENSAVER,
  STATE_CREDIT,
  STATE_CONFIG_ID,
  STATE_NEAR_END_NOTICE
};

// ---- สถานะที่ใช้เลือกสีและคำบนจอ ----
// ต้องประกาศไว้ตอนต้นไฟล์ เพราะ Arduino IDE แทรก prototype ของฟังก์ชันไว้ก่อนส่วนแสดงผล
enum UiStatus { UI_OK = 0, UI_FAST, UI_SLOW, UI_NOFLOW, UI_NEAREND, UI_DONE, UI_PAUSED };

AppState currentState       = STATE_NORMAL_VIEW;
int currentNursePage        = 1;
uint8_t currentStationId    = 1;
uint8_t tempConfigStationId = 1;
uint8_t screenRotation      = TFT_ROTATION_DEFAULT;   // 0 = ปกติ, 2 = กลับหัว 180 องศา
unsigned long configAutoSaveTimeout = 0;
unsigned long lastUserActivityTime  = 0;

bool isRunning              = true;
bool isClockSynced          = false;
volatile bool isHostOnline  = false;
volatile int lastHostRssi   = -100;
volatile unsigned long lastHostRecvTime = 0;
bool stateNeedsRedraw       = true;

// ---- สัญญาณและการตรวจจับหยด (ประกาศชนิดไว้ตอนต้นไฟล์ เพราะ Arduino แทรก prototype ก่อนฟังก์ชันแรก) ----
enum DropPhase { DP_IDLE, DP_ACTIVE, DP_REARM };

int8_t  dropPolarity     = -1;                       // -1 = หยดทำให้ค่า ADC ลดลง, +1 = เพิ่มขึ้น
int     dropTriggerDelta = CAL_DEFAULT_TRIGGER;      // เบนจากเส้นฐานเท่านี้ = เริ่มนับว่าเป็นพัลส์
int     dropReleaseDelta = CAL_DEFAULT_TRIGGER * 2 / 5;  // ต่ำกว่านี้ = พัลส์จบ (ฮิสเทอรีซิส)
int     calDropAmp       = 0;                        // ความสูงพัลส์กลางจากตอนคาลิเบรต
int     calNoisePp       = 0;                        // สัญญาณรบกวนยอดถึงยอดตอนคาลิเบรต
uint8_t calSnrX10        = 0;                        // อัตราส่วนสัญญาณต่อสัญญาณรบกวน คูณ 10
uint8_t calQuality       = 0;                        // 0 = ยังไม่คาลิเบรต/อ่อนเกินไป ... 4 = ดีมาก
int     lastDropPeak     = 0;
int     lastDropWidthMs  = 0;
int     lastDropRiseMs   = 0;
int     learnAmp         = 0;    // ความสูงพัลส์ที่เครื่องเรียนรู้ไว้ (0 = ยังไม่รู้จักหยด)
int     learnWidth       = 0;    // ความกว้างพัลส์ที่เรียนรู้ไว้ (ms)
bool    shapeGateEnabled = true; // ปิดชั่วคราวระหว่างขั้นเรียนรู้ในหน้าคาลิเบรต

// ----------------------------------------------------------------------------
// โครงสร้างข้อมูลรับ-ส่ง ESP-NOW (Protocol v2) — ต้องเหมือนกับ Host ทุกไบต์
// ----------------------------------------------------------------------------
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

struct_message myData;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint32_t totalDrops                 = 0;
uint32_t periodDropsCounter         = 0;

float currentFlowRate_ml_hr         = 0.0;
float currentGttMin                 = 0.0;
float totalVolumeMl                 = 0.0;

// ค่าจาก Host (0 = Host ยังไม่ได้ตั้งค่า)
volatile float targetRateHr         = 0.0;
volatile float totalPlanMl          = 0.0;
volatile uint8_t hostAlertCode      = ALERT_NONE;
volatile uint8_t dropFactor         = 20;
volatile uint8_t hostReportedChannel = 0;
volatile bool pendingCounterReset   = false;
volatile uint8_t nearEndPct         = DEFAULT_NEAR_END_PCT;
volatile bool hostNearEndAck        = false;   // Host ยืนยันว่ารับทราบแล้ว
bool nearEndAckRequest              = false;   // กดรับทราบที่เตียง รอ Host ยืนยัน
unsigned long nearNextChimeTime     = 0;
uint8_t nearChimeRemaining          = 0;
int16_t knownResetSeq               = -1;

uint8_t espnowChannel               = DEFAULT_ESPNOW_CHANNEL;
uint8_t savedChannel                = DEFAULT_ESPNOW_CHANNEL;
unsigned long lastChannelHopTime    = 0;

unsigned long lastDropTimestamp     = 0;
bool hasFirstDropOccurred           = false;
bool skipNextInterval               = false;

unsigned long lastSendTime          = 0;
unsigned long nextSendDelay         = SEND_INTERVAL_MS;   // คาบของรอบถัดไป (สุ่มใหม่ทุกครั้ง)
unsigned long lastDebounceTime      = 0;
unsigned long snoozeUntilTime       = 0;
const unsigned long minDropInterval = 55;

unsigned long buzzerBeepUntil       = 0;
unsigned long buzzerNextBeepTime    = 0;

bool isDropFalling                  = false;
unsigned long dropFallStartTime     = 0;
int animDropY                       = 190;
int animDropPrevY                   = 190;
int8_t animModeDrawn                = -1;
unsigned long lastAnimFrame         = 0;
unsigned long poolFlashUntil        = 0;
const int CHAMBER_NOZZLE_Y          = 190;   // ตำแหน่งปลายหยดในกระเปาะ (หน้าจอใหม่)
const int CHAMBER_POOL_Y            = 204;   // ผิวน้ำในกระเปาะ
const unsigned long DROP_FALL_DURATION = 160;

bool isHoldUiActive                 = false;
bool holdBeep3sDone                 = false;
bool holdBeep5sDone                 = false;
unsigned long lastHoldRenderTime    = 0;

// ---- รหัสเตือนที่หน้าจอใช้ (อัปเดตทุกรอบใน loop) ----
uint8_t uiAlertCode = ALERT_NONE;

// ---- แคชของหน้าจอ: วาดใหม่เฉพาะเมื่อค่าที่แสดงเปลี่ยนจริง ----
String cacheClock = "";
int    cacheBars = -9999;
int    cacheStatusBar = -1;
bool   cacheBedChip = false;
int    cacheRate = -9999, cacheTarget = -9999, cacheLeftMl = -9999;
int    cachePct = -999, cacheBagPct = -999;
String cacheTimeLeft = "", cacheEndClock = "", cacheStatusWord = "";
String cacheP2a = "", cacheP2b = "", cacheP2c = "", cacheP2d = "", cacheP2foot = "";
String cacheP3a = "", cacheP3b = "", cacheP3c = "";
String cacheP4 = "", cacheNear = "", cacheConfig = "";
uint8_t lastEmergencyCode = 255;

// ---- กราฟแนวโน้มอัตราไหล: 30 แท่ง แท่งละ 1 นาที = ย้อนหลัง 30 นาที ----
#define TREND_BARS   30
#define TREND_BIN_MS 60000UL
float    trendBin[TREND_BARS];
bool     trendHas[TREND_BARS];
int      trendHead      = 0;
float    trendSum       = 0.0f;
int      trendSamples   = 0;
float    trendLive      = 0.0f;
float    trendScale     = 100.0f;
bool     trendNeedsRedraw = true;
unsigned long trendBinStart   = 0;
unsigned long trendLastSample = 0;

// ---- ประกาศล่วงหน้า (ฟังก์ชันที่ถูกเรียกก่อนบรรทัดที่นิยามไว้) ----
// ฟังก์ชันวาดจอที่ย้ายไป StationScreen.h แล้ว แต่ถูกเรียกจากโค้ดเหนือบรรทัด include
void drawTopBar(bool force);
void drawProgressBar(int x, int y, int w, int h, int pct, uint16_t fill);
void drawStatCard(int y, const char* label, const String &value, uint16_t valueColor, bool labelOnly);
void drawTrendChart();
void drawTrendLiveBar();
void applyScreenRotation(uint8_t rot);
void beepNonBlocking(uint16_t freq, uint16_t durationMs);
void textAt(const String &s, int x, int y, uint8_t size, uint16_t col, uint16_t bg);
String padTo(const String &s, unsigned int width);
void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg);
void resetTrendData();
void updateTrendAccumulator(unsigned long now);

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

// สูตรเดียวกับ Host
uint32_t occlusionThresholdMs() {
  float rate = targetRateHr;
  if (rate <= 0.0f) return MIN_OCCLUSION_MS;
  float dropsPerHr = rate * (float)safeDropFactor(dropFactor);
  uint32_t expected = (uint32_t)(3600000.0f / dropsPerHr);
  uint32_t th = (expected * 5) / 2;
  if (th < MIN_OCCLUSION_MS) th = MIN_OCCLUSION_MS;
  if (th > MAX_OCCLUSION_MS) th = MAX_OCCLUSION_MS;
  return th;
}

bool isLocallyOccluded() {
  return isRunning && hasFirstDropOccurred && (millis() - lastDropTimestamp > occlusionThresholdMs());
}

uint8_t safeNearPct(uint8_t p) {
  return (p >= 50 && p <= 95) ? p : DEFAULT_NEAR_END_PCT;
}

bool isNearEndReached() {
  if (totalPlanMl <= 0) return false;
  return totalVolumeMl >= totalPlanMl * (float)safeNearPct(nearEndPct) / 100.0f;
}

bool isNearEndAcknowledged() {
  return hostNearEndAck || nearEndAckRequest;
}

// รหัสเตือนที่ใช้จริงในเครื่องนี้
uint8_t effectiveAlertCode() {
  if (!isRunning) return ALERT_NONE;
  if (isLocallyOccluded()) return ALERT_OCCLUSION;           // ตัดสินทันทีในเครื่อง
  if (isHostOnline) {
    uint8_t c = hostAlertCode;
    if (c == ALERT_OCCLUSION) c = ALERT_NONE;                // ข้อมูลในเครื่องใหม่กว่า
    return c;
  }
  if (totalPlanMl > 0 && totalVolumeMl >= totalPlanMl) return ALERT_COMPLETE;
  if (isNearEndReached()) return ALERT_NEAR_END;
  return ALERT_NONE;
}

bool isNearEndPending(uint8_t code) {
  return code == ALERT_NEAR_END && !isNearEndAcknowledged();
}

void acknowledgeNearEnd() {
  nearEndAckRequest = true;
  nearChimeRemaining = 0;
  noTone(BUZZER_PIN);
  beepNonBlocking(2600, 40);
}

float readStationBattery() {
#if STATION_BAT_ADC_PIN >= 0
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(STATION_BAT_ADC_PIN);
  float v = ((sum / 8.0f) / 4095.0f) * 3.3f * BAT_DIVIDER_RATIO;
  return (v < 2.5f) ? 0.0f : v;
#else
  return 0.0f;
#endif
}

void drawCenteredString(const String &str, int y, uint8_t size, uint16_t color, uint16_t bg) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(size);
  tft.getTextBounds(str, 0, y, &x1, &y1, &w, &h);
  int x = (172 - (int)w) / 2;
  if (x < 0) x = 0;
  tft.setTextColor(color, bg);
  tft.setCursor(x, y);
  tft.print(str);
}

void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  neopixelWrite(RGB_LED_PIN, r, g, b);
}

void beepNonBlocking(uint16_t freq, uint16_t durationMs) {
  tone(BUZZER_PIN, freq);
  buzzerBeepUntil = millis() + durationMs;
}

void modalSafeBeep(uint16_t freq, uint16_t durationMs) {
  tone(BUZZER_PIN, freq);
  delay(durationMs);
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;
}

void soundClick()       { beepNonBlocking(2600, 20); }
void soundStart()       { beepNonBlocking(2000, 70); }
void soundStop()        { beepNonBlocking(1000, 140); }
void soundScreensaver() { beepNonBlocking(1800, 40); }

void playWelcomeMelody() {
  int notes[] = { 1046, 1318, 1568, 2093 };
  for (int i = 0; i < 4; i++) {
    modalSafeBeep(notes[i], 80);
    delay(25);
  }
}

void handlePassiveBuzzerEngine(uint8_t code) {
  unsigned long now = millis();
  if (buzzerBeepUntil > 0 && now >= buzzerBeepUntil) {
    noTone(BUZZER_PIN);
    buzzerBeepUntil = 0;
  }

  // เสียงเตือนใกล้หมด: 3 ครั้งสั้น ๆ แล้วเงียบ 5 นาที (ดังเฉพาะเมื่อยังไม่มีเหตุวิกฤต)
  if (!isCriticalAlert(code)) {
    if (!isNearEndPending(code)) { nearChimeRemaining = 0; nearNextChimeTime = 0; return; }
    if (nearChimeRemaining == 0 && (nearNextChimeTime == 0 || now >= nearNextChimeTime)) {
      nearChimeRemaining = 3;
      buzzerNextBeepTime = now;
      nearNextChimeTime = now + NEAR_END_REMIND_MS;
    }
    if (nearChimeRemaining > 0 && now >= buzzerNextBeepTime) {
      beepNonBlocking(2000, 70);
      buzzerNextBeepTime = now + 220;
      nearChimeRemaining--;
    }
    return;
  }

  if (now < snoozeUntilTime) return;
  if (now < buzzerNextBeepTime) return;

  switch (code) {
    case ALERT_OCCLUSION: beepNonBlocking(1800, 150); buzzerNextBeepTime = now + 900;  break;
    case ALERT_TOO_FAST:  beepNonBlocking(2200, 80);  buzzerNextBeepTime = now + 700;  break;
    case ALERT_TOO_SLOW:  beepNonBlocking(1200, 120); buzzerNextBeepTime = now + 1200; break;
    case ALERT_COMPLETE:  beepNonBlocking(1500, 250); buzzerNextBeepTime = now + 1500; break;
  }
}

void updateStatusLed(uint8_t code) {
  static int lastLed = -1;
  int led;
  if (currentState == STATE_CONFIG_ID) led = 4;
  else if (isCriticalAlert(code) && millis() >= snoozeUntilTime) led = 0;
  else if (isNearEndPending(code)) led = 5;
  else if (!isRunning) led = 1;
  else if (!isHostOnline) led = 2;
  else led = 3;

  if (led == lastLed) return;
  lastLed = led;
  switch (led) {
    case 0: setLedColor(90, 0, 0);  break;
    case 1: setLedColor(60, 40, 0); break;
    case 2: setLedColor(0, 0, 60);  break;
    case 3: setLedColor(0, 30, 0);  break;
    case 4: setLedColor(50, 0, 50); break;
    case 5: setLedColor(90, 35, 0); break;
  }
}

void drawHoldProgressHUD(unsigned long holdDur) {
  int boxX = 6, boxY = 100, boxW = 160, boxH = 100;
  tft.fillRoundRect(boxX, boxY, boxW, boxH, 8, THEME_NAVY);
  tft.drawRoundRect(boxX, boxY, boxW, boxH, 8, THEME_CYAN);

  int barX = boxX + 15;
  int barY = boxY + 48;
  int barW = 130;
  int barH = 12;

  if (holdDur < 3000) {
    int fill = map(holdDur, 400, 3000, 0, barW);
    if (fill < 0) fill = 0;
    if (fill > barW) fill = barW;

    drawCenteredString("BUTTON HOLDING", boxY + 12, 1, THEME_SKYBLUE, THEME_NAVY);
    drawCenteredString("-> CALIBRATE (3s)", boxY + 28, 1, COLOR_YELLOW, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, fill, barH - 2, THEME_PINK);
    drawCenteredString("Release <3s = Cancel", boxY + 82, 1, ST77XX_LIGHTGREY, THEME_NAVY);
  }
  else if (holdDur < 5000) {
    int fill = map(holdDur, 3000, 5000, 0, barW);
    if (fill < 0) fill = 0;
    if (fill > barW) fill = barW;

    drawCenteredString("RELEASE = CALIB", boxY + 12, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("-> CREDITS (5s)", boxY + 28, 1, THEME_PINK, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, fill, barH - 2, THEME_CYAN);
  }
  else {
    drawCenteredString(">> CREDITS READY <<", boxY + 14, 1, COLOR_GREEN, THEME_NAVY);
    drawCenteredString("RELEASE TO VIEW", boxY + 30, 1, COLOR_YELLOW, THEME_NAVY);
    tft.drawRect(barX, barY, barW, barH, COLOR_WHITE);
    tft.fillRect(barX + 1, barY + 1, barW - 2, barH - 2, COLOR_GREEN);
  }
}

// ============================================================================
// เครื่องตรวจจับหยด v7.5.1 — ตัวกรอง + เส้นฐานปรับอัตโนมัติ + จับเป็น "เหตุการณ์"
// ----------------------------------------------------------------------------
// ปัญหาของ v7.5.0 ที่แก้ตรงนี้
//  1) นับเบิ้ล: โค้ดเดิมเมื่อค่าต่ำกว่าเกณฑ์แต่ยังไม่พ้นเวลากันรัว จะ "ไม่นับ
//     แต่ก็ไม่เปลี่ยนสถานะ" ตัวจับจึงยังง้างอยู่ พอพ้น 55 ms ขณะที่สัญญาณยัง
//     ต่ำอยู่ ก็นับซ้ำจากหยดเดิมทันที (และนับอีกทุก ๆ 55 ms ที่ค้างอยู่)
//     -> ของใหม่นับตอน "พัลส์จบ" ที่เดียว และต้องกลับมานิ่งใต้เกณฑ์ปลดก่อน
//        จึงจะรับหยดถัดไปได้ 1 พัลส์ = 1 หยด เสมอ
//  2) หาช่วงต่างยาก: โค้ดเดิมตั้งเกณฑ์จากค่ากึ่งกลางระหว่าง "กระเปาะมีน้ำ" กับ
//     "กระเปาะเปล่า" ซึ่งเป็นค่านิ่งสองค่า แต่หยดที่ตกผ่านลำแสงเป็นสัญญาณชั่วขณะ
//     ที่เบนจากค่าปกติเพียงบางส่วน เกณฑ์กึ่งกลางจึงสูงเกินจะจับหยดได้
//     -> ของใหม่วัดจาก "ระยะเบนจากเส้นฐาน" ที่ไล่ตามค่าปกติเองตลอดเวลา
//        และตั้งเกณฑ์จากหยดจริงที่วัดได้ในขั้นตอนคาลิเบรต
// ============================================================================

int  sensorRaw          = 0;      // ค่าดิบล่าสุดจาก ADC
int  sensorFiltered     = 0;      // หลังผ่านมีเดียน (ตัดสไปก์เดี่ยว)
long baselineAcc        = 0;      // เส้นฐาน เก็บคูณ 256 ไว้ให้ละเอียด
int  sensorBaseline     = 0;      // เส้นฐาน = ค่าปกติตอนไม่มีหยด
int  sensorDelta        = 0;      // ระยะเบนจากเส้นฐานในทิศที่แปลว่า "มีหยด"
int  sensorNoisePp      = 0;      // สัญญาณรบกวนยอดถึงยอดของช่วงที่ไม่มีหยด
long deltaSlowAcc       = 0;      // ตัวตามค่าเบนแบบช้า (คูณ 256)
int  deltaSlow          = 0;
int  deltaEdge          = 0;      // ส่วนที่ไต่ขึ้นเร็วของสัญญาณ = สิ่งที่ตัวตรวจจับใช้จริง
bool sensorReady        = false;

int medBuf[SENSOR_MEDIAN_N];
uint8_t medIdx = 0;
uint8_t medFill = 0;

int  noiseMin = 4095, noiseMax = 0;
unsigned long noiseWindowStart = 0;

DropPhase dropPhase       = DP_IDLE;
unsigned long dropPhaseMs = 0;    // เวลาที่เข้าเฟสปัจจุบัน
unsigned long dropSettleMs = 0;   // ครั้งล่าสุดที่สัญญาณนิ่งใต้เกณฑ์ปลด
unsigned long dropRiseMs  = 0;    // เวลาที่ใช้ไต่จากเกณฑ์เข้าถึงยอดพัลส์
uint32_t baselineRecovers = 0;    // จำนวนครั้งที่ต้องยึดเส้นฐานใหม่เพราะระดับเลื่อนค้าง
uint32_t completedPulses  = 0;    // จำนวนพัลส์ที่ผ่านเกณฑ์ทั้งหมด (ให้หน้าคาลิเบรตอ่าน)
uint32_t sampleGaps       = 0;    // จำนวนครั้งที่การอ่านขาดช่วงนานผิดปกติ

// ---- ตรวจว่าเส้นฐานกำลังเคลื่อน (ถุงน้ำเกลือแกว่งตอนคนไข้เดิน) ----
int  baselineRef          = 0;
unsigned long motionCheckMs = 0;
unsigned long motionUntil = 0;
int  motionLevel          = 0;
bool inMotion             = false;
int  dropPeakDelta        = 0;
uint32_t lastSampleUs     = 0;
uint32_t rejectedPulses   = 0;    // พัลส์ที่คัดทิ้ง (แคบหรือกว้างผิดปกติ)

// อ่านหลายครั้งแล้วเฉลี่ย ใช้ตั้งเส้นฐานตอนเริ่ม เพื่อไม่ให้ตัวอย่างเดียวที่บังเอิญ
// ตกบนยอดสัญญาณรบกวนกลายเป็นเส้นฐานที่เพี้ยนไปทั้งรอบ
int readSensorAverage(int n) {
  long sum = 0;
  for (int i = 0; i < n; i++) { sum += analogRead(SENSOR_AO_PIN); delayMicroseconds(200); }
  return (int)(sum / n);
}

void resetSignalChain(int seed) {
  for (int i = 0; i < SENSOR_MEDIAN_N; i++) medBuf[i] = seed;
  medIdx = 0; medFill = SENSOR_MEDIAN_N;
  sensorFiltered = seed;
  baselineAcc = (long)seed * 256;
  sensorBaseline = seed;
  sensorDelta = 0;
  deltaSlowAcc = 0;
  deltaSlow = 0;
  deltaEdge = 0;
  dropPhase = DP_REARM;
  dropPhaseMs = millis();
  dropSettleMs = dropPhaseMs;
  dropPeakDelta = 0;
  noiseMin = seed; noiseMax = seed;
  noiseWindowStart = millis();
  sensorReady = true;
}

int pushMedian(int v) {
  medBuf[medIdx] = v;
  medIdx = (uint8_t)((medIdx + 1) % SENSOR_MEDIAN_N);
  if (medFill < SENSOR_MEDIAN_N) medFill++;
  int t[SENSOR_MEDIAN_N];
  memcpy(t, medBuf, sizeof(t));
  for (int i = 1; i < SENSOR_MEDIAN_N; i++) {         // เรียงแบบแทรก (N=5 เร็วกว่า qsort)
    int k = t[i], j = i - 1;
    while (j >= 0 && t[j] > k) { t[j + 1] = t[j]; j--; }
    t[j + 1] = k;
  }
  return t[SENSOR_MEDIAN_N / 2];
}

// delta > 0 = สัญญาณเบนไปทางที่แปลว่า "มีหยดผ่านลำแสง"
// dropPolarity -1 = หยดทำให้ค่าลดลง (ค่าเริ่มต้น), +1 = หยดทำให้ค่าเพิ่มขึ้น
inline int deltaFromBaseline(int filtered) {
  return (dropPolarity < 0) ? (sensorBaseline - filtered) : (filtered - sensorBaseline);
}

// ปรับเกณฑ์ตามความสูงพัลส์ที่เรียนรู้ไว้ แต่ห้ามต่ำกว่าพื้นสัญญาณรบกวน
void adaptTriggerFromLearning() {
  if (learnAmp <= 0) return;
  int t = (learnAmp * 45) / 100;
  int floorN = calNoisePp * 3;
  if (floorN < CAL_MIN_TRIGGER) floorN = CAL_MIN_TRIGGER;
  if (t < floorN) t = floorN;
  dropTriggerDelta = t;
  dropReleaseDelta = t * 2 / 5;
  if (dropReleaseDelta < 2) dropReleaseDelta = 2;
}

// เรียนรู้จากหยดที่เพิ่งผ่านเกณฑ์ ทำต่อเนื่องตลอดการใช้งาน ไม่ใช่เฉพาะตอนคาลิเบรต
// จึงตามการเปลี่ยนแปลงช้า ๆ ได้เอง (ฝุ่นเกาะ ไฟอ่อนลง ท่าทางของสายเปลี่ยน)
void learnFromPulse(int peak, int width) {
  if (learnAmp <= 0) { learnAmp = peak; learnWidth = width; }
  else {
    learnAmp   += (peak  - learnAmp)  / LEARN_DIV;
    learnWidth += (width - learnWidth) / LEARN_DIV;
  }
  if (learnWidth < DROP_MIN_WIDTH_MS) learnWidth = DROP_MIN_WIDTH_MS;
  adaptTriggerFromLearning();
}

// พัลส์นี้ "หน้าตาเหมือนหยด" ที่เครื่องรู้จักหรือไม่
// ใช้คัดจังหวะที่ถุงน้ำเกลือแกว่งตอนคนไข้เดิน ซึ่งทำให้สัญญาณเบนแรงแต่ไต่ขึ้นช้า
// และค้างนานกว่าหยดจริงหลายเท่า
bool pulseLooksLikeDrop(int peak, int width, int riseMs) {
  if (width < DROP_MIN_WIDTH_MS || width > DROP_MAX_WIDTH_MS) return false;
  if (!shapeGateEnabled || learnAmp <= 0) return true;

  (void)riseMs;
  if (peak < (learnAmp * LEARN_MIN_AMP_PCT) / 100) return false;      // จางเกินกว่าจะเป็นหยด

  int wMax = learnWidth * LEARN_MAX_WIDTH_MUL + 20;
  if (width > wMax) return false;                                     // ค้างนานผิดรูปหยด
  return true;
}


// ----------------------------------------------------------------------------
// หน้าต่างคำนวณอัตราการไหล (ใหม่ใน v7.5.4)
// ----------------------------------------------------------------------------
// เดิมใช้ค่าเฉลี่ยถ่วงน้ำหนักของ "อัตราขณะนั้น" (60000/dt) ซึ่งเป็นการเฉลี่ย
// ส่วนกลับของเวลา ตามอสมการเจนเซนจะให้ค่าสูงกว่าความจริงเสมอ และแกว่งแรงตาม
// ความไม่สม่ำเสมอตามธรรมชาติของหยด จนหลุดกรอบ +-10% ที่คู่มือใช้คาลิเบรต
// (ตัวอย่าง: หยดสลับ 1.0/1.4 วินาที ของจริง 150 mL/h จอแกว่ง 139-170 mL/h)
//
// ใหม่: นับจำนวนหยดที่ตกจริงในหน้าต่าง แล้วหารด้วยเวลาที่ใช้ไปทั้งหน้าต่าง
//   mL/h = (จำนวนหยดในหน้าต่าง / Drop Factor) x 3,600,000 / ช่วงเวลา(ms)
// เป็นวิธีเดียวกับที่พยาบาลนับหยด 1 นาทีตามคู่มือ และเดียวกับที่ Host บันทึก
// ลง Log รายนาที ตัวเลขทั้งสามจุดจึงตรงกันโดยนิยาม และไม่มีค่าเอน
#define RATE_WINDOW_DROPS   8

unsigned long rateWinMs[RATE_WINDOW_DROPS] = {0};
uint8_t       rateWinIdx   = 0;
uint8_t       rateWinCount = 0;

void resetRateWindow() {
  rateWinIdx   = 0;
  rateWinCount = 0;
}

void pushRateSample(unsigned long t) {
  rateWinMs[rateWinIdx] = t;
  rateWinIdx = (uint8_t)((rateWinIdx + 1) % RATE_WINDOW_DROPS);
  if (rateWinCount < RATE_WINDOW_DROPS) rateWinCount++;
}

// ช่วงเวลาทั้งหน้าต่าง (ms) — 0 = ยังมีตัวอย่างไม่พอ
unsigned long rateWindowSpanMs() {
  if (rateWinCount < 2) return 0;
  uint8_t oldest = (uint8_t)((rateWinIdx + RATE_WINDOW_DROPS - rateWinCount) % RATE_WINDOW_DROPS);
  uint8_t newest = (uint8_t)((rateWinIdx + RATE_WINDOW_DROPS - 1) % RATE_WINDOW_DROPS);
  return rateWinMs[newest] - rateWinMs[oldest];
}

// ช่วงห่างเฉลี่ยระหว่างหยดในหน้าต่าง (ms) — 0 = ยังคำนวณไม่ได้
unsigned long rateWindowMeanIntervalMs() {
  unsigned long span = rateWindowSpanMs();
  if (span == 0) return 0;
  return span / (unsigned long)(rateWinCount - 1);
}

// อัตราการไหลจากหน้าต่าง (mL/h) — 0 = ยังคำนวณไม่ได้
// N ตัวอย่างเวลา = N-1 ช่วง = มีหยดตกจริง N-1 หยดในช่วงเวลานั้น
float rateFromWindow(uint8_t df) {
  unsigned long span = rateWindowSpanMs();
  if (span == 0 || df == 0) return 0.0f;
  float dropsInSpan = (float)(rateWinCount - 1);
  return (dropsInSpan / (float)df) * 3600000.0f / (float)span;
}

// บันทึกหยด 1 หยด — eventMs คือเวลาที่พัลส์ "เริ่ม" ซึ่งเป็นจังหวะจริงที่หยดผ่านลำแสง
void registerDrop(unsigned long eventMs) {
  totalDrops++;
  periodDropsCounter++;

  isDropFalling = true;
  dropFallStartTime = millis();
  animDropY = CHAMBER_NOZZLE_Y;

  uint8_t df = safeDropFactor(dropFactor);

  // ---- สะสมต่อในหน้าต่างเดิม หรือเริ่มหน้าต่างใหม่? ----
  if (!hasFirstDropOccurred || skipNextInterval) {
    resetRateWindow();            // เริ่มถุงใหม่/กลับจากหยุดชั่วคราว: ช่วงที่ขาดหายไม่ใช่การไหล
  } else {
    unsigned long dropDeltaMs = eventMs - lastDropTimestamp;
    unsigned long meanIv      = rateWindowMeanIntervalMs();
    if (dropDeltaMs <= minDropInterval || dropDeltaMs >= MAX_OCCLUSION_MS) {
      resetRateWindow();          // ช่วงหยดผิดปกติ ไม่เอามาปนกับของเดิม
    } else if (meanIv > 0 && (dropDeltaMs > meanIv * 3 || dropDeltaMs * 3 < meanIv)) {
      resetRateWindow();          // อัตราเปลี่ยนก้าวกระโดด (หมุนโรลเลอร์แคลมป์)
                                  // ทิ้งค่าเก่าทันที ไม่ให้ถ่วงจนตรวจจับช้า
    }
  }

  pushRateSample(eventMs);
  float windowRate = rateFromWindow(df);
  if (windowRate > 0.0f) currentFlowRate_ml_hr = windowRate;

  hasFirstDropOccurred = true;
  skipNextInterval = false;
  lastDropTimestamp = eventMs;
  currentGttMin = (currentFlowRate_ml_hr * (float)df) / 60.0f;
  totalVolumeMl = (float)totalDrops / (float)df;
}

// อ่านเซนเซอร์ 1 ครั้งตามคาบที่กำหนด แล้วเดินเครื่องจักรสถานะ
// countDrops = false ใช้ตอนคาลิเบรต/หยุดชั่วคราว (ยังอัปเดตเส้นฐานและสัญญาณรบกวนตามปกติ)
// คืนค่า true เมื่อเพิ่ง "จบพัลส์ที่ผ่านเกณฑ์" ในรอบนี้
bool serviceDropSensor(bool countDrops) {
  uint32_t nowUs = micros();
  uint32_t dtUs  = nowUs - lastSampleUs;
  if (dtUs < SENSOR_SAMPLE_US) return false;
  lastSampleUs = nowUs;

  sensorRaw = analogRead(SENSOR_AO_PIN);

  if (!sensorReady) { resetSignalChain(sensorRaw); return false; }

  // อ่านขาดช่วงนาน (เช่นเพิ่งวาดจอทั้งหน้า) — ทิ้งเฉพาะพัลส์ที่กำลังดำเนินอยู่ เพราะมองเห็นไม่ครบ
  // แต่ต้อง "เก็บเส้นฐานเดิมไว้" เพราะเป็นค่าที่สะสมมานาน ถ้าตั้งต้นใหม่ทุกครั้งที่วาดจอ
  // ตัวตรวจจับจะถูกดีดกลับไปเฟสรอนิ่งเรื่อย ๆ จนแทบไม่มีจังหวะเฝ้ารอหยดเลย
  if (dtUs > (uint32_t)SENSOR_GAP_RESYNC_MS * 1000UL) {
    for (int i = 0; i < SENSOR_MEDIAN_N; i++) medBuf[i] = sensorRaw;
    sensorFiltered = sensorRaw;
    if (dropPhase == DP_ACTIVE) rejectedPulses++;
    dropPhase = DP_REARM;
    dropPhaseMs = millis();
    dropSettleMs = dropPhaseMs;
    sampleGaps++;
    return false;
  }

  sensorFiltered = pushMedian(sensorRaw);
  sensorBaseline = (int)(baselineAcc / 256);
  sensorDelta    = deltaFromBaseline(sensorFiltered);

  // อัตราการไล่ตามคิดจากเวลาที่ผ่านไปจริง (หน่วย 100 us) ไม่ใช่จำนวนตัวอย่าง
  long tick = (long)(dtUs / 100);
  if (tick < 1)   tick = 1;
  if (tick > 200) tick = 200;

  // แยก "ส่วนที่ไต่ขึ้นเร็ว" ออกจาก "ส่วนที่ค่อย ๆ เลื่อน"
  // ถุงน้ำเกลือแกว่งตอนคนไข้เดินทำให้สัญญาณส่ายเป็นจังหวะหลายร้อยมิลลิวินาที
  // ตัวตามช้าจะไล่ตามส่วนนั้นได้ทัน แล้วหักออกจนเหลือศูนย์
  // ส่วนหยดที่กว้างแค่ราว 10 ms ตัวตามช้าตามไม่ทัน จึงเหลือเป็นยอดแหลมให้จับ
  // (หยุดไล่ตามระหว่างที่พัลส์กำลังดำเนินอยู่ ไม่งั้นจะกลืนหยดหายไปเอง)
  if (dropPhase != DP_ACTIVE) {
    long tau = inMotion ? DELTA_SLOW_TAU_MOTION : DELTA_SLOW_TAU;
    deltaSlowAcc += ((long)sensorDelta * 256 - deltaSlowAcc) * tick / tau;
  }
  deltaSlow = (int)(deltaSlowAcc / 256);
  deltaEdge = sensorDelta - deltaSlow;

  unsigned long nowMs = millis();
  bool completed = false;

  // เส้นฐานไล่ตามการดริฟต์ (อุณหภูมิ/แสงรอบข้าง/แรงดันไฟ)
  // ต้องไล่ตามทั้งในเฟสเฝ้าดูและเฟสรอนิ่ง หยุดเฉพาะตอนที่พัลส์กำลังดำเนินอยู่เท่านั้น
  // (ถ้าหยุดตามในเฟสรอนิ่งด้วย ระดับที่ไหลไปเรื่อย ๆ จะดันให้ค้างอยู่ในเฟสนั้นตลอดไป)
  // ถุงน้ำเกลือแกว่งตอนคนไข้เดิน = เส้นฐานเคลื่อนเป็นช่วง ๆ ตรวจไว้เพื่อไล่ตามให้ทัน
  if (nowMs - motionCheckMs >= MOTION_WINDOW_MS) {
    motionLevel = abs(sensorBaseline - baselineRef);
    baselineRef = sensorBaseline;
    motionCheckMs = nowMs;
    int thr = (calNoisePp > 4) ? calNoisePp : 4;
    if (motionLevel > thr) motionUntil = nowMs + MOTION_HOLD_MS;
  }
  inMotion = (nowMs < motionUntil);

  if (dropPhase != DP_ACTIVE) {
    long tau = BASELINE_TAU;
    if (inMotion) tau = BASELINE_TAU_FAST;             // กำลังแกว่ง -> ไล่ตามเร็วขึ้น
    if (dropPhase == DP_REARM && deltaEdge > dropReleaseDelta)
      tau = BASELINE_TAU_FAST;                         // ค้างเหนือเกณฑ์ปลด -> ดึงกลับเร็วขึ้น
    baselineAcc += ((long)sensorFiltered * 256 - baselineAcc) * tick / tau;

    if (dropPhase == DP_IDLE && abs(deltaEdge) < dropTriggerDelta / 2) {  // วัดสัญญาณรบกวนจากช่วงที่สงบจริง ๆ
      if (sensorFiltered < noiseMin) noiseMin = sensorFiltered;
      if (sensorFiltered > noiseMax) noiseMax = sensorFiltered;
    }
    if (nowMs - noiseWindowStart >= NOISE_WINDOW_MS) {
      if (noiseMax >= noiseMin) sensorNoisePp = noiseMax - noiseMin;
      noiseMin = sensorFiltered; noiseMax = sensorFiltered;
      noiseWindowStart = nowMs;
    }
  }

  switch (dropPhase) {
    case DP_IDLE:
      if (deltaEdge >= dropTriggerDelta) {
        dropPhase = DP_ACTIVE;
        dropPhaseMs = nowMs;
        dropPeakDelta = deltaEdge;
        dropRiseMs = 0;
      }
      break;

    case DP_ACTIVE: {
      if (deltaEdge > dropPeakDelta) {
        dropPeakDelta = deltaEdge;
        dropRiseMs = nowMs - dropPhaseMs;      // ยอดใหม่ -> ขอบขาขึ้นยังไต่อยู่
      }
      unsigned long width = nowMs - dropPhaseMs;

      if (deltaEdge <= dropReleaseDelta) {
        // พัลส์จบ -> ตัดสินใจ "ที่เดียว" จึงนับได้ไม่เกิน 1 ครั้งต่อ 1 พัลส์
        if (pulseLooksLikeDrop(dropPeakDelta, (int)width, (int)dropRiseMs)) {
          lastDropPeak    = dropPeakDelta;
          lastDropWidthMs = (int)width;
          lastDropRiseMs  = (int)dropRiseMs;
          completed = true;
          completedPulses++;
          learnFromPulse(dropPeakDelta, (int)width);
          if (countDrops) registerDrop(dropPhaseMs);
        } else {
          rejectedPulses++;                 // รูปร่างไม่ใช่หยด (รบกวน/ถุงแกว่ง/สิ่งบังลำแสง)
        }
        dropPhase = DP_REARM;
        dropPhaseMs = nowMs;
        dropSettleMs = nowMs;
      }
      else if (width > DROP_MAX_WIDTH_MS) {
        // ค้างนานผิดปกติ = ไม่ใช่หยด (ระดับน้ำเปลี่ยน / มีอะไรบังลำแสง)
        rejectedPulses++;
        dropPhase = DP_REARM;
        dropPhaseMs = nowMs;
        dropSettleMs = nowMs;
      }
      break;
    }

    case DP_REARM:
      // ต้องกลับมานิ่งใต้เกณฑ์ปลดต่อเนื่องครบเวลา จึงจะรับพัลส์ถัดไป
      // นี่คือส่วนที่ทำให้ "กดสวิตช์แล้วลั่น" หายไป
      if (deltaEdge > dropReleaseDelta) {
        dropSettleMs = nowMs;                          // ยังไม่นิ่ง เริ่มจับเวลานิ่งใหม่
      } else if (nowMs - dropSettleMs >= DROP_REARM_MS) {
        dropPhase = DP_IDLE;
      }

      // ทางออกกันค้าง: ถ้าอยู่ในเฟสนี้นานเกินไปแปลว่าระดับสัญญาณเลื่อนไปจริง ๆ
      // (เซนเซอร์ขยับ ไฟตก แสงรอบข้างเปลี่ยน หรือเส้นฐานตอนเริ่มเพี้ยน)
      // ถ้าไม่ยึดเส้นฐานใหม่ตรงนี้ เครื่องจะค้างอยู่ในเฟสนี้ตลอดไปและจะไม่นับหยดอีกเลย
      if (nowMs - dropPhaseMs >= DROP_REARM_TIMEOUT_MS) {
        baselineAcc = (long)sensorFiltered * 256;
        sensorBaseline = sensorFiltered;
        sensorDelta = 0;
        dropPhase = DP_IDLE;
        baselineRecovers++;
      }
      break;
  }

  return completed;
}

// ============================================================================
// จอออสซิลโลสโคปเล็ก ๆ สำหรับหน้าคาลิเบรต
// แสดง "ระยะเบนจากเส้นฐาน" ตามเวลา จึงเห็นชัดว่าอันไหนคือหยด อันไหนคือช่วงว่าง
// ============================================================================
#define SCOPE_X            7
#define SCOPE_W            158
#define SCOPE_H            72
#define SCOPE_MS_PER_COL   16          // 1 คอลัมน์ = 16 ms -> เห็นย้อนหลัง ~2.5 วินาที

int16_t scopeCol[SCOPE_W];
uint8_t scopeHead = 0;
int scopeColMax = 0;
int scopeFull = 200;                   // ค่าที่ทำให้แท่งสูงเต็มจอ
int scopeFixedFull = 0;                // > 0 = ล็อกสเกลไว้เอง (ใช้ตอนวัดสัญญาณรบกวน)
unsigned long scopeColStart = 0;

void scopeReset() {
  for (int i = 0; i < SCOPE_W; i++) scopeCol[i] = 0;
  scopeHead = 0;
  scopeColMax = 0;
  scopeColStart = millis();
}

void scopePush(int delta) {
  if (delta < 0) delta = 0;
  if (delta > scopeColMax) scopeColMax = delta;
  unsigned long now = millis();
  if (now - scopeColStart < SCOPE_MS_PER_COL) return;
  scopeColStart = now;
  scopeCol[scopeHead] = (int16_t)scopeColMax;
  scopeHead = (uint8_t)((scopeHead + 1) % SCOPE_W);
  scopeColMax = delta;
}

void scopeFrame(int y) {
  tft.drawRect(SCOPE_X - 1, y - 1, SCOPE_W + 2, SCOPE_H + 2, UI_LINE);
}

int scopeYof(int y, int value) {
  if (scopeFull < 20) scopeFull = 20;
  long h = (long)value * (SCOPE_H - 1) / scopeFull;
  if (h < 0) h = 0;
  if (h > SCOPE_H - 1) h = SCOPE_H - 1;
  return y + SCOPE_H - 1 - (int)h;
}

void scopeDraw(int y) {
  // ปรับสเกลอัตโนมัติให้พัลส์ที่เห็นอยู่พอดีจอ (หรือใช้สเกลที่ล็อกไว้)
  if (scopeFixedFull > 0) {
    scopeFull = scopeFixedFull;
  } else {
    int peak = dropTriggerDelta * 2;
    for (int i = 0; i < SCOPE_W; i++) if (scopeCol[i] > peak) peak = scopeCol[i];
    scopeFull = (peak * 12) / 10;
    if (scopeFull < 40) scopeFull = 40;
  }

  int yTrig = scopeYof(y, dropTriggerDelta);
  int yRel  = scopeYof(y, dropReleaseDelta);
  int yNoise = scopeYof(y, sensorNoisePp);

  for (int i = 0; i < SCOPE_W; i++) {
    // การวาดกราฟทั้งแถบใช้เวลาหลายสิบมิลลิวินาทีบนจอจริง ถ้าปล่อยให้ขาดการอ่าน
    // ตลอดช่วงนั้น ตัวตรวจจับจะถูกดีดกลับไปเฟสรอนิ่งทุกครั้งที่วาดจอ จนไม่เหลือ
    // จังหวะเฝ้ารอหยดเลย จึงต้องแทรกการอ่านเซนเซอร์ไว้ระหว่างวาดด้วย
    if ((i % 12) == 0) serviceDropSensor(false);
    int x = SCOPE_X + i;
    int v = scopeCol[(scopeHead + i) % SCOPE_W];
    int yTop = scopeYof(y, v);

    tft.drawFastVLine(x, y, yTop - y, THEME_BG);                       // ลบของเดิมด้านบน
    if (yTop < y + SCOPE_H)
      tft.drawFastVLine(x, yTop, y + SCOPE_H - yTop,
                        (v >= dropTriggerDelta) ? COLOR_YELLOW : THEME_SKYBLUE);
    // เส้นอ้างอิงวาดทับเฉพาะจุดที่เป็นพื้นหลัง (ทุก ๆ 2 พิกเซลให้เป็นเส้นประ)
    if ((i % 4) < 2) {
      if (dropTriggerDelta < scopeFull && yTrig  < yTop) tft.drawPixel(x, yTrig,  COLOR_ORANGE);
      if (dropReleaseDelta < scopeFull && yRel   < yTop) tft.drawPixel(x, yRel,   THEME_PINK);
      if (sensorNoisePp    < scopeFull && yNoise < yTop) tft.drawPixel(x, yNoise, UI_DIM);
    }
  }
}

// ============================================================================
// ตัวช่วยของหน้าคาลิเบรต
// ============================================================================
int calPeaks[CAL_MAX_SAMPLES];
int calWidths[CAL_MAX_SAMPLES];
int calCount = 0;

int calMedian(int *src, int n) {
  int t[CAL_MAX_SAMPLES];
  memcpy(t, src, sizeof(int) * n);
  for (int i = 1; i < n; i++) {
    int k = t[i], j = i - 1;
    while (j >= 0 && t[j] > k) { t[j + 1] = t[j]; j--; }
    t[j + 1] = k;
  }
  return t[n / 2];
}

int calMinOf(int *src, int n) {
  int m = src[0];
  for (int i = 1; i < n; i++) if (src[i] < m) m = src[i];
  return m;
}

int calMaxOf(int *src, int n) {
  int m = src[0];
  for (int i = 1; i < n; i++) if (src[i] > m) m = src[i];
  return m;
}

const char* calQualityWord(uint8_t q) {
  switch (q) {
    case 4:  return "EXCELLENT";
    case 3:  return "GOOD";
    case 2:  return "FAIR";
    case 1:  return "POOR";
    default: return "TOO WEAK";
  }
}

uint16_t calQualityColor(uint8_t q) {
  switch (q) {
    case 4:  return COLOR_GREEN;
    case 3:  return UI_GREEN;
    case 2:  return COLOR_YELLOW;
    case 1:  return COLOR_ORANGE;
    default: return COLOR_RED;
  }
}

void calHeader(const char* step, const char* line1, const char* line2) {
  tft.fillScreen(THEME_BG);
  tft.fillRect(0, 0, 172, 28, THEME_PINK);
  drawCenteredString("CALIBRATION", 8, 1, COLOR_WHITE, THEME_PINK);
  drawCenteredString(step, 36, 1, COLOR_YELLOW, THEME_BG);
  drawCenteredString(line1, 52, 1, COLOR_WHITE, THEME_BG);
  drawCenteredString(line2, 66, 1, UI_DIM, THEME_BG);
  scopeFrame(SCOPE_TOP);
  tft.fillRoundRect(6, 164, 160, 122, 6, THEME_NAVY);
}

bool calButtonPressed() { return digitalRead(BTN_PIN) == LOW; }

// ลูปหลักไม่ได้ทำงานระหว่างอยู่ในหน้าคาลิเบรต จึงต้องมีตัวปิดเสียงของตัวเอง
// (ใช้ beepNonBlocking ตรงนี้ไม่ได้ เพราะไม่มีใครเรียก noTone ให้)
unsigned long calBeepUntil = 0;
void calBeep(uint16_t freq, uint16_t ms) { tone(BUZZER_PIN, freq); calBeepUntil = millis() + ms; }
void calServiceBeep() {
  if (calBeepUntil && millis() >= calBeepUntil) { noTone(BUZZER_PIN); calBeepUntil = 0; }
}

void calWaitRelease() {
  while (digitalRead(BTN_PIN) == LOW) delay(10);
  delay(60);
}

// แถวค่าตัวเลขในการ์ดผลลัพธ์ (ล้างพื้นก่อนเขียนเสมอ เพื่อไม่ให้ตัวเลขเก่าค้าง)
void calRow(int y, const String &label, const String &value, uint16_t col) {
  tft.fillRect(10, y, 152, 14, THEME_NAVY);
  textAt(label, 12, y, 1, UI_DIM, THEME_NAVY);
  textAt(value, 78, y, 1, col, THEME_NAVY);
}

// ============================================================================
// ขั้นที่ 1 — วัดพื้นสัญญาณรบกวน (ไม่ต้องมีหยด)
// ได้ค่า: เส้นฐาน และสัญญาณรบกวนยอดถึงยอด ซึ่งเป็น "ขีดล่าง" ของเซ็นเซอร์ตัวนี้
// ============================================================================
bool calStepNoise() {
  calHeader("STEP 1 OF 3   NOISE", "NO DROPS PLEASE", "measuring sensor noise");
  drawCenteredString("CLICK = CANCEL", 296, 1, UI_DIM, THEME_BG);
  scopeReset();

  dropTriggerDelta = 4000;            // ปิดการจับพัลส์ชั่วคราว ให้วัดเฉพาะสัญญาณรบกวน
  dropReleaseDelta = 3999;
  scopeFixedFull = 120;               // ซูมให้เห็นย่านสัญญาณรบกวนชัด ๆ
  resetSignalChain(readSensorAverage(16));

  int wMin = 4095, wMax = 0;
  unsigned long start = millis();
  unsigned long lastDraw = 0;

  while (millis() - start < CAL_NOISE_MS) {
    if (serviceDropSensor(false)) { /* ไม่นับระหว่างคาลิเบรต */ }
    scopePush(deltaEdge);   // แสดงสัญญาณเดียวกับที่ตัวตรวจจับใช้ตัดสิน
    if (sensorFiltered < wMin) wMin = sensorFiltered;
    if (sensorFiltered > wMax) wMax = sensorFiltered;

    if (calButtonPressed()) { calWaitRelease(); scopeFixedFull = 0; return false; }

    if (millis() - lastDraw >= 150) {
      lastDraw = millis();
      scopeDraw(SCOPE_TOP);
      int pct = (int)((millis() - start) * 100 / CAL_NOISE_MS);
      tft.drawRect(12, 170, 148, 10, UI_LINE);
      tft.fillRect(13, 171, 146 * pct / 100, 8, THEME_CYAN);
      calRow(190, "BASELINE", String(sensorBaseline), COLOR_WHITE);
      calRow(208, "NOISE pp", String(wMax - wMin) + " adc", COLOR_WHITE);
      calRow(226, "LIVE", String(sensorFiltered), THEME_SKYBLUE);
    }
    calServiceBeep();
    delay(1);
  }

  scopeFixedFull = 0;
  calNoisePp = wMax - wMin;
  if (calNoisePp < 1) calNoisePp = 1;

  const char* verdict = (calNoisePp <= 20) ? "VERY QUIET"
                      : (calNoisePp <= 60) ? "OK"
                      : (calNoisePp <= 150) ? "NOISY - SHIELD IT"
                                            : "VERY NOISY";
  uint16_t vcol = (calNoisePp <= 60) ? COLOR_GREEN : (calNoisePp <= 150) ? COLOR_YELLOW : COLOR_RED;
  calRow(208, "NOISE pp", String(calNoisePp) + " adc", vcol);
  calRow(244, "RESULT", String(verdict), vcol);
  modalSafeBeep(2000, 60);
  delay(900);
  return true;
}

// ============================================================================
// ขั้นที่ 2 — เรียนรู้จากหยดจริง
// จับพัลส์จริงด้วยเกณฑ์ไวชั่วคราว (4 เท่าของสัญญาณรบกวน) แล้วเก็บความสูง/ความกว้าง
// พร้อมเดาทิศของสัญญาณให้เอง (หยดทำให้ค่าลดลงหรือเพิ่มขึ้น)
// ============================================================================
bool calStepLearn() {
  calHeader("STEP 2 OF 3   DROPS", "LET IT DRIP NOW", "aim beam below nozzle");
  drawCenteredString("CLICK = DONE", 296, 1, UI_DIM, THEME_BG);
  scopeReset();
  calCount = 0;

  int probe = calNoisePp * 4;
  if (probe < CAL_MIN_TRIGGER) probe = CAL_MIN_TRIGGER;
  dropTriggerDelta = probe;
  dropReleaseDelta = probe * 2 / 5;
  if (dropReleaseDelta < 2) dropReleaseDelta = 2;

  // ระหว่างขั้นนี้ให้รับพัลส์ทุกแบบก่อน เพราะยังไม่รู้ว่าหยดหน้าตาอย่างไร
  learnAmp = 0; learnWidth = 0;
  shapeGateEnabled = false;

  // ---- เดาทิศสัญญาณ: ดูว่าค่าเบนออกจากเส้นฐานไปทางไหนแรงกว่ากัน ----
  dropPolarity = -1;
  resetSignalChain(readSensorAverage(16));
  int excDown = 0, excUp = 0;
  unsigned long polStart = millis();
  unsigned long lastDraw = 0;
  bool polarityLocked = false;

  while (!polarityLocked && millis() - polStart < CAL_POLARITY_MS) {
    if (serviceDropSensor(false)) { }
    int d = sensorBaseline - sensorFiltered;
    if (d > excDown) excDown = d;
    if (-d > excUp) excUp = -d;
    if (excDown >= probe || excUp >= probe) polarityLocked = true;
    if (calButtonPressed()) { calWaitRelease(); return false; }
    if (millis() - lastDraw >= 120) {
      lastDraw = millis();
      calRow(172, "SIGNAL", "watching...", THEME_SKYBLUE);
      calRow(190, "DOWN/UP", String(excDown) + " / " + String(excUp), COLOR_WHITE);
    }
    calServiceBeep();
    delay(1);
  }
  if (excUp > excDown) dropPolarity = 1;
  resetSignalChain(readSensorAverage(16));
  scopeReset();
  calRow(172, "SIGNAL", (dropPolarity < 0) ? "drop = LOWER" : "drop = HIGHER", COLOR_WHITE);

  // ---- เก็บพัลส์จริง ----
  unsigned long start = millis();
  lastDraw = 0;
  uint32_t seenPulses = completedPulses;
  while (calCount < CAL_TARGET_DROPS && millis() - start < CAL_LEARN_TIMEOUT_MS) {
    serviceDropSensor(false);
    // อ่านจากตัวนับ ไม่ใช่ค่าที่ฟังก์ชันคืนมา เพราะพัลส์อาจจบระหว่างที่กำลังวาดกราฟอยู่
    if (completedPulses != seenPulses) {
      seenPulses = completedPulses;
      if (calCount < CAL_MAX_SAMPLES) {
        calPeaks[calCount]  = lastDropPeak;
        calWidths[calCount] = lastDropWidthMs;
        calCount++;
        calBeep(2600, 25);
      }
    }
    // ได้หยดพอประมาณและความสูงใกล้เคียงกันแล้ว = รู้จักหยดแล้ว ไม่ต้องรอให้ครบ
    if (calCount >= CAL_QUICK_DROPS) {
      int mn = calMinOf(calPeaks, calCount), mx = calMaxOf(calPeaks, calCount);
      if (mx > 0 && (mx - mn) * 100 / mx <= CAL_QUICK_SPREAD_PCT) break;
    }
    scopePush(deltaEdge);   // แสดงสัญญาณเดียวกับที่ตัวตรวจจับใช้ตัดสิน

    if (calButtonPressed()) { calWaitRelease(); break; }

    if (millis() - lastDraw >= 150) {
      lastDraw = millis();
      scopeDraw(SCOPE_TOP);
      calRow(190, "DROPS", String(calCount) + " / " + String(CAL_TARGET_DROPS),
             (calCount >= CAL_MIN_DROPS) ? COLOR_GREEN : COLOR_YELLOW);
      calRow(208, "LAST PK", String(lastDropPeak) + " adc", COLOR_WHITE);
      calRow(226, "LAST W", String(lastDropWidthMs) + " ms", COLOR_WHITE);
      calRow(244, "REJECT", String(rejectedPulses) + " RE " + String(baselineRecovers) +
                            " GAP " + String(sampleGaps),
             (rejectedPulses || baselineRecovers) ? COLOR_ORANGE : UI_DIM);
      const char* ph = (dropPhase == DP_IDLE) ? "WATCH" : (dropPhase == DP_ACTIVE) ? "PULSE" : "SETTLE";
      calRow(262, ph, String(deltaEdge) + " / " + String(dropTriggerDelta) +
             (inMotion ? "  MOVE" : ""), THEME_SKYBLUE);
    }
    calServiceBeep();
    delay(1);
  }

  shapeGateEnabled = true;
  if (calCount < CAL_MIN_DROPS) {
    calRow(262, "RESULT", "NOT ENOUGH DROPS", COLOR_RED);
    modalSafeBeep(700, 250);
    delay(1600);
    return false;
  }
  modalSafeBeep(2200, 80);
  return true;
}

// ============================================================================
// ขั้นที่ 3 — คำนวณเกณฑ์ที่ดีที่สุดระหว่าง "พ้นสัญญาณรบกวน" กับ "ต่ำกว่าหยดที่จางที่สุด"
// แล้วรายงานอัตราส่วนสัญญาณต่อสัญญาณรบกวน (SNR) ให้เห็นขีดความสามารถจริงของเซ็นเซอร์
// ============================================================================
void calFinish() {
  int medPeak  = calMedian(calPeaks, calCount);
  int maxPeak  = calMaxOf(calPeaks, calCount);
  int medWidth = calMedian(calWidths, calCount);

  // หา "หยดที่จางที่สุดที่ยังน่าเชื่อถือ" โดยตัดพัลส์ที่ต่ำกว่าครึ่งหนึ่งของค่ากลางทิ้ง
  // (พัลส์เตี้ยผิดปกติมักเกิดจากจังหวะที่การอ่านขาดช่วงไปตอนวาดจอ ไม่ใช่หยดที่จางจริง)
  int minPeak = medPeak;
  for (int i = 0; i < calCount; i++)
    if (calPeaks[i] >= medPeak / 2 && calPeaks[i] < minPeak) minPeak = calPeaks[i];

  int noiseFloor = calNoisePp * 3;                 // ต้องสูงกว่าสัญญาณรบกวนอย่างน้อย 3 เท่า
  if (noiseFloor < CAL_MIN_TRIGGER) noiseFloor = CAL_MIN_TRIGGER;
  int signalCap = (minPeak * 55) / 100;            // ต้องต่ำกว่าหยดที่จางที่สุดพอสมควร

  dropTriggerDelta = (noiseFloor + signalCap) / 2; // จุดกึ่งกลางของสองข้อจำกัด
  // ถ้าสัญญาณอ่อนจนสองข้อจำกัดชนกัน อย่างน้อยต้องไม่ตกไปอยู่ในย่านสัญญาณรบกวน
  if (dropTriggerDelta < calNoisePp * 2) dropTriggerDelta = calNoisePp * 2;
  if (dropTriggerDelta < CAL_MIN_TRIGGER) dropTriggerDelta = CAL_MIN_TRIGGER;
  dropReleaseDelta = dropTriggerDelta * 2 / 5;
  if (dropReleaseDelta < 2) dropReleaseDelta = 2;

  // ให้ตัวเรียนรู้เริ่มจากค่ากลางที่วัดได้จริง แล้วค่อยปรับตัวต่อไปเองระหว่างใช้งาน
  learnAmp   = medPeak;
  learnWidth = medWidth;
  shapeGateEnabled = true;

  calDropAmp = medPeak;
  calSnrX10  = (uint8_t)min(255L, (long)medPeak * 10 / max(1, calNoisePp));
  int spreadPct = (medPeak > 0) ? ((maxPeak - minPeak) * 100 / medPeak) : 999;

  if      (calSnrX10 >= 80) calQuality = 4;
  else if (calSnrX10 >= 50) calQuality = 3;
  else if (calSnrX10 >= 30) calQuality = 2;
  else if (calSnrX10 >= 20) calQuality = 1;
  else                      calQuality = 0;

  stationPrefs.begin("st_cal", false);
  stationPrefs.putInt("pol",   dropPolarity);
  stationPrefs.putInt("trig",  dropTriggerDelta);
  stationPrefs.putInt("rel",   dropReleaseDelta);
  stationPrefs.putInt("amp",   calDropAmp);
  stationPrefs.putInt("noise", calNoisePp);
  stationPrefs.putUChar("snr", calSnrX10);
  stationPrefs.putUChar("q",   calQuality);
  stationPrefs.putInt("lamp",  learnAmp);
  stationPrefs.putInt("lwid",  learnWidth);
  stationPrefs.end();

  calHeader("STEP 3 OF 3   RESULT", "SETTINGS SAVED", "click to verify");
  drawCenteredString("CLICK = VERIFY", 296, 1, UI_DIM, THEME_BG);
  scopeDraw(SCOPE_TOP);

  uint16_t qc = calQualityColor(calQuality);
  calRow(172, "QUALITY", String(calQualityWord(calQuality)), qc);
  calRow(190, "SNR", String(calSnrX10 / 10) + "." + String(calSnrX10 % 10) + " x", qc);
  calRow(208, "DROP PK", String(medPeak) + " adc", COLOR_WHITE);
  calRow(226, "NOISE pp", String(calNoisePp) + " adc", COLOR_WHITE);
  calRow(244, "TRIGGER", String(dropTriggerDelta) + " / " + String(dropReleaseDelta), COLOR_WHITE);
  calRow(262, "WIDTH", String(medWidth) + " ms", COLOR_WHITE);

  if (spreadPct > 80) {
    tft.fillRect(12, 272, 148, 12, THEME_NAVY);
    textCenterIn("UNEVEN-ALIGN SENSOR", 8, 156, 273, 1, COLOR_ORANGE, THEME_NAVY);
  }

  modalSafeBeep(2000, 90);
  delay(120);
  modalSafeBeep(2600, 140);

  unsigned long start = millis();
  while (millis() - start < 12000) {
    if (serviceDropSensor(false)) { }
    if (calButtonPressed()) { calWaitRelease(); break; }
    calServiceBeep();
    delay(2);
  }
}

// ============================================================================
// ขั้นตรวจสอบ — นับหยดจริงด้วยค่าที่เพิ่งได้ ให้ผู้ใช้เทียบกับตาตัวเอง
// ============================================================================
void calVerify() {
  calHeader("VERIFY", "COUNT WITH YOUR EYES", "1 drop must add 1");
  drawCenteredString("CLICK = FINISH", 296, 1, UI_DIM, THEME_BG);
  scopeReset();
  resetSignalChain(readSensorAverage(16));

  uint32_t seen = 0;
  uint32_t rejBase = rejectedPulses;
  unsigned long start = millis();
  unsigned long lastDraw = 0;

  uint32_t seenPulses = completedPulses;
  while (millis() - start < CAL_VERIFY_MS) {
    serviceDropSensor(false);
    if (completedPulses != seenPulses) { seenPulses = completedPulses; seen++; calBeep(2800, 25); }
    scopePush(deltaEdge);   // แสดงสัญญาณเดียวกับที่ตัวตรวจจับใช้ตัดสิน
    if (calButtonPressed()) { calWaitRelease(); break; }

    if (millis() - lastDraw >= 150) {
      lastDraw = millis();
      scopeDraw(SCOPE_TOP);
      tft.fillRect(12, 170, 148, 34, THEME_NAVY);
      drawCenteredString(String(seen), 172, 4, COLOR_GREEN, THEME_NAVY);
      calRow(208, "LAST PK", String(lastDropPeak) + " adc", COLOR_WHITE);
      calRow(226, "LAST W", String(lastDropWidthMs) + " ms", COLOR_WHITE);
      calRow(244, "REJECT", String(rejectedPulses - rejBase),
             (rejectedPulses - rejBase) ? COLOR_ORANGE : UI_DIM);
      int left = (int)((CAL_VERIFY_MS - (millis() - start)) / 1000);
      calRow(262, "TIME", String(left) + " s", UI_DIM);
    }
    calServiceBeep();
    delay(1);
  }
  modalSafeBeep(2400, 80);
}

// ============================================================================
// ตัวคุมลำดับทั้งหมด (เรียกจากการกดปุ่มค้าง 3 วินาที)
// ============================================================================
void executeButtonCalibrationWizard() {
  noTone(BUZZER_PIN);
  buzzerBeepUntil = 0;

  int keepTrig = dropTriggerDelta, keepRel = dropReleaseDelta;
  int8_t keepPol = dropPolarity;

  bool ok = calStepNoise();
  if (ok) ok = calStepLearn();

  if (ok) {
    calFinish();
    calVerify();
  } else {
    // ยกเลิกหรือเก็บหยดไม่พอ -> คืนค่าเดิม ไม่เขียนทับของที่ใช้ได้อยู่
    dropTriggerDelta = keepTrig;
    dropReleaseDelta = keepRel;
    dropPolarity = keepPol;
    tft.fillScreen(THEME_BG);
    drawCenteredString("CANCELLED", 140, 2, COLOR_YELLOW, THEME_BG);
    drawCenteredString("previous settings kept", 168, 1, UI_DIM, THEME_BG);
    delay(1200);
  }

  noTone(BUZZER_PIN);
  calBeepUntil = 0;
  buzzerBeepUntil = 0;

  // ระหว่างคาลิเบรตไม่ได้นับหยด -> อย่าให้ช่วงที่หายไปกลายเป็นสายพับหรือทำอัตราไหลเพี้ยน
  resetSignalChain(readSensorAverage(16));
  if (hasFirstDropOccurred) {
    lastDropTimestamp = millis();
    skipNextInterval = true;
  }
  lastUserActivityTime = millis();
  stateNeedsRedraw = true;
}

// ----------------------------------------------------------------------------
// ESP-NOW Receive Callback (รับ Sync จาก Host)
// ----------------------------------------------------------------------------
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
void OnTimeSyncRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  int rssi = (info && info->rx_ctrl) ? info->rx_ctrl->rssi : -55;
#else
void OnTimeSyncRecv(const uint8_t *mac, const uint8_t *incomingDataBytes, int len) {
  int rssi = -55;
#endif
  if (len != sizeof(struct_host_sync)) return;   // แพ็กเก็ตของ Station อื่น (22 bytes) จะถูกกรองออก

  struct_host_sync syncData;
  memcpy(&syncData, incomingDataBytes, sizeof(syncData));
  lastHostRssi = rssi;
  lastHostRecvTime = millis();
  isHostOnline = true;
  hostReportedChannel = syncData.hostChannel;

  if (syncData.epochTime > 1600000000) {
    struct timeval tv = { (time_t)syncData.epochTime, 0 };
    settimeofday(&tv, NULL);
    isClockSynced = (syncData.isSynced == 1);
  }

  if (syncData.stationId == currentStationId) {
    targetRateHr  = syncData.targetRateHr;
    totalPlanMl   = syncData.totalPlanMl;
    hostAlertCode = syncData.alertCode;
    dropFactor    = safeDropFactor(syncData.dropFactor);
    nearEndPct    = safeNearPct(syncData.nearEndPct);
    hostNearEndAck = (syncData.flags & 0x01) != 0;
    if (hostNearEndAck) nearEndAckRequest = false;   // Host ยืนยันแล้ว

    if (knownResetSeq < 0) {
      knownResetSeq = syncData.resetSeq;          // ครั้งแรกหลังบูต: รับค่าโดยไม่รีเซ็ต
    } else if (syncData.resetSeq != (uint8_t)knownResetSeq) {
      knownResetSeq = syncData.resetSeq;
      pendingCounterReset = true;                  // ให้ loop() เป็นผู้รีเซ็ต
    }
  }
}

void applyCounterReset() {
  pendingCounterReset = false;
  totalDrops = 0;
  periodDropsCounter = 0;
  totalVolumeMl = 0;
  currentFlowRate_ml_hr = 0;
  currentGttMin = 0;
  resetRateWindow();
  hasFirstDropOccurred = false;
  skipNextInterval = false;
  isDropFalling = false;
  snoozeUntilTime = 0;
  hostAlertCode = ALERT_NONE;
  hostNearEndAck = false;
  nearEndAckRequest = false;
  nearNextChimeTime = 0;
  nearChimeRemaining = 0;
  if (currentState == STATE_EMERGENCY || currentState == STATE_NEAR_END_NOTICE) currentState = STATE_NORMAL_VIEW;
  resetTrendData();                 // เริ่มถุงใหม่ = เริ่มกราฟแนวโน้มใหม่
  stateNeedsRedraw = true;
  beepNonBlocking(2400, 120);
}

// ----------------------------------------------------------------------------
// ค้นหาช่องสัญญาณของ Host
// ----------------------------------------------------------------------------
void setEspNowChannel(uint8_t ch) {
  if (esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE) == ESP_OK) {
    espnowChannel = ch;
  }
}

void manageChannelHunting(unsigned long now) {
  if (isHostOnline) {
    if (espnowChannel != savedChannel) {
      savedChannel = espnowChannel;
      stationPrefs.begin("st_cfg", false);
      stationPrefs.putUChar("ch", savedChannel);
      stationPrefs.end();
    }
    return;
  }
  if (now < CHANNEL_SCAN_START_MS) return;
  if (currentState == STATE_CONFIG_ID) return;

  if (now - lastChannelHopTime >= CHANNEL_SCAN_DWELL_MS) {
    lastChannelHopTime = now;
    uint8_t next = (espnowChannel % MAX_WIFI_CHANNEL) + 1;
    setEspNowChannel(next);
  }
}

void togglePause() {
  isRunning = !isRunning;
  currentFlowRate_ml_hr = 0.0f;
  currentGttMin = 0.0f;
  resetRateWindow();
  if (isRunning) {
    soundStart();
    // เริ่มนับเวลาใหม่หลังกลับมาทำงาน ไม่ให้ช่วงที่หยุดถูกนับเป็นสายพับหรือใช้คำนวณ rate
    if (hasFirstDropOccurred) {
      lastDropTimestamp = millis();
      skipNextInterval = true;
    }
  } else {
    soundStop();
    noTone(BUZZER_PIN);
  }
  stateNeedsRedraw = true;
}

String getStationClockStr() {
  if (!isClockSynced) return String("--:--");
  time_t now;
  time(&now);
  struct tm *timeinfo = localtime(&now);
  char buf[12];
  strftime(buf, sizeof(buf), "%H:%M", timeinfo);
  return String(buf);
}

// ============================================================================
// ส่วนแสดงผล (UI v2) — เน้นอ่านง่ายจากปลายเตียง ใช้ข้อความเท่าที่จำเป็น
//   หน้า 1 IV BAG  : ภาพถุงน้ำเกลือ + กระเปาะหยด + ตัวเลขสำคัญ 4 ค่า
//   หน้า 2 PLAN    : แผนการให้สารน้ำและเวลา (ตัวเลขใหญ่ 4 ค่า)
//   หน้า 3 LINK    : สถานะการเชื่อมต่อและเซนเซอร์
//   หน้า 4 TREND   : กราฟแท่งแนวโน้มอัตราไหลย้อนหลัง 30 นาที
// ทุกหน้าวาดเฉพาะส่วนที่ค่าเปลี่ยน เพื่อไม่ให้ SPI แย่งเวลาการอ่านเซนเซอร์
// ============================================================================

UiStatus uiStatusOf(uint8_t code) {
  if (!isRunning) return UI_PAUSED;
  switch (code) {
    case ALERT_OCCLUSION: return UI_NOFLOW;
    case ALERT_TOO_FAST:  return UI_FAST;
    case ALERT_TOO_SLOW:  return UI_SLOW;
    case ALERT_COMPLETE:  return UI_DONE;
    case ALERT_NEAR_END:  return UI_NEAREND;
    default:              return UI_OK;
  }
}

uint16_t uiStatusColor(UiStatus s) {
  switch (s) {
    case UI_OK:      return UI_GREEN;
    case UI_FAST:    return COLOR_ORANGE;
    case UI_SLOW:    return COLOR_YELLOW;
    case UI_NOFLOW:  return COLOR_RED;
    case UI_NEAREND: return COLOR_ORANGE;
    case UI_DONE:    return THEME_CYAN;
    default:         return THEME_SKYBLUE;
  }
}

const char* uiStatusWord(UiStatus s) {
  switch (s) {
    case UI_OK:      return "NORMAL";
    case UI_FAST:    return "TOO FAST";
    case UI_SLOW:    return "TOO SLOW";
    case UI_NOFLOW:  return "NO FLOW";
    case UI_NEAREND: return "NEXT BAG";
    case UI_DONE:    return "COMPLETE";
    default:         return "PAUSED";
  }
}


void textRight(const String &s, int xRight, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, xRight - (int)s.length() * 6 * size, y, size, col, bg);
}

void textCenterIn(const String &s, int x, int w, int y, uint8_t size, uint16_t col, uint16_t bg) {
  textAt(s, x + (w - (int)s.length() * 6 * size) / 2, y, size, col, bg);
}

// เติมช่องว่างท้ายข้อความให้ยาวคงที่ เพื่อเขียนทับของเดิมได้โดยไม่ต้องล้างพื้นที่ (ไม่กะพริบ)
String padTo(const String &s, unsigned int width) {
  String out = s;
  while (out.length() < width) out += ' ';
  return out;
}

// ---- ค่าที่ใช้ร่วมกันหลายหน้า ----
int remainingMlInt() {
  float r = totalPlanMl - totalVolumeMl;
  if (r < 0) r = 0;
  return (int)(r + 0.5f);
}

int infusedPct() {
  if (totalPlanMl <= 0) return -1;
  int p = (int)(totalVolumeMl * 100.0f / totalPlanMl + 0.5f);
  return (p > 100) ? 100 : p;
}

// นาทีที่เหลือจนหมดถุง (-1 = คำนวณไม่ได้)
int minutesLeft() {
  if (totalPlanMl <= 0) return -1;
  if (!isRunning || isLocallyOccluded()) return -1;
  float calcRate = (currentFlowRate_ml_hr > 5.0f) ? currentFlowRate_ml_hr : (float)targetRateHr;
  if (calcRate <= 0) return -1;
  return (int)((remainingMlInt() / calcRate) * 60.0f);
}

String timeLeftText(bool compact) {
  int m = minutesLeft();
  if (m < 0) return compact ? String("--h--m") : String("--h --m");
  char b[16];
  snprintf(b, sizeof(b), compact ? "%dh%02dm" : "%dh %02dm", m / 60, m % 60);
  return String(b);
}

String endClockText() {
  int m = minutesLeft();
  if (!isClockSynced || m < 0) return String("--:--");
  time_t now;
  time(&now);
  time_t fin = now + ((time_t)m * 60);
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", localtime(&fin));
  return String(buf);
}





// ============================================================================
// หน้า 1: IV BAG — ภาพถุงน้ำเกลือ + กระเปาะหยด + ตัวเลขสำคัญ
// ============================================================================
#define BAG_X       12
#define BAG_Y       34
#define BAG_W       58
#define BAG_H       124
#define CH_X        26
#define CH_Y        176
#define CH_W        30
#define CH_H        46
#define DROP_X      41
#define RIGHT_X     84








void updatePage2Dynamic() {
  drawTopBar(false);

  String v1 = (targetRateHr > 0) ? String((int)(targetRateHr + 0.5f)) : String("--");
  String v2 = (totalPlanMl > 0) ? String((int)totalPlanMl) : String("--");
  String v3 = String((int)totalVolumeMl);
  String v4 = endClockText();

  if (v1 != cacheP2a) { cacheP2a = v1; drawStatCard(30,  "", v1, THEME_SKYBLUE, false); }
  if (v2 != cacheP2b) { cacheP2b = v2; drawStatCard(84,  "", v2, COLOR_WHITE,   false); }
  if (v3 != cacheP2c) { cacheP2c = v3; drawStatCard(138, "", v3, COLOR_WHITE,   false); }
  if (v4 != cacheP2d) { cacheP2d = v4; drawStatCard(192, "", v4, UI_GREEN,      false); }

  String foot = timeLeftText(false) + " left";
  String foot2 = (totalPlanMl > 0)
      ? ("NEXT BAG AT " + String(safeNearPct(nearEndPct)) + "%")
      : String("NEXT BAG AT --");
  String key = foot + foot2;
  if (key != cacheP2foot) {
    cacheP2foot = key;
    textCenterIn(padTo(foot, 12), 6, 160, 248, 2, COLOR_WHITE, THEME_BG);
    textCenterIn(padTo(foot2, 20), 6, 160, 274, 1, COLOR_ORANGE, THEME_BG);
  }
}



// ============================================================================
// หน้า 4: TREND — กราฟแท่งอัตราไหลย้อนหลัง 30 นาที (แท่งละ 1 นาที)
// ============================================================================
#define CHART_X    12
#define CHART_BARW 5
#define CHART_TOP  56
#define CHART_BOT  196

float trendValueAt(int i) {
  int idx = (trendHead + i) % TREND_BARS;
  return trendHas[idx] ? trendBin[idx] : 0.0f;
}

bool trendValidAt(int i) {
  int idx = (trendHead + i) % TREND_BARS;
  return trendHas[idx];
}

float niceScale(float v) {
  float s = 20.0f;
  while (s < v && s < 400.0f) s += (s < 100.0f) ? 10.0f : (s < 200.0f ? 20.0f : 50.0f);
  return s;
}




void updatePage4Dynamic() {
  drawTopBar(false);

  float sum = 0.0f;
  int cnt = 0;
  for (int i = 1; i < TREND_BARS; i++) {
    if (!trendValidAt(i)) continue;
    sum += trendValueAt(i);
    cnt++;
  }
  int avg = (cnt > 0) ? (int)(sum / cnt + 0.5f) : -1;
  int now = isRunning ? (int)(currentFlowRate_ml_hr + 0.5f) : -1;

  String key = String(now) + ":" + String(avg);
  if (key != cacheP4) {
    cacheP4 = key;
    textAt(padTo(now >= 0 ? String(now) : String("--"), 4), 14, 234, 3, COLOR_WHITE, UI_CARD);
    textAt(padTo(avg >= 0 ? String(avg) : String("--"), 4), 100, 238, 2, THEME_CYAN, UI_CARD);
  }
  if (trendNeedsRedraw) drawTrendChart();
  else                  drawTrendLiveBar();
}

// ============================================================================
// เก็บข้อมูลกราฟแนวโน้ม (ทำงานตลอดเวลา ไม่ว่าจะเปิดหน้าไหนอยู่)
// ============================================================================
void resetTrendData() {
  for (int i = 0; i < TREND_BARS; i++) { trendBin[i] = 0.0f; trendHas[i] = false; }
  trendHead = 0;
  trendSum = 0.0f;
  trendSamples = 0;
  trendLive = 0.0f;
  trendBinStart = millis();
  trendNeedsRedraw = true;
}

void updateTrendAccumulator(unsigned long now) {
  if (now - trendLastSample >= 1000) {
    trendLastSample = now;
    trendSum += isRunning ? currentFlowRate_ml_hr : 0.0f;
    trendSamples++;
    trendLive = (trendSamples > 0) ? (trendSum / trendSamples) : 0.0f;
  }
  if (now - trendBinStart >= TREND_BIN_MS) {
    trendBinStart = now;
    trendBin[trendHead] = (trendSamples > 0) ? (trendSum / trendSamples) : 0.0f;
    trendHas[trendHead] = true;
    trendHead = (trendHead + 1) % TREND_BARS;
    trendSum = 0.0f;
    trendSamples = 0;
    trendLive = 0.0f;
    trendNeedsRedraw = true;
  }
}




void updateNearEndNoticeDynamic() {
  int pct = infusedPct();
  int left = remainingMlInt();
  String end = endClockText();
  String key = String(pct) + ":" + String(left) + ":" + end;
  if (key == cacheNear) return;
  cacheNear = key;

  textCenterIn(padTo("INFUSED " + String(pct < 0 ? 0 : pct) + "%", 12), 12, 148, 126, 2, COLOR_WHITE, THEME_BG);
  textCenterIn(padTo(String(left) + " mL LEFT", 12), 12, 148, 152, 2, COLOR_ORANGE, THEME_BG);
  textCenterIn(padTo("EMPTY AT " + end, 18), 12, 148, 180, 1, COLOR_YELLOW, THEME_BG);
  drawProgressBar(26, 196, 120, 10, (pct < 0) ? 0 : pct, COLOR_ORANGE);
}





void saveScreenRotation() {
  stationPrefs.begin("st_cfg", false);
  stationPrefs.putUChar("rot", screenRotation);
  stationPrefs.end();
}

const char *screenRotationLabel() {
  return (screenRotation == TFT_ROT_FLIP180) ? "FLIP 180" : "NORMAL";
}



// ----------------------------------------------------------------------------
// ปุ่มกด
// ----------------------------------------------------------------------------
void handleButton() {
  static unsigned long btnPressStart = 0;
  static bool isHolding = false;
  static int clickCount = 0;
  static unsigned long lastReleaseTime = 0;

  bool reading = (digitalRead(BTN_PIN) == LOW);
  unsigned long now = millis();

  if (reading && !isHolding) {
    btnPressStart = now;
    isHolding = true;
    isHoldUiActive = false;
    holdBeep3sDone = false;
    holdBeep5sDone = false;
    lastUserActivityTime = now;
  }
  else if (reading && isHolding) {
    unsigned long holdDur = now - btnPressStart;
    if (holdDur >= 400 && currentState != STATE_CONFIG_ID) {
      isHoldUiActive = true;
      if (holdDur >= 3000 && !holdBeep3sDone) {
        beepNonBlocking(2200, 50);
        holdBeep3sDone = true;
      }
      if (holdDur >= 5000 && !holdBeep5sDone) {
        beepNonBlocking(2600, 80);
        holdBeep5sDone = true;
      }
      if (now - lastHoldRenderTime >= 35) {
        lastHoldRenderTime = now;
        drawHoldProgressHUD(holdDur);
      }
    }
  }
  else if (!reading && isHolding) {
    unsigned long pressDur = now - btnPressStart;
    isHolding = false;
    lastUserActivityTime = now;

    if (isHoldUiActive) {
      isHoldUiActive = false;
      clickCount = 0;
      if (pressDur >= 5000) {
        soundScreensaver();
        currentState = STATE_CREDIT;
        stateNeedsRedraw = true;
      } else if (pressDur >= 3000) {
        executeButtonCalibrationWizard();
      } else {
        soundClick();
        stateNeedsRedraw = true;
      }
      return;
    }
    else if (pressDur > 30 && pressDur < 400) {
      clickCount++;
      lastReleaseTime = now;
    }
  }

  if (clickCount > 0 && !isHolding && (now - lastReleaseTime > 280)) {
    int clicks = clickCount;
    clickCount = 0;

    if (currentState == STATE_CONFIG_ID) {
      soundClick();
      if (clicks >= 2) {
        // ดับเบิลคลิก = สลับจอกลับหัว 180 องศา เห็นผลทันที แล้วจำค่าไว้เลย
        applyScreenRotation(screenRotation == TFT_ROT_FLIP180 ? TFT_ROT_NORMAL
                                                              : TFT_ROT_FLIP180);
        saveScreenRotation();
      } else {
        tempConfigStationId = (tempConfigStationId % 8) + 1;
      }
      configAutoSaveTimeout = now + 4000;
      return;
    }

    if (currentState == STATE_EMERGENCY) {
      if (clicks >= 2) {
        togglePause();                         // หยุดนับ เช่น ขณะเปลี่ยนถุง/แก้สายพับ
      } else {
        snoozeUntilTime = now + SNOOZE_MS;
        noTone(BUZZER_PIN);
        buzzerBeepUntil = 0;
        soundClick();
      }
      currentState = STATE_NORMAL_VIEW;
      stateNeedsRedraw = true;
      return;
    }

    if (currentState == STATE_NEAR_END_NOTICE) {
      acknowledgeNearEnd();                    // คลิกกี่ครั้งก็ได้ = รับทราบ
      currentState = STATE_NORMAL_VIEW;
      currentNursePage = 1;
      stateNeedsRedraw = true;
      return;
    }

    if (currentState == STATE_CREDIT) {
      soundClick();
      currentState = STATE_NORMAL_VIEW;
      stateNeedsRedraw = true;
      return;
    }

    if (clicks >= 3) {
      soundClick();
      noTone(BUZZER_PIN);
      tempConfigStationId = currentStationId;
      configAutoSaveTimeout = now + 4000;
      currentState = STATE_CONFIG_ID;
      stateNeedsRedraw = true;
    }
    else if (clicks == 2) {
      togglePause();
      if (currentState == STATE_SCREENSAVER) currentState = STATE_NORMAL_VIEW;
    }
    else {
      soundClick();
      if (currentState == STATE_SCREENSAVER) {
        currentState = STATE_NORMAL_VIEW;
      } else {
        currentNursePage = (currentNursePage % 4) + 1;
      }
      stateNeedsRedraw = true;
    }
  }
}

// ---------------------------------------------------------------------------
// โค้ดวาดจอทั้งหมดถูกแยกไปไว้ที่ไฟล์นี้ เพื่อให้ไฟล์หลักสั้นลงและหาของเจอเร็วขึ้น
// ต้อง #include ตรงนี้เท่านั้น คือหลังตัวแปรและฟังก์ชันช่วยทั้งหมด แต่ก่อน setup()
// ห้ามย้ายขึ้นไปบนสุด เพราะโค้ดในไฟล์นั้นใช้ตัวแปรและฟังก์ชันที่ประกาศไว้ด้านบน
// ---------------------------------------------------------------------------
#include "StationScreen.h"

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  setenv("TZ", "ICT-7", 1);
  tzset();

  pinMode(BUZZER_PIN, OUTPUT); noTone(BUZZER_PIN);
  pinMode(TFT_BLK, OUTPUT); digitalWrite(TFT_BLK, HIGH);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(SENSOR_AO_PIN, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  resetSignalChain(readSensorAverage(16));

  SPI_TFT.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(172, 320);
  tft.setSPISpeed(40000000);
  tft.setRotation(TFT_ROTATION_DEFAULT);
  tft.setTextWrap(false);

  stationPrefs.begin("st_cfg", false);
  currentStationId = stationPrefs.getUChar("id", 1);
  savedChannel     = stationPrefs.getUChar("ch", DEFAULT_ESPNOW_CHANNEL);
  uint8_t savedRot = stationPrefs.getUChar("rot", TFT_ROTATION_DEFAULT);
  stationPrefs.end();
  applyScreenRotation(savedRot);   // ทิศจอที่เคยตั้งไว้ที่หน้าเครื่อง
  if (currentStationId < 1 || currentStationId > 8) currentStationId = 1;

  // ---- กันสเตชันหลายตัวส่งชนกัน ----
  // เมล็ดสุ่มมาจาก MAC (ไม่ซ้ำกันทุกบอร์ด) และรอบแรกเหลื่อมกันตามเลขเตียง
  // ทำให้แพ็กเก็ตของแต่ละเตียงกระจายกันทั้งวินาทีตั้งแต่บูต ไม่ต้องรอให้ jitter ค่อย ๆ แยก
  {
    uint8_t mac[6] = {0};
    WiFi.macAddress(mac);
    randomSeed(((uint32_t)mac[5] << 16) ^ ((uint32_t)mac[4] << 8) ^ (uint32_t)mac[3]
               ^ ((uint32_t)currentStationId << 24) ^ (uint32_t)micros());
    lastSendTime  = millis();
    nextSendDelay = (unsigned long)currentStationId * (SEND_INTERVAL_MS / 9);
  }
  if (savedChannel < 1 || savedChannel > MAX_WIFI_CHANNEL) savedChannel = DEFAULT_ESPNOW_CHANNEL;

  stationPrefs.begin("st_cal", true);
  dropPolarity     = (int8_t)stationPrefs.getInt("pol", -1);
  dropTriggerDelta = stationPrefs.getInt("trig",  CAL_DEFAULT_TRIGGER);
  dropReleaseDelta = stationPrefs.getInt("rel",   CAL_DEFAULT_TRIGGER * 2 / 5);
  calDropAmp       = stationPrefs.getInt("amp",   0);
  calNoisePp       = stationPrefs.getInt("noise", 0);
  calSnrX10        = stationPrefs.getUChar("snr", 0);
  calQuality       = stationPrefs.getUChar("q",   0);
  learnAmp         = stationPrefs.getInt("lamp",  0);
  learnWidth       = stationPrefs.getInt("lwid",  0);
  stationPrefs.end();
  if (learnWidth < DROP_MIN_WIDTH_MS || learnWidth > DROP_MAX_WIDTH_MS) learnWidth = 0;
  if (learnAmp < 0) learnAmp = 0;
  if (dropPolarity != 1) dropPolarity = -1;
  if (dropTriggerDelta < CAL_MIN_TRIGGER) dropTriggerDelta = CAL_DEFAULT_TRIGGER;
  if (dropReleaseDelta < 2 || dropReleaseDelta >= dropTriggerDelta)
    dropReleaseDelta = dropTriggerDelta * 2 / 5;

  setLedColor(0, 40, 80);
  resetTrendData();
  trendLastSample = millis();

  tft.fillScreen(THEME_BG);
  drawCenteredString("SMART IV", 60, 3, THEME_SKYBLUE, THEME_BG);
  drawCenteredString("ALERT", 92, 2, THEME_PINK, THEME_BG);

  char bedBuf[12];
  snprintf(bedBuf, sizeof(bedBuf), "BED %02d", currentStationId);
  tft.fillRoundRect(26, 140, 120, 48, 8, THEME_SKYBLUE);
  drawCenteredString(String(bedBuf), 154, 3, THEME_BG, THEME_SKYBLUE);

  drawCenteredString("Bed Station Node", 214, 1, UI_DIM, THEME_BG);
  drawCenteredString("FW v" APP_VERSION, 232, 1, THEME_SKYBLUE, THEME_BG);
  drawCenteredString("BCN Phrae Innovation", 256, 1, UI_DIM, THEME_BG);

  playWelcomeMelody();
  delay(600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  setEspNowChannel(savedChannel);

  if (esp_now_init() != ESP_OK) {
    drawCenteredString("ESP-NOW INIT FAILED", 270, 1, COLOR_RED, THEME_BG);
    Serial.println("[STATION] ESP-NOW init FAILED");
  } else {
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnTimeSyncRecv));

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;              // 0 = ใช้ช่องปัจจุบัน (รองรับการเปลี่ยนช่องระหว่างค้นหา Host)
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
  }

  currentState = STATE_NORMAL_VIEW;
  currentNursePage = 1;
  stateNeedsRedraw = true;
  lastUserActivityTime = millis();
  lastChannelHopTime = millis();
}

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------
void loop() {
  serviceDropSensor(isRunning);   // อ่านเซนเซอร์/เดินเครื่องจักรสถานะ (ไม่นับหยดตอนหยุดชั่วคราว)
  handleButton();

  unsigned long currentMillis = millis();

  if (pendingCounterReset) applyCounterReset();

  // ตรวจสอบการเชื่อมต่อกับ Host หากเกิน 5 วินาทีให้ถือว่าหลุด
  unsigned long lr = lastHostRecvTime;
  if (lr == 0 || (currentMillis > lr && currentMillis - lr > HOST_TIMEOUT_MS)) {
    isHostOnline = false;
  }
  manageChannelHunting(currentMillis);

  // สายพับ -> อัตราไหลเป็น 0
  if (isLocallyOccluded() && currentFlowRate_ml_hr != 0.0f) {
    currentFlowRate_ml_hr = 0.0f;
    currentGttMin = 0.0f;
    resetRateWindow();
  }
  totalVolumeMl = (float)totalDrops / (float)safeDropFactor(dropFactor);

  uint8_t alertCode = effectiveAlertCode();
  uiAlertCode = alertCode;                                // ให้ทุกหน้าจอใช้รหัสเดียวกัน
  updateTrendAccumulator(currentMillis);                  // เก็บข้อมูลกราฟตลอดเวลา
  if (!isCriticalAlert(alertCode)) snoozeUntilTime = 0;   // เหตุการณ์หายไปแล้ว ยกเลิก snooze
  handlePassiveBuzzerEngine(alertCode);
  updateStatusLed(alertCode);

  // ---- ส่งข้อมูลให้ Host ทุก 1 วินาที (ส่งแม้อยู่ในหน้าตั้งค่า) ----
  if (currentMillis - lastSendTime >= nextSendDelay) {
    myData.stationId       = currentStationId;
    myData.isRunning       = isRunning ? 1 : 0;
    myData.totalDrops      = totalDrops;
    myData.periodDrops     = periodDropsCounter;
    myData.flowRateHr      = isRunning ? currentFlowRate_ml_hr : 0.0f;
    myData.msSinceLastDrop = hasFirstDropOccurred ? (uint32_t)(currentMillis - lastDropTimestamp) : 0;
    if (myData.msSinceLastDrop == 0 && hasFirstDropOccurred) myData.msSinceLastDrop = 1;
    myData.batteryVolts    = readStationBattery();
    myData.flags           = nearEndAckRequest ? 0x01 : 0x00;

    if (esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData)) == ESP_OK) {
      periodDropsCounter = 0;
    }
    lastSendTime = currentMillis;
    // สุ่มคาบถัดไปใหม่เสมอ สองสเตชันที่บังเอิญส่งพร้อมกันจึงแยกจากกันในรอบถัดไปทันที
    nextSendDelay = (unsigned long)(SEND_INTERVAL_MS - SEND_JITTER_MS)
                  + (unsigned long)random(0, 2 * SEND_JITTER_MS + 1);
  }

  if (currentState == STATE_CONFIG_ID) {
    if (stateNeedsRedraw) drawStationIdConfigFramework();
    updateStationIdConfigDynamic();

    if (currentMillis >= configAutoSaveTimeout) {
      if (tempConfigStationId != currentStationId) {
        currentStationId = tempConfigStationId;
        stationPrefs.begin("st_cfg", false);
        stationPrefs.putUChar("id", currentStationId);
        stationPrefs.end();
        // ล้างค่าของเตียงเดิม รอค่าของเตียงใหม่จาก Host
        targetRateHr = 0;
        totalPlanMl = 0;
        hostAlertCode = ALERT_NONE;
        knownResetSeq = -1;
      }
      soundStart();
      currentState = STATE_NORMAL_VIEW;
      stateNeedsRedraw = true;
    }
    return;
  }

  // ---- จัดการสถานะเตือน ----
  bool critical = isCriticalAlert(alertCode);
  if (critical && currentMillis >= snoozeUntilTime) {
    if (currentState != STATE_EMERGENCY && !isHoldUiActive) {
      currentState = STATE_EMERGENCY;
      stateNeedsRedraw = true;
    } else if (currentState == STATE_EMERGENCY && alertCode != lastEmergencyCode) {
      stateNeedsRedraw = true;
    }
  } else if (currentState == STATE_EMERGENCY) {
    currentState = STATE_NORMAL_VIEW;
    stateNeedsRedraw = true;
  }

  // ---- เตือนใกล้หมด (ไม่ใช่เหตุวิกฤต) ----
  if (!critical && isNearEndPending(alertCode) && !isHoldUiActive &&
      (currentState == STATE_NORMAL_VIEW || currentState == STATE_SCREENSAVER)) {
    currentState = STATE_NEAR_END_NOTICE;
    stateNeedsRedraw = true;
  } else if (currentState == STATE_NEAR_END_NOTICE && (critical || !isNearEndPending(alertCode))) {
    currentState = critical ? STATE_EMERGENCY : STATE_NORMAL_VIEW;
    stateNeedsRedraw = true;
  }

  // ---- Screensaver เมื่อไม่มีการใช้งาน ----
  if (currentState == STATE_NORMAL_VIEW && !isHoldUiActive &&
      currentMillis - lastUserActivityTime >= SCREENSAVER_IDLE_MS) {
    currentState = STATE_SCREENSAVER;
    stateNeedsRedraw = true;
  }

  if (!isHoldUiActive) {
    if (currentState == STATE_EMERGENCY) {
      if (stateNeedsRedraw) { drawEmergencyScreen(alertCode); updateEmergencyDynamic(); }
    }
    else if (currentState == STATE_NORMAL_VIEW) {
      if (currentNursePage == 1) {
        if (stateNeedsRedraw) { drawPage1Framework(); updatePage1Dynamic(); }
        updateDripAnimation();
      } else if (currentNursePage == 2) {
        if (stateNeedsRedraw) { drawPage2Framework(); updatePage2Dynamic(); }
      } else if (currentNursePage == 3) {
        if (stateNeedsRedraw) { drawPage3Framework(); updatePage3Dynamic(); }
      } else if (currentNursePage == 4) {
        if (stateNeedsRedraw) { drawPage4Framework(); updatePage4Dynamic(); }
      }
    }
    else if (currentState == STATE_SCREENSAVER) {
      if (stateNeedsRedraw) { drawScreensaverFramework(); updateScreensaverDynamic(); }
    }
    else if (currentState == STATE_NEAR_END_NOTICE) {
      if (stateNeedsRedraw) { drawNearEndNoticeFramework(); updateNearEndNoticeDynamic(); }
    }
    else if (currentState == STATE_CREDIT) {
      if (stateNeedsRedraw) drawDeveloperCreditScreen();
    }
  }

  static unsigned long lastDynamicUpdate = 0;
  if (currentMillis - lastDynamicUpdate >= 1000) {
    lastDynamicUpdate = currentMillis;

    if (!isHoldUiActive) {
      if (currentState == STATE_NORMAL_VIEW) {
        if (currentNursePage == 1) updatePage1Dynamic();
        else if (currentNursePage == 2) updatePage2Dynamic();
        else if (currentNursePage == 3) updatePage3Dynamic();
        else if (currentNursePage == 4) updatePage4Dynamic();
      } else if (currentState == STATE_SCREENSAVER) {
        updateScreensaverDynamic();
      } else if (currentState == STATE_EMERGENCY) {
        updateEmergencyDynamic();
      } else if (currentState == STATE_NEAR_END_NOTICE) {
        updateNearEndNoticeDynamic();
      }
    }
  }
}
