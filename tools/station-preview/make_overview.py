#!/usr/bin/env python3
"""รวมภาพหน้าจอ Station v7.6.x เป็นแผ่นสรุป 3 แผ่น สำหรับใส่ใน README

ใช้: python3 make_overview.py [docs_dir]
(ฟอนต์มาตรฐานของ PIL ไม่มีตัวอักษรไทย ป้ายกำกับจึงเป็นอังกฤษ)
"""
import sys
from PIL import Image, ImageDraw

DOCS = sys.argv[1] if len(sys.argv) > 1 else "../../docs/screens-station"

SHEETS = {
    "overview_pages": [
        ("p1_bag.png",   "1 IV BAG"),
        ("p2_plan.png",  "2 PLAN"),
        ("p3_link.png",  "3 LINK - sensor + host"),
        ("p4_trend.png", "4 TREND - 30 min"),
    ],
    "overview_alerts": [
        ("p1_noflow.png",  "IV BAG - no flow"),
        ("emergency.png",  "EMERGENCY screen"),
        ("p1_nearend.png", "IV BAG - near end"),
        ("near_end.png",   "NEXT BAG notice"),
    ],
    "overview_other": [
        ("sensor_test.png", "SENSOR TEST (hold 3s)"),
        ("config_id.png",   "SET BED ID"),
        ("screensaver.png", "SCREENSAVER"),
        ("credit.png",      "CREDITS (hold 5s)"),
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
