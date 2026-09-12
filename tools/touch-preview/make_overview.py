#!/usr/bin/env python3
"""รวมภาพหน้าจอ Host v4.9.x-TOUCH เป็นแผ่นสรุป 3 แผ่น สำหรับใส่ใน README

ใช้: python3 make_overview.py [docs_dir]
(ฟอนต์มาตรฐานของ PIL ไม่มีตัวอักษรไทย ป้ายกำกับจึงเป็นอังกฤษ)
"""
import sys
from PIL import Image, ImageDraw

DOCS = sys.argv[1] if len(sys.argv) > 1 else "../../docs/screens-touch"

SHEETS = {
    "overview_home": [
        ("home_normal.png",  "HOME - all normal"),
        ("home_nearend.png", "HOME - bag near end"),
        ("home_alert.png",   "HOME - critical alert"),
        ("home_8beds.png",   "HOME - 8 beds"),
    ],
    "overview_settings": [
        ("bed_detail.png",   "BED detail + actions"),
        ("bed_settings.png", "BED settings (-/+)"),
        ("numpad.png",       "NUMPAD - type a value"),
        ("system.png",       "SYSTEM - wifi / clock / SD"),
    ],
    "overview_other": [
        ("splash.png",       "BOOT splash"),
        ("alarm.png",        "FULL SCREEN alarm"),
        ("calibration.png",  "TOUCH calibration"),
        ("poweroff.png",     "POWER off"),
    ],
}

GAP, LABEL_H, BG, FG = 18, 26, (14, 14, 18), (200, 205, 215)

for name, items in SHEETS.items():
    tiles = [(Image.open(f"{DOCS}/{f}"), text) for f, text in items]
    w, h = tiles[0][0].size
    sheet = Image.new("RGB", (GAP + len(tiles) * (w + GAP), LABEL_H + h + GAP * 2), BG)
    draw = ImageDraw.Draw(sheet)
    for i, (img, text) in enumerate(tiles):
        x = GAP + i * (w + GAP)
        draw.text((x, 8), text, fill=FG)
        sheet.paste(img, (x, LABEL_H + GAP // 2))
    out = f"{DOCS}/{name}.png"
    sheet.save(out)
    print(out, sheet.size)
