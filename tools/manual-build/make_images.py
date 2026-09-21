#!/usr/bin/env python3
"""เตรียมภาพประกอบของคู่มือจากภาพหน้าจอที่เครื่องมือ preview สร้างไว้

ใช้: python3 make_images.py            (รันจากโฟลเดอร์นี้)
ภาพผังองค์ประกอบระบบ (diagram_system.png) วาดด้วยสคริปต์นี้เช่นกัน
"""
import os
from PIL import Image
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
OUT = os.path.join(os.path.dirname(__file__), 'manual-img')
os.makedirs(OUT, exist_ok=True)

def save(src, dst, crop=None, w=None):
    im = Image.open(os.path.join(ROOT, src)).convert('RGB')
    if crop: im = im.crop(crop)
    if w: im = im.resize((w, round(im.height * w / im.width)), Image.LANCZOS)
    im.save(os.path.join(OUT, dst))
    print(dst, im.size)

save('docs/screens-station/p1_bag.png',         'st_p1.png',   w=260)
save('docs/screens-station/p3_link.png',        'st_p3.png',   w=260)
save('docs/screens-station/emergency.png',      'st_emg.png',  w=260)
save('docs/screens-station/near_end.png',       'st_near.png', w=260)
save('docs/screens-station/config_id.png',      'st_cfg.png',  w=260)
save('docs/screens-oled/01_block_5beds.png',    'ol_block.png',  w=520)
save('docs/screens-oled/02_detail_bed1.png',    'ol_detail.png', w=520)
save('docs/screens-oled/14_link_diag_dup_id.png','ol_diag.png',  w=520)
save('docs/screens-dashboard/live_monitor.png', 'web_top.png',  crop=(20, 0, 1380, 700), w=540)
save('docs/screens-dashboard/live_monitor.png', 'web_card.png', crop=(30, 300, 720, 1000), w=420)
print('หมายเหตุ: diagram_system.png ไม่ได้สร้างใหม่จากสคริปต์นี้ เก็บไว้ในที่เก็บโดยตรง')
