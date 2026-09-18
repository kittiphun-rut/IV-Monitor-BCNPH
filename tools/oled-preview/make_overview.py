#!/usr/bin/env python3
"""รวมภาพจำลองหน้าจอ OLED ทั้งหมดเป็นแผ่นเดียวสำหรับใส่ใน README

ใช้: python3 make_overview.py [docs_dir]
(ฟอนต์มาตรฐานของ PIL ไม่มีตัวอักษรไทย ป้ายกำกับจึงเป็นอังกฤษ)
"""
import sys
from PIL import Image, ImageDraw

DOCS = sys.argv[1] if len(sys.argv) > 1 else "../../docs/screens-oled"

TILES = [
    ("00_splash.png",            "00  SPLASH / boot"),
    ("01_block_5beds.png",       "01  BLOCK MONITOR - 5 beds"),
    ("02_detail_bed1.png",       "02  BED DETAIL - bed 1"),
    ("03_detail_bed5_nosignal.png", "03  BED DETAIL - sensor sees no drop"),
    ("04_block_all_normal.png",  "04  BLOCK MONITOR - all normal"),
    ("05_block_near_end.png",    "05  BLOCK MONITOR - near end footer"),
    ("06_block_1bed.png",        "06  BLOCK MONITOR - 1 bed"),
    ("07_block_2beds.png",       "07  BLOCK MONITOR - 2 beds"),
    ("08_block_8beds.png",       "08  BLOCK MONITOR - 8 beds"),
    ("09_screensaver.png",       "09  SCREENSAVER"),
]

COLS, GAP, LABEL_H = 2, 24, 26
BG, FG = (14, 14, 18), (200, 205, 215)

imgs = [(Image.open(f"{DOCS}/{f}"), text) for f, text in TILES]
w, h = imgs[0][0].size
rows = (len(imgs) + COLS - 1) // COLS
sheet = Image.new("RGB", (GAP + COLS * (w + GAP), GAP + rows * (LABEL_H + h + GAP)), BG)
draw = ImageDraw.Draw(sheet)
for i, (img, text) in enumerate(imgs):
    x = GAP + (i % COLS) * (w + GAP)
    y = GAP + (i // COLS) * (LABEL_H + h + GAP)
    draw.text((x, y + 6), text, fill=FG)
    sheet.paste(img, (x, y + LABEL_H))
out = f"{DOCS}/overview.png"
sheet.save(out)
print(out, sheet.size)
