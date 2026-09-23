# สารบัญเวอร์ชันเฟิร์มแวร์

ทุกเวอร์ชันมีแบรนช์ของตัวเอง แต่ละแบรนช์มีเฟิร์มแวร์ชุดเดียว
และมี `VERSION.md` ที่รากบอกที่มาที่ไปของเวอร์ชันนั้น

แบ่งตาม**ชนิดของหน้าจอ** เพราะนั่นคือสิ่งเดียวที่ทำให้สายเวอร์ชันต่างกัน

## `station` — เครื่องประจำเตียง

เครื่องประจำเตียง (จอ TFT ในตัว)

| เวอร์ชัน | แบรนช์ | งานที่ทำในเวอร์ชันนี้ |
|---|---|---|
| 7.5.0 | [`release/station/v7.5.0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.5.0) | fix(firmware): ย้ายการประกาศ enum/struct ขึ้นต้นไฟล์ ให้ Arduino IDE คอมไพล์ผ่าน |
| 7.5.1 | [`release/station/v7.5.1`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.5.1) | station: แก้การวาดกราฟทำให้ไม่นับหยด + เรียนรู้หยดต่อเนื่อง + กันถุงแกว่ง (v7.5.1) |
| 7.5.2 | [`release/station/v7.5.2`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.5.2) | Station v7.5.2: พอร์ตการหมุนจอ 180 องศาจาก v7.7.1 ลงสายเสถียร |
| 7.5.3 | [`release/station/v7.5.3`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.5.3) | Host v4.7.3 + Station v7.7.2/v7.5.3: แก้อาการ Host ขึ้น OFFLINE เมื่อมีหลายเตียง |
| 7.5.4 | [`release/station/v7.5.4`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.5.4) | แก้สูตรคำนวณอัตราการไหลและบั๊กฝั่ง Host (Station 7.7.3/7.5.4, Host 4.7.5) |
| 7.6.0 | [`release/station/v7.6.0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.6.0) | station: รองรับเซ็นเซอร์หยดแบบเอาต์พุตดิจิทัล + ผังขาใหม่ (v7.6.0) |
| 7.7.0 | [`release/station/v7.7.0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.7.0) | station: ไม่ต้องคาลิเบรตด้วยมืออีกต่อไป (v7.7.0) |
| 7.7.1 | [`release/station/v7.7.1`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.7.1) | Station v7.7.1: หมุนจอ 180 องศาได้ที่หน้าเครื่อง |
| 7.7.2 | [`release/station/v7.7.2`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.7.2) | Host v4.7.3 + Station v7.7.2/v7.5.3: แก้อาการ Host ขึ้น OFFLINE เมื่อมีหลายเตียง |
| 7.7.3 | [`release/station/v7.7.3`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/station/v7.7.3) | แก้สูตรคำนวณอัตราการไหลและบั๊กฝั่ง Host (Station 7.7.3/7.5.4, Host 4.7.5) |

ล่าสุด: **v7.7.3** — `firmware/ESP32-S3-Station-V_7_7_3/`

## `oled` — Host จอ OLED

OLED SH1106 128x64 (ขาว-น้ำเงิน)

| เวอร์ชัน | แบรนช์ | งานที่ทำในเวอร์ชันนี้ |
|---|---|---|
| 4.7.0 | [`release/oled/v4.7.0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/oled/v4.7.0) | host: move the web dashboard out of the .ino into web_dashboard.h |
| 4.7.1 | [`release/oled/v4.7.1`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/oled/v4.7.1) | host(oled): รองรับ Station v7.5.1 และ v7.7.0 + เตือนเซนเซอร์ยังไม่จับหยด (v4.7.1-OLED) |
| 4.7.2 | [`release/oled/v4.7.2`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/oled/v4.7.2) | เพิ่มเครื่องมือจำลองหน้าจอ OLED ของ Host v4.7.2 บน PC |
| 4.7.3 | [`release/oled/v4.7.3`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/oled/v4.7.3) | Host v4.7.3 + Station v7.7.2/v7.5.3: แก้อาการ Host ขึ้น OFFLINE เมื่อมีหลายเตียง |
| 4.7.4 | [`release/oled/v4.7.4`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/oled/v4.7.4) | Host v4.7.4: ตรวจจับเลขเตียงซ้ำ + จอ OLED ที่อ่านง่ายขึ้น |
| 4.7.5 | [`release/oled/v4.7.5`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/oled/v4.7.5) | แก้สูตรคำนวณอัตราการไหลและบั๊กฝั่ง Host (Station 7.7.3/7.5.4, Host 4.7.5) |

ล่าสุด: **v4.7.5** — `firmware/ESP32-S3-Host-OLED-V_4_7_5/`

## `tft28` — Host จอ TFT 2.8"

TFT สี 2.8" ST7789V 320x240 แนวนอน

| เวอร์ชัน | แบรนช์ | งานที่ทำในเวอร์ชันนี้ |
|---|---|---|
| 4.8.0 | [`release/tft28/v4.8.0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/tft28/v4.8.0) | fix(firmware): ย้ายการประกาศ enum/struct ขึ้นต้นไฟล์ ให้ Arduino IDE คอมไพล์ผ่าน |

ล่าสุด: **v4.8.0** — `firmware/ESP32-S3-Host-TFT-V_4_8_0/`

## `touch24` — Host จอสัมผัส 2.4"

TFT สัมผัส 2.4" ILI9341 + XPT2046 240x320 แนวตั้ง

| เวอร์ชัน | แบรนช์ | งานที่ทำในเวอร์ชันนี้ |
|---|---|---|
| 4.9.0 | [`release/touch24/v4.9.0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/touch24/v4.9.0) | fix(firmware): ย้ายการประกาศ enum/struct ขึ้นต้นไฟล์ ให้ Arduino IDE คอมไพล์ผ่าน |
| 4.9.2 | [`release/touch24/v4.9.2`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/release/touch24/v4.9.2) | พอร์ตคุณสมบัติทั้งหมดจากรุ่นจอ OLED ลงรุ่นจอสัมผัส (Touch v4.9.2) |

ล่าสุด: **v4.9.2** — `firmware/ESP32-S3-Host-Touch-V_4_9_2/`

## เพิ่มเวอร์ชันใหม่

ดูขั้นตอนที่ `CLAUDE.md` หัวข้อ "การออกเวอร์ชันและแบรนช์" โดยสรุปคือ

1. สร้างโฟลเดอร์เวอร์ชันใหม่ ห้ามแก้ทับของเดิม
2. หาคอลเลกชันจากชนิดจอที่ใช้ ถ้าเป็นจอชนิดใหม่ให้เพิ่มคอลเลกชันใหม่
3. สร้างแบรนช์ `release/<คอลเลกชัน>/v<เวอร์ชัน>` ที่ commit ล่าสุดซึ่งแตะโฟลเดอร์นั้น
4. บนแบรนช์นั้น ตัดเฟิร์มแวร์เวอร์ชันอื่นออกให้หมด เหลือชุดเดียว แล้วเพิ่ม `VERSION.md`
5. อัปเดตตารางในไฟล์นี้

