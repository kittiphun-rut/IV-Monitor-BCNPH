# -*- coding: utf-8 -*-
"""ประกอบภาพหน้าจอของ Station-C3-OLED เป็นแผ่นรวมพร้อมคำอธิบายภาษาไทย
และต่อเฟรมภาพเคลื่อนไหวเป็นไฟล์ GIF

จอจริงกว้าง 72 สูง 40 พิกเซล ภาพที่เรนเดอร์มาขยายไว้ 8 เท่า (576x320)
แผ่นรวมย่อลงครึ่งหนึ่งเป็น 4 เท่า (288x160) ซึ่งเป็นอัตราส่วนจำนวนเต็ม
ภาพจึงยังคมเหมือนเดิม ไม่เบลอแบบการย่อด้วยอัตราส่วนที่ไม่ลงตัว

ใช้:  python3 tools/c3-preview/make_overview.py [โฟลเดอร์ภาพ]
"""
import os
import sys
import glob
from PIL import Image, ImageDraw, ImageFont

# ภาษาไทยไม่มีช่องว่างระหว่างคำ ถ้าตัดบรรทัดด้วยการนับตัวอักษรจะได้คำขาดกลางคำ
# เช่น "ตามปก|ติ" ซึ่งอ่านสะดุด ใช้ตัวตัดคำของ pythainlp ถ้ามี ถ้าไม่มีก็ถอยไป
# ตัดทีละตัวอักษรเหมือนเดิม เครื่องมือนี้จึงไม่ผูกติดกับไลบรารีนั้น
try:
    from pythainlp.tokenize import word_tokenize as _th_tokenize
except Exception:
    _th_tokenize = None

OUT = sys.argv[1] if len(sys.argv) > 1 else "docs/screens-c3"

TH_R = "/usr/local/share/fonts/thsarabun/THSarabunNew-webfont.ttf"
TH_B = "/usr/local/share/fonts/thsarabun/THSarabunNew_bold-webfont.ttf"
LAYOUT = ImageFont.Layout.RAQM          # ต้องใช้ RAQM มิฉะนั้นสระบนและวรรณยุกต์จะลอยผิดที่

BG      = (18, 20, 24)
FG      = (232, 234, 238)
DIM     = (150, 156, 166)
ACCENT  = (96, 178, 255)
LINE    = (52, 56, 64)

SW, SH  = 288, 160                      # ขนาดจอในแผ่นรวม (4 เท่าของจอจริง)
COLS    = 4
GAP     = 26
CAP_GAP = 8                             # ระยะจากภาพถึงหัวข้อ
MARGIN  = 34

TITLE = "หน้าจอจำลองของเครื่องประจำเตียงรุ่นบอร์ดเล็ก"
SUBTITLE = ("ESP32-C3 Super Mini + จอ OLED 0.42 นิ้ว  ความละเอียดจริง 72 x 40 พิกเซล  "
            "ภาพในแผ่นนี้ขยาย 4 เท่า  (firmware/Station-C3-OLED v1.0.0-C3)")

