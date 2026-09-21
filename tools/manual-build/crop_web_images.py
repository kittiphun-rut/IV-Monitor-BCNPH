# -*- coding: utf-8 -*-
"""ตัดภาพหน้าจอเว็บเป็นส่วน ๆ สำหรับใช้เป็นภาพประกอบในคู่มือ

ภาพ Live Monitor เต็มหน้าสูง 2400 px ถ้าย่อลงใส่หน้ากระดาษเดียวจะเล็กจนอ่านไม่ออก
จึงตัดเป็นส่วนย่อยตามหัวข้อที่คู่มืออธิบาย
"""
import shutil
from pathlib import Path
from PIL import Image

SRC = Path(__file__).parent / '../../docs/screens-dashboard'
DST = Path(__file__).parent / 'manual-img'

for name in ('live_monitor', 'visual_graphs', 'log_report', 'about'):
    shutil.copy(SRC / f'{name}.png', DST / f'web_{name}.png')

src = Image.open(SRC / 'live_monitor.png')
W, H = src.size
CROPS = {
    'web_header':   (0,  0,    W,    140),
    'web_banners':  (20, 150,  W-20, 510),
    'web_cards':    (20, 480,  W-20, 1100),
    'web_card_one': (30, 585,  375,  1035),
    'web_settings': (20, 1480, W-20, 2320),
}
for name, box in CROPS.items():
    src.crop(box).save(DST / f'{name}.png')
    print('ตัดแล้ว:', name)
