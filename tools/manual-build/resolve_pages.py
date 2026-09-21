#!/usr/bin/env python3
"""อ่าน PDF ที่แปลงจากคู่มือ แล้วหาเลขหน้าจริงของทุกบรรทัดในสารบัญ

เลขหน้าที่พิมพ์ในเล่มเริ่มนับ ๑ ใหม่ที่บทที่ ๑ จึงต้องหาค่าชดเชยจากหน้า PDF ก่อน
"""
import json, re, subprocess, sys

pdf = sys.argv[1]
out = sys.argv[2]
entries_js = sys.argv[3]

npages = int(re.search(r'Pages:\s+(\d+)',
             subprocess.run(['pdfinfo', pdf], capture_output=True, text=True).stdout).group(1))

pages = []
for i in range(1, npages + 1):
    t = subprocess.run(['pdftotext', '-f', str(i), '-l', str(i), pdf, '-'],
                       capture_output=True, text=True).stdout
    pages.append(re.sub(r'\s+', '', t))

# ดึงข้อความของทุกบรรทัดสารบัญออกจากไฟล์ต้นฉบับ
src = open(entries_js, encoding='utf-8').read()
keys = re.findall(r"toc\('([^']+)'", src)

# ส่วนนำจบที่หน้า "สารบัญภาพ" (หน้าสุดท้ายที่มีคำนี้) เนื้อหาเริ่มหน้าถัดไป
last_front = None
for i, t in enumerate(pages):
    if 'สารบัญภาพ' in t:
        last_front = i
if last_front is None:
    print('หาไม่พบหน้าสารบัญภาพ'); sys.exit(1)
offset = last_front + 1        # index ฐานศูนย์ของหน้าที่พิมพ์เลข ๑
if 'บทที่๑' not in pages[offset]:
    print('คำเตือน: หน้าแรกของเนื้อหาไม่ได้ขึ้นต้นด้วยบทที่ ๑')

def printed(idx):
    return idx - offset + 1

mapping, missing = {}, []
for k in keys:
    flat = re.sub(r'\s+', '', k)
    if flat in ('คำนำ', 'สารบัญ', 'สารบัญตาราง', 'สารบัญภาพ'):
        continue                                  # ส่วนนำใช้เลขพยัญชนะไทยที่กำหนดไว้แล้ว
    # หัวข้อย่อยค้นได้ตรงตัว ส่วนหัวบท/ภาคผนวกต้องตัดคำนำหน้าออก
    needle = flat
    m = re.match(r'^บทที่[๐-๙]+(.+)$', flat)
    if m: needle = m.group(1)
    m = re.match(r'^ภาคผนวก[ก-ฮ](.+)$', flat)
    if m: needle = m.group(1)
    found = None
    for i in range(offset, npages):
        if needle in pages[i]:
            found = printed(i); break
    if found is None:
        missing.append(k)
    else:
        mapping[flat] = found

json.dump(mapping, open(out, 'w', encoding='utf-8'), ensure_ascii=False, indent=0)
print('จำนวนหน้าใน PDF:', npages, ' หน้าแรกของเนื้อหาอยู่ที่ PDF หน้า', offset + 1)
print('เติมเลขหน้าได้', len(mapping), 'รายการ')
if missing:
    print('หาไม่พบ', len(missing), 'รายการ:')
    for m in missing: print('   ', m)
