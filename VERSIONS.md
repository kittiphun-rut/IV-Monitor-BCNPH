# สารบัญเวอร์ชัน (Release Index)

ทุกจุดที่เลขเวอร์ชันของสายใดสายหนึ่งขยับ ถูกชี้ตำแหน่งไว้ที่นี่
เพื่อให้ **ย้อนกลับไปใช้ชุดที่สมบูรณ์ชุดไหนก็ได้** โดยไม่ต้องไล่หาคอมมิตเอง

หนึ่งรุ่นคือ **เฟิร์มแวร์ครบทั้งชุด** ทั้งเครื่องประจำเตียงและเครื่องส่วนกลาง
ที่อยู่ด้วยกัน ณ เวลานั้น — ไม่ควรจับคู่ข้ามรุ่น เพราะโครงสร้างแพ็กเก็ต ESP-NOW
(Protocol v3) และรหัสสัญญาณเตือนเปลี่ยนไปในบางรุ่น

ทุกรุ่นถูก push ขึ้น GitHub ไว้เป็น **branch** ใต้โฟลเดอร์ `release/`
กดโหลดเป็นไฟล์ zip จากตารางข้างล่างได้เลย ไม่ต้องใช้คำสั่ง git

## สายผลิตภัณฑ์

| สาย | โฟลเดอร์บน `main` | อุปกรณ์ | จอ |
|---|---|---|---|
| Station | `firmware/Station/` | เครื่องประจำเตียง | TFT ในตัว |
| Station สายเสถียร | `firmware/Station-Stable/` | เครื่องประจำเตียง รุ่นอัลกอริทึมเดิม | TFT ในตัว |
| Station บอร์ดเล็ก | `firmware/Station-C3-OLED/` | เครื่องประจำเตียง บอร์ด ESP32-C3 | OLED 0.42" 72×40 |
| Host OLED | `firmware/Host-OLED/` | เครื่องส่วนกลาง | SH1106 128×64 |
| Host TFT 2.8" | `firmware/Host-TFT28/` | เครื่องส่วนกลาง | ST7789V 320×240 แนวนอน |
| Host สัมผัส 2.4" | `firmware/Host-Touch24/` | เครื่องส่วนกลาง | ILI9341 + XPT2046 240×320 แนวตั้ง |

เลือก Host **หนึ่งตัว** ตามชนิดจอที่มี ใช้คู่กับ Station สายใดสายหนึ่ง

## โครงสร้าง branch

```
main                          branch หลัก งานล่าสุดทั้งหมดอยู่ที่นี่
release/00-h4.7.0              หมุดเวลาของแต่ละรุ่น ห้ามแก้ ห้าม push ทับ
release/01-s7.5.0-h4.7.0       
release/02-s7.5.0-h4.7.0-tft4.8.0 
release/03-s7.5.0-h4.7.0-touch4.9.0 
release/04-s7.6.0-h4.7.0       
release/05-s7.6.0-h4.7.0-lts7.5.1 
release/06-s7.7.0-h4.7.0       
release/07-s7.7.0-h4.7.1       
release/08-s7.7.0-h4.7.2       
release/09-s7.7.1-h4.7.2       
release/10-s7.7.1-h4.7.2-lts7.5.2 
release/11-s7.7.2-h4.7.3-lts7.5.3 
release/12-s7.7.2-h4.7.4       
release/13-s7.7.3-h4.7.5-lts7.5.4 
release/14-s7.7.3-h4.7.5-touch4.9.2 
release/15-s7.7.3-h4.7.6       
release/16-s7.8.0-h4.7.7-lts7.5.5-tft4.8.1-touch4.9.3 
release/17-s7.8.0-h4.7.7-c3v1.0.0 
```

> **branch เหล่านี้ห้ามแก้และห้าม push ทับ** ให้ถือเป็นหมุดเวลาอย่างเดียว
> งานที่พัฒนาต่ออยู่ที่ `main`
>
> ปกติงานลักษณะนี้ควรเป็น **tag** ไม่ใช่ branch แต่บัญชีที่เซสชันนี้ใช้
> push `refs/tags/*` และลบ ref ไม่ได้ เซิร์ฟเวอร์ตอบ 403 จึงใช้ branch แทน
> ได้ผลเหมือนกันทุกอย่างสำหรับการย้อนกลับ