GROUPS = [
    ("สี่หน้าที่พยาบาลใช้ประจำ  กดปุ่มสั้นเพื่อเปลี่ยนหน้า", [
        ("02_rate_normal",
         "หน้า ๑ · อัตราการไหล",
         "๑๐๐ mL/h ตัวใหญ่เต็มจอ  >๑๐๐ คือค่าที่แพทย์สั่ง\nขวาคือกระเปาะหยด  แถบล่างคือให้ไปแล้ว ๕๒% ของถุง"),
        ("06_gtt",
         "หน้า ๒ · หยดต่อนาที",
         "๓๓ gtt/min เทียบกับเป้าหมาย ๓๓.๓\nเป็นหน่วยที่พยาบาลนับด้วยตาจริงข้างเตียง"),
        ("07_volume",
         "หน้า ๓ · ยอดสะสม",
         "ให้ไปแล้ว ๕๒๑ จาก ๑๐๐๐ mL\nนับได้ ๑๐,๔๑๒ หยด และสถานะเซนเซอร์"),
        ("08_status",
         "หน้า ๔ · สถานะเครื่อง",
         "เลขเตียง ช่องสัญญาณ การเชื่อมต่อ\nเซนเซอร์ และแรงดันแบตเตอรี่"),
    ]),
    ("สถานะที่ต้องสังเกต", [
        ("01_rate_learning",
         "เซนเซอร์ยังเรียนรู้",
         "ขึ้น LEARN แทนค่าเป้าหมาย\nเพราะระหว่างนี้ตัวเลขบนจอยังเชื่อไม่ได้"),
        ("09_no_link",
         "ขาดการเชื่อมต่อ",
         "หัวจอขึ้น NO LINK กะพริบแทนแท่งสัญญาณ\nเครื่องยังนับหยดต่อไปตามปกติ"),
        ("10_paused",
         "หยุดชั่วคราว",
         "กดปุ่มสั้นสองครั้งเพื่อเริ่มใหม่\nหัวจอขึ้น PAUSE กะพริบด้วย"),
        ("05_rate_low_value",
         "อัตราต่ำกว่า ๑๐",
         "เปลี่ยนเป็นทศนิยมหนึ่งตำแหน่งเอง\nจะได้ไม่เสียความละเอียดที่อัตราต่ำ"),
    ]),
    ("หน้าเตือน  กินทั้งจอและกะพริบ เพราะการเตือนเล็ก ๆ บนจอขนาดนี้ไม่มีใครเห็น", [
        ("11_alert_no_flow",
         "ไม่ไหล / สายพับ",
         "ให้ไปดูสายว่าพับหรือไม่\nเครื่องตรวจเองได้แม้ Host เงียบ"),
        ("12_alert_too_fast",
         "ไหลเร็วเกินแผน",
         "หรี่โรลเลอร์ลง"),
        ("13_alert_too_slow",
         "ไหลช้ากว่าแผน",
         "เปิดโรลเลอร์เพิ่ม"),
        ("14_alert_near_end",
         "ใกล้หมด เตรียมถุงใหม่",
         "กดปุ่มหนึ่งครั้งเพื่อรับทราบที่เตียง\nสถานะรับทราบถูกส่งกลับไปที่ Host ด้วย"),
        ("15_alert_finished",
         "ให้ครบตามแผนแล้ว",
         "ปริมาตรถึงค่าที่วางแผนไว้"),
        ("16_alert_blink_invert",
         "อีกครึ่งรอบของการกะพริบ",
         "สลับกับภาพพื้นดำทุก ๐.๖ วินาที\nมองเห็นได้จากปลายเตียงแม้ไม่ได้ตั้งใจมอง"),
    ]),
    ("ตัวช่วยคาลิเบรตสี่ขั้นตอน  กดสั้นสามครั้งเพื่อเข้า  กดค้างเพื่อออก", [
        ("19_cal1_rawhigh",
         "ขั้น ๑ · ยังไม่เข้าเกณฑ์",
         "RAW HI = ค่าดิบชนเพดาน\nเซนเซอร์หลุดหรือโดนแสงแรงเกิน"),
        ("20_cal1_countdown",
         "ขั้น ๑ · นับถอยหลัง",
         "เข้าเกณฑ์แล้ว มีขีดใต้ชื่อขั้น\nต้องนิ่งค้างครบ ๓ วินาทีจึงผ่าน"),
        ("21_cal2_searching",
         "ขั้น ๒ · รอหยดแรก",
         "เปิดแคลมป์ให้หยด\nจอนับหยดที่เห็น จะได้รู้ว่าไม่ค้าง"),
        ("22_cal2_direction",
         "ขั้น ๒ · รู้ทิศแล้ว",
         "DROP LO = หยดทำให้ค่าลดลง\nวัดค่าดิบเอง รู้ผลในสามหยด"),
        ("23_cal3_learn",
         "ขั้น ๓ · เรียนรู้รูปคลื่น",
         "เก็บความสูงและความกว้าง ๑๐ หยด\nต้องได้ SNR ไม่ต่ำกว่า ๖ เท่า"),
        ("24_cal4_verify",
         "ขั้น ๔ · ยืนยัน",
         "นับอีก ๑๐ หยดดูความสม่ำเสมอ\nกันผลฟลุกจากสิบหยดแรก"),
        ("25_cal_done",
         "ผ่านครบสี่ขั้น",
         "กดหนึ่งครั้งเพื่อบันทึกถาวร\nเปิดเครื่องหน้าพร้อมนับทันที"),
        ("26_cal_failed",
         "ขั้นที่ไม่ผ่าน",
         "กลับสีทั้งแถบ บอกสาเหตุที่แก้ได้\nแก้แล้วกดหนึ่งครั้งเพื่อลองใหม่"),
    ]),
    ("ตั้งค่าและหน้าอื่น", [
        ("17_set_bed_id",
         "ตั้งเลขเตียงที่หน้าเครื่อง",
         "กดค้าง ๔ วินาทีเพื่อเข้า  กดสั้นเพิ่มเลข  กดค้างบันทึก\nของเดิมต้องแก้โค้ดแล้วคอมไพล์ใหม่ทีละเตียง"),
        ("00_splash",
         "หน้าต้อนรับตอนเปิดเครื่อง",
         "บอกเวอร์ชันและเลขเตียงที่จำไว้"),
        ("18_screensaver",
         "หน้าพักจอ",
         "หลังไม่แตะปุ่ม ๓ นาที  ลดการเบิร์นของจอ OLED\nยังบอกเวลาและอัตราการไหลไว้"),
    ]),
]


def font(path, size):
    return ImageFont.truetype(path, size, layout_engine=LAYOUT)


def th_pieces(text):
    """หั่นข้อความเป็นชิ้นที่ตัดบรรทัดได้โดยไม่ทำให้คำไทยขาดกลางคำ"""
    if _th_tokenize is None:
        return list(text)
    out = []
    for tok in _th_tokenize(text, engine="newmm", keep_whitespace=True):
        out.append(tok)
    return out


