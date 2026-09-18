#!/usr/bin/env python3
"""เรนเดอร์คำสั่งวาดของ U8g2 เป็นภาพจอ OLED ขาวดำ 128x64

ใช้: render_oled.py ops.txt outdir [scale]

ความแม่นยำ
  - กรอบ/กล่อง/เส้น เรนเดอร์ตรงพิกัดจริงทุกพิกเซล
  - ฟอนต์ความกว้างคงที่ (4x6, 5x8, 6x10, 7x13, 7x14B) ใช้ความกว้างและเส้นฐานตรงตาม U8g2
    จึงตรวจ "ข้อความล้นจอ 128 px" ได้ตรงกับของจริง
  - ฟอนต์ความกว้างไม่คงที่ (helvB10/12, logisoso16/32) ใช้ความกว้างโดยประมาณ
    รูปร่างตัวอักษรจึงไม่เหมือนของจริงเป๊ะ แต่ตำแหน่งและขนาดใกล้เคียงพอสำหรับตรวจผัง
"""
import os
import sys
from PIL import Image, ImageDraw, ImageFont

W, H = 128, 64
MONO = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
MONO_B = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"
SANS_B = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"

# ชื่อฟอนต์ -> (ความกว้างต่อตัว, ระยะจากเส้นฐานขึ้นไปถึงยอดตัวอักษร, ขนาด px, ไฟล์ฟอนต์, คงที่?)
FONTS = {
    "4x6":        (4,  5,  6,  MONO,   True),
    "5x8":        (5,  7,  8,  MONO,   True),
    "6x10":       (6,  7,  9,  MONO,   True),
    "7x13":       (7, 10, 12,  MONO,   True),
    "7x14B":      (7, 11, 13,  MONO_B, True),
    "helvB10":    (7, 10, 12,  SANS_B, False),
    "helvB12":    (8, 12, 14,  SANS_B, False),
    "logisoso16": (11, 16, 19, SANS_B, False),
    "logisoso32": (21, 32, 38, SANS_B, False),
}
_cache = {}


def pil_font(path, size):
    key = (path, size)
    if key not in _cache:
        _cache[key] = ImageFont.truetype(path, size)
    return _cache[key]


def draw_text(img, x, y, name, color, text, scale):
    adv, ascent, px, path, fixed = FONTS.get(name, FONTS["5x8"])
    f = pil_font(path, px * scale)
    d = ImageDraw.Draw(img)
    fill = 255 if color else 0
    top = (y - ascent) * scale            # y ของ U8g2 คือเส้นฐาน
    if fixed:
        for i, ch in enumerate(text):     # วางทีละตัวในช่องกว้างคงที่ = ตรงกับของจริง
            d.text(((x + i * adv) * scale, top), ch, font=f, fill=fill)
    else:
        d.text((x * scale, top), text, font=f, fill=fill)


def text_width(name, text):
    adv, *_ = FONTS.get(name, FONTS["5x8"])
    return adv * len(text)


def main(ops_path, outdir, scale=6):
    os.makedirs(outdir, exist_ok=True)
    img = Image.new("L", (W * scale, H * scale), 0)
    warnings = []
    saved = []

    for line in open(ops_path, encoding="utf-8"):
        p = line.rstrip("\n").split(" ")
        op = p[0]
        if op == "CLEAR":
            img = Image.new("L", (W * scale, H * scale), 0)
        elif op == "MARK":
            name = p[1]
            out = os.path.join(outdir, name + ".png")
            img.convert("RGB").save(out)
            saved.append(out)
        elif op == "TEXT":
            x, y, font, color = int(p[1]), int(p[2]), p[3], int(p[4])
            text = " ".join(p[5:])
            draw_text(img, x, y, font, color, text, scale)
            end = x + text_width(font, text)
            if end > W:
                warnings.append(f"ข้อความล้นขอบขวา ({end} > {W} px) ที่ ({x},{y}) ฟอนต์ {font}: {text!r}")
        else:
            x, y, w, h, r, color = (int(v) for v in p[1:7])
            d = ImageDraw.Draw(img)
            fill = 255 if color else 0
            box = [x * scale, y * scale, (x + w) * scale - 1, (y + h) * scale - 1]
            if op == "BOX" or op == "HLINE":
                d.rectangle(box, fill=fill)
            elif op == "FRAME":
                d.rectangle(box, outline=fill, width=scale)
            elif op == "RBOX":
                d.rounded_rectangle(box, radius=r * scale, fill=fill)
            elif op == "RFRAME":
                d.rounded_rectangle(box, radius=r * scale, outline=fill, width=scale)

    for s in saved:
        print(s)
    if warnings:
        print("\nคำเตือน:")
        for w in sorted(set(warnings)):
            print("  " + w)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 6))