## ตารางรุ่น

| ชื่อรุ่น | Station | สายเสถียร | บอร์ดเล็ก | OLED | TFT 2.8" | สัมผัส 2.4" | คอมไพล์ | โหลด zip | คอมมิต | สาระสำคัญ |
|---|---|---|---|---|---|---|---|---|---|---|
| `release/00-h4.7.0` | — | — | — | 4.7.0 | — | — | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/00-h4.7.0.zip) | [`676b321`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/676b3212e3802170e3a465e4a2b30d57e656920e) | host: AP-only mode (remove Wi-Fi management) + station UI concepts<br><sub>16/9/2026</sub> |
| `release/01-s7.5.0-h4.7.0` | — | 7.5.0 | — | 4.7.0 | — | — | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/01-s7.5.0-h4.7.0.zip) | [`4a7dc3b`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/4a7dc3b69df2e936c5aa88662564b3b79e162c23) | station: redesign TFT UI (IV BAG concept) + 30-min flow trend page<br><sub>12/9/2026</sub> |
| `release/02-s7.5.0-h4.7.0-tft4.8.0` | — | 7.5.0 | — | 4.7.0 | 4.8.0 | — | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/02-s7.5.0-h4.7.0-tft4.8.0.zip) | [`f21c6d0`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/f21c6d024162cf8494244af1e6ed526395f204c0) | host: new 2.8" TFT build (v4.8.0-TFT) with FOCUS dashboard<br><sub>12/9/2026</sub> |
| `release/03-s7.5.0-h4.7.0-touch4.9.0` | — | 7.5.0 | — | 4.7.0 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/03-s7.5.0-h4.7.0-touch4.9.0.zip) | [`ef8f433`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/ef8f4330fb822c86399dc377f36e742e591f3931) | host: touch-screen build v4.9.0-TOUCH (2.4" 240x320 + XPT2046)<br><sub>12/9/2026</sub> |
| `release/04-s7.6.0-h4.7.0` | 7.6.0 | 7.5.0 | — | 4.7.0 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/04-s7.6.0-h4.7.0.zip) | [`4aec24a`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/4aec24a05f957f75e47072860ebf75379ee47ad6) | station: รองรับเซ็นเซอร์หยดแบบเอาต์พุตดิจิทัล + ผังขาใหม่ (v7.6.0)<br><sub>12/9/2026</sub> |
| `release/05-s7.6.0-h4.7.0-lts7.5.1` | 7.6.0 | 7.5.1 | — | 4.7.0 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/05-s7.6.0-h4.7.0-lts7.5.1.zip) | [`c27be5a`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/c27be5a8bc8c43b8abffe6734a1b41f7dbb1efe8) | station: รื้อระบบคัดกรองหยดและหน้าคาลิเบรตใหม่ (v7.5.1)<br><sub>12/9/2026</sub> |
| `release/06-s7.7.0-h4.7.0` | 7.7.0 | 7.5.1 | — | 4.7.0 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/06-s7.7.0-h4.7.0.zip) | [`eba3f61`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/eba3f6159a6058cffcdaab578883022210cdead0) | station: ไม่ต้องคาลิเบรตด้วยมืออีกต่อไป (v7.7.0)<br><sub>12/9/2026</sub> |
| `release/07-s7.7.0-h4.7.1` | 7.7.0 | 7.5.1 | — | 4.7.1 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/07-s7.7.0-h4.7.1.zip) | [`5140a94`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/5140a94dd1674dc8e5fc29161bc16ad3c665de76) | host(oled): รองรับ Station v7.5.1 และ v7.7.0 + เตือนเซนเซอร์ยังไม่จับหยด (v4.7.1-OLED)<br><sub>12/9/2026</sub> |
| `release/08-s7.7.0-h4.7.2` | 7.7.0 | 7.5.1 | — | 4.7.2 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/08-s7.7.0-h4.7.2.zip) | [`1b1dd84`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/1b1dd843a6be87c5a37b6031f102a3d4e948c51f) | host(oled): ปรับหน้าเว็บ Dashboard ตามที่พยาบาลขอ (v4.7.2-OLED)<br><sub>18/9/2026</sub> |
| `release/09-s7.7.1-h4.7.2` | 7.7.1 | 7.5.1 | — | 4.7.2 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/09-s7.7.1-h4.7.2.zip) | [`cb57ea3`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/cb57ea3d27d0f5cd4bee714a0accb25805179ca8) | Station v7.7.1: หมุนจอ 180 องศาได้ที่หน้าเครื่อง<br><sub>12/9/2026</sub> |
| `release/10-s7.7.1-h4.7.2-lts7.5.2` | 7.7.1 | 7.5.2 | — | 4.7.2 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/10-s7.7.1-h4.7.2-lts7.5.2.zip) | [`17604ca`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/17604cad2455dc4c06e61458a6bf154df82f54d0) | Station v7.5.2: พอร์ตการหมุนจอ 180 องศาจาก v7.7.1 ลงสายเสถียร<br><sub>19/9/2026</sub> |
| `release/11-s7.7.2-h4.7.3-lts7.5.3` | 7.7.2 | 7.5.3 | — | 4.7.3 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/11-s7.7.2-h4.7.3-lts7.5.3.zip) | [`319fc45`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/319fc4530a96b1a08f48cadaf8da07710bb716dc) | Host v4.7.3 + Station v7.7.2/v7.5.3: แก้อาการ Host ขึ้น OFFLINE เมื่อมีหลายเตียง<br><sub>12/9/2026</sub> |
| `release/12-s7.7.2-h4.7.4` | 7.7.2 | 7.5.3 | — | 4.7.4 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/12-s7.7.2-h4.7.4.zip) | [`ea20bb9`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/ea20bb9b2056d3e375842852c4df93927f03b565) | Host v4.7.4: ตรวจจับเลขเตียงซ้ำ + จอ OLED ที่อ่านง่ายขึ้น<br><sub>20/9/2026</sub> |
| `release/13-s7.7.3-h4.7.5-lts7.5.4` | 7.7.3 | 7.5.4 | — | 4.7.5 | 4.8.0 | 4.9.0 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/13-s7.7.3-h4.7.5-lts7.5.4.zip) | [`35ce57b`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/35ce57b99d51794fc28af44b2b59410ef8549180) | แก้สูตรคำนวณอัตราการไหลและบั๊กฝั่ง Host (Station 7.7.3/7.5.4, Host 4.7.5)<br><sub>12/9/2026</sub> |
| `release/14-s7.7.3-h4.7.5-touch4.9.2` | 7.7.3 | 7.5.4 | — | 4.7.5 | 4.8.0 | 4.9.2 | ❔ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/14-s7.7.3-h4.7.5-touch4.9.2.zip) | [`8fdd598`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/8fdd598949b176f7e87b95b8bc019397581d9f64) | พอร์ตคุณสมบัติทั้งหมดจากรุ่นจอ OLED ลงรุ่นจอสัมผัส (Touch v4.9.2)<br><sub>23/9/2026</sub> |
| `release/15-s7.7.3-h4.7.6` | 7.7.3 | 7.5.4 | — | **4.7.6** | 4.8.0 | 4.9.2 | ✅ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/15-s7.7.3-h4.7.6.zip) | [`ae0d061`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/ae0d061bd8abee1b0439f1e515dd912d57e068eb) | **นำการเชื่อมต่อเราเตอร์กลับมา** เกาะ Wi-Fi โรงพยาบาลเป็นหลักแล้วปิด AP เพื่อลดภาระซีพียู ต่อไม่ได้เปิด AP สำรองเอง · ปล่อยให้ ESP-NOW ใช้ช่องของเราเตอร์ (Station ไล่หาช่องเอง) · ค้นหาเครือข่ายแบบไม่บล็อก · หน้าเว็บปรับจังหวะรีเฟรชตามจำนวนผู้ใช้<br><sub>23/9/2026</sub> |
| `release/16-s7.8.0-h4.7.7-lts7.5.5-tft4.8.1-touch4.9.3` | **7.8.0** | **7.5.5** | — | **4.7.7** | **4.8.1** | **4.9.3** | ✅ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/16-s7.8.0-h4.7.7-lts7.5.5-tft4.8.1-touch4.9.3.zip) | [`d58537c`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/d58537caa7ec975e28701feaec180c76a3fda8d9) | **แยกโค้ดวาดจอออกจากไฟล์หลักทั้ง 5 สาย** ไปไว้ใน `StationScreen.h` / `HostScreen.h` ย้าย 2,261 บรรทัด เป็นการย้ายที่อยู่ล้วน ๆ พิสูจน์ด้วยการเทียบภาพหน้าจอก่อน-หลังทีละพิกเซล เหมือนกันครบ 87 ภาพ · ตรึงนาฬิกาในตัวจำลองให้เรนเดอร์ซ้ำได้ · เพิ่ม `tools/split-check/`<br><sub>23/9/2026</sub> |
| `release/17-s7.8.0-h4.7.7-c3v1.0.0` | 7.8.0 | 7.5.5 | **1.0.0** | 4.7.7 | 4.8.1 | 4.9.3 | ✅ | [zip](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/archive/refs/heads/release/17-s7.8.0-h4.7.7-c3v1.0.0.zip) | [`28d6fd3`](https://github.com/kittiphun-rut/IV-Monitor-BCNPH/tree/28d6fd36de6098cc6927da403a5badf8776984fc) | **เพิ่มสายเครื่องประจำเตียงบอร์ดเล็ก** ESP32-C3 + จอ OLED 0.42" 72x40 ปรับจากไฟล์ v4.4.2 เดิมที่คุยกับ Host ไม่ได้ (แพ็กเก็ต 14 ไบต์ ต้องเป็น 23) · ไล่หาช่องสัญญาณเองแบบสายหลัก · ใช้ drop_detector.h ตัวเดียวกัน · หน้าจอออกแบบใหม่ทั้งหมดสำหรับ 72x40<br><sub>23/9/2026</sub> |

**คอลัมน์คอมไพล์**
✅ = ชุดตรวจ ๙ ตัวใน `tools/` รันผ่านครบ ณ commit นั้น (คอมไพล์ด้วย g++ พร้อมตัวจำลอง
Arduino บน PC ไม่ใช่ Arduino IDE จริง) และเรนเดอร์หน้าเว็บด้วย Chromium จริงแล้ว
❔ = ยังไม่ได้ไล่ตรวจย้อนหลังทีละรุ่น

## ข้อควรทราบเรื่องโครงไฟล์ของรุ่นเก่า

รุ่น `00` ถึง `14` เกิดขึ้น **ก่อน** การจัดโครงโฟลเดอร์ใหม่ ตอนนั้นรีโพเก็บ
เฟิร์มแวร์เป็นโฟลเดอร์แยกตามเลขเวอร์ชัน (`firmware/ESP32-S3-Station-V_7_7_3/`)
branch ของรุ่นเหล่านั้นจึงยังเป็นโครงเดิม ซึ่งถูกต้องแล้ว เพราะเป็นภาพจริง
ของรีโพ ณ ตอนนั้น ไม่ได้ดัดแปลงย้อนหลัง

ตั้งแต่รุ่นถัดจาก `14` เป็นต้นไป โครงจะเป็นแบบใหม่คือโฟลเดอร์ชื่อคงที่สายละหนึ่ง

## เพิ่มรุ่นใหม่

1. แก้เฟิร์มแวร์ในโฟลเดอร์เดิม **ไม่ต้องสร้างโฟลเดอร์ใหม่** แล้วเลื่อน `APP_VERSION`
2. รันชุดตรวจใน `CLAUDE.md` ให้ผ่านครบ
3. commit ลง `main`
4. สร้าง branch `release/<ลำดับถัดไป>-s<station>-h<host>` ที่คอมมิตนั้น แล้ว push
5. เติมแถวในตารางข้างบน