def wrap(draw, text, f, maxw):
    """ตัดบรรทัดตามความกว้างจริงของฟอนต์ โดยตัดที่ขอบคำภาษาไทย"""
    out = []
    for para in text.split("\n"):
        line = ""
        for piece in th_pieces(para):
            cand = line + piece
            if draw.textlength(cand.strip(), font=f) <= maxw or not line.strip():
                line = cand
            else:
                out.append(line.rstrip())
                line = piece.lstrip()
        # ชิ้นเดี่ยวที่ยาวเกินคอลัมน์จริง ๆ ต้องหั่นทีละตัวอักษร (เกิดยากมาก)
        while draw.textlength(line.strip(), font=f) > maxw and len(line) > 1:
            cut = len(line)
            while cut > 1 and draw.textlength(line[:cut], font=f) > maxw:
                cut -= 1
            out.append(line[:cut])
            line = line[cut:]
        if line.strip():
            out.append(line.rstrip())
    return out


def build_sheet():
    f_title = font(TH_B, 46)
    f_sub   = font(TH_R, 26)
    f_grp   = font(TH_B, 32)
    f_cap   = font(TH_B, 26)
    f_txt   = font(TH_R, 24)

    width = MARGIN * 2 + COLS * SW + (COLS - 1) * GAP
    probe = ImageDraw.Draw(Image.new("RGB", (8, 8)))

    # ---- รอบแรก: คำนวณความสูงของทุกแถวจากจำนวนบรรทัดที่ตัดได้จริง ----
    plan = []          # [(ชื่อกลุ่ม, [ (แถว: [(base, cap, [บรรทัด]) ]) ], ...)]
    for gname, items in GROUPS:
        rows = []
        for i in range(0, len(items), COLS):
            row = []
            for base, cap, txt in items[i:i + COLS]:
                row.append((base, cap, wrap(probe, txt, f_txt, SW)))
            rows.append(row)
        plan.append((gname, rows))

    sub_lines = wrap(probe, SUBTITLE, f_sub, width - MARGIN * 2)

    y = MARGIN + 54 + len(sub_lines) * 30 + 26
    for gname, rows in plan:
        y += len(wrap(probe, gname, f_grp, width - MARGIN * 2)) * 38 + 16
        for row in rows:
            nlines = max(len(lines) for _, _, lines in row)
            y += SH + CAP_GAP + 30 + nlines * 26 + GAP
        y += 18
    height = y + MARGIN - GAP

    img = Image.new("RGB", (width, height), BG)
    d = ImageDraw.Draw(img)

    d.text((MARGIN, MARGIN), TITLE, font=f_title, fill=FG)
    y = MARGIN + 54
    for line in sub_lines:
        d.text((MARGIN, y), line, font=f_sub, fill=DIM)
        y += 30
    y += 26

    for gname, rows in plan:
        d.line([(MARGIN, y - 14), (width - MARGIN, y - 14)], fill=LINE, width=2)
        for line in wrap(probe, gname, f_grp, width - MARGIN * 2):
            d.text((MARGIN, y), line, font=f_grp, fill=ACCENT)
            y += 38
        y += 16
        for row in rows:
            nlines = max(len(lines) for _, _, lines in row)
            for col, (base, cap, lines) in enumerate(row):
                path = os.path.join(OUT, base + ".png")
                if not os.path.exists(path):
                    continue
                x = MARGIN + col * (SW + GAP)
                im = Image.open(path).convert("RGB").resize((SW, SH), Image.NEAREST)
                img.paste(im, (x, y))
                d.rectangle([x, y, x + SW - 1, y + SH - 1], outline=LINE)
                d.text((x, y + SH + CAP_GAP), cap, font=f_cap, fill=FG)
                ty = y + SH + CAP_GAP + 30
                for line in lines:
                    d.text((x, ty), line, font=f_txt, fill=DIM)
                    ty += 26
            y += SH + CAP_GAP + 30 + nlines * 26 + GAP
        y += 18

    out = os.path.join(OUT, "overview.png")
    img.save(out)
    print("เขียนแล้ว:", out, img.size)


def build_gifs():
    frames = [Image.open(f).convert("P")
              for f in sorted(glob.glob(os.path.join(OUT, "anim_drip_*.png")))]
    if frames:
        p = os.path.join(OUT, "drip_animation.gif")
        frames[0].save(p, save_all=True, append_images=frames[1:], duration=150, loop=0)
        print("เขียนแล้ว:", p, len(frames), "เฟรม")

    a = os.path.join(OUT, "11_alert_no_flow.png")
    b = os.path.join(OUT, "16_alert_blink_invert.png")
    if os.path.exists(a) and os.path.exists(b):
        p = os.path.join(OUT, "alert_blink.gif")
        Image.open(a).convert("P").save(p, save_all=True,
                                        append_images=[Image.open(b).convert("P")],
                                        duration=600, loop=0)
        print("เขียนแล้ว:", p, "2 เฟรม")


if __name__ == "__main__":
    build_sheet()
    build_gifs()
