# -*- coding: utf-8 -*-
"""ตรวจว่าหัวไฟล์และป้ายเวอร์ชันเป็นไปตาม docs/CODE-STANDARD.md

ยกมาจากโครงการ MCUPR-CANTEEN แล้วปรับสองข้อให้เข้ากับรีโพนี้
  ก) เลขเวอร์ชันของรีโพนี้มีส่วนต่อท้าย เช่น 4.7.7-OLED, 1.0.0-C3, S1.0.0
     จึงต้องตัดส่วนต่อท้ายออกก่อนเทียบว่าใหม่กว่ากันไหม
  ข) drop_detector.h ใช้ร่วมกันสองสเก็ตช์ที่เลขเวอร์ชันคนละสาย (7.8.0 กับ 1.0.0-C3)
     และต้องเหมือนกันทุกไบต์ จึงเทียบกับ APP_VERSION ไม่ได้ ใช้เลขของอัลกอริทึมเอง

ตรวจห้าข้อ
  1. ทุกไฟล์ .ino และ .h มีบล็อกหัวไฟล์ พร้อมแท็กบังคับครบ
  2. @file ตรงกับชื่อไฟล์จริง
  3. @version ของไฟล์ .ino ตรงกับ APP_VERSION เป๊ะ
     ส่วนไฟล์ .h คือ "เวอร์ชันที่ไฟล์นี้ถูกแก้ครั้งล่าสุด" ต้องเป็นแถวบนสุดของตาราง
     และต้องไม่ใหม่กว่า APP_VERSION (ไฟล์ที่ไม่ได้แก้ในรุ่นล่าสุดจึงค้างเลขเดิมไว้ได้)
  4. @date เป็นรูปแบบ YYYY-MM-DD และตาราง Revision History มีแถวของเวอร์ชันปัจจุบัน
  5. ป้าย // [x.y.z] ในเนื้อโค้ด อ้างเลขที่มีอยู่จริงในตารางของไฟล์นั้น

ข้อที่ห้ามีไว้กันไม่ให้ปักป้ายด้วยเลขที่ไม่มีใครตามได้
ใช้:  bash tools/header-check/run.sh
"""
import io, os, re, sys, glob

REQUIRED = ['@file', '@brief', '@version', '@date', '@author']

# ไฟล์ที่ใช้ร่วมกันหลายสเก็ตช์ เลขเวอร์ชันเป็นของอัลกอริทึมเอง ไม่ผูกกับ APP_VERSION
SHARED = {'drop_detector.h'}

VER = r'[A-Za-z]*\d+\.\d+\.\d+(?:-[A-Za-z0-9]+)?' 
MARK = re.compile(r'^\s*//\s*\[(%s)\]\s*(\S+):' % VER, re.M)
VERBS = {'เพิ่ม', 'แก้', 'ย้าย'}

def vtuple(v):
    """แปลง 4.7.7-OLED เป็น (4, 7, 7) เพื่อเทียบว่าใหม่กว่ากันไหม
    ตัดตัวอักษรนำหน้าและส่วนต่อท้ายออก เพราะเป็นชื่อรุ่นไม่ใช่ลำดับ"""
    core = re.search(r'(\d+)\.(\d+)\.(\d+)', v)
    return tuple(int(x) for x in core.groups()) if core else (0, 0, 0)

def sketch_version(path):
    """อ่าน APP_VERSION จากไฟล์ .ino ของโฟลเดอร์เดียวกัน"""
    folder = os.path.dirname(path)
    for ino in glob.glob(os.path.join(folder, '*.ino')):
        m = re.search(r'#define\s+APP_VERSION\s+"([^"]+)"',
                      io.open(ino, encoding='utf-8').read())
        if m:
            return m.group(1)
    return None

def check(path):
    problems = []
    text = io.open(path, encoding='utf-8').read()
    if not text.startswith('/**'):
        return ['ไม่มีบล็อกหัวไฟล์ ต้องขึ้นต้นด้วย /**']
    head = text[:text.index(' */\n') + 4]

    tags = {}
    for tag in REQUIRED:
        m = re.search(r'^\s*\*\s*%s\s+(.+?)\s*$' % re.escape(tag), head, re.M)
        if not m:
            problems.append('ไม่มีแท็ก %s' % tag)
        else:
            tags[tag] = m.group(1)

    if '@file' in tags and tags['@file'] != os.path.basename(path):
        problems.append('@file เขียนว่า %s แต่ชื่อไฟล์จริงคือ %s'
                        % (tags['@file'], os.path.basename(path)))

    want = None if os.path.basename(path) in SHARED else sketch_version(path)
    if '@version' in tags and want:
        if path.endswith('.ino'):
            if tags['@version'] != want:
                problems.append('@version คือ %s แต่ APP_VERSION คือ %s'
                                % (tags['@version'], want))
        elif vtuple(tags['@version']) > vtuple(want):
            problems.append('@version คือ %s ซึ่งใหม่กว่า APP_VERSION ของสเก็ตช์ (%s)'
                            % (tags['@version'], want))

    if '@date' in tags and not re.fullmatch(r'\d{4}-\d{2}-\d{2}', tags['@date']):
        problems.append('@date ต้องเป็นรูปแบบ YYYY-MM-DD ไม่ใช่ %s' % tags['@date'])

    if 'Revision History' not in head:
        problems.append('ไม่มีตาราง @par Revision History')
        return problems

    listed = re.findall(r'^\s*\*\s*\|\s*(%s)\s*\|' % VER, head, re.M)
    if not listed:
        problems.append('ตาราง Revision History ไม่มีแถวไหนเลย')
    elif '@version' in tags and tags['@version'] != listed[0]:
        problems.append('@version คือ %s แต่แถวบนสุดของตารางคือ %s '
                        'ตารางต้องเรียงจากใหม่ไปเก่า และแถวบนสุดต้องเป็นเวอร์ชันของไฟล์'
                        % (tags['@version'], listed[0]))

    body = text[len(head):]
    for ver, verb in MARK.findall(body):
        if ver not in listed:
            problems.append('ป้าย [%s] อ้างเวอร์ชันที่ไม่มีในตาราง Revision History' % ver)
        if verb not in VERBS:
            problems.append('ป้าย [%s] ใช้คำว่า "%s" ต้องเป็น เพิ่ม/แก้/ย้าย เท่านั้น'
                            % (ver, verb))
    return problems

targets = sys.argv[1:] or sorted(glob.glob('firmware/*/*.ino') + glob.glob('firmware/*/*.h'))
bad = 0
for f in targets:
    p = check(f)
    marks = len(MARK.findall(io.open(f, encoding='utf-8').read()))
    print('%-48s %-28s %s' % (f, 'ป้ายเวอร์ชัน %d จุด' % marks,
                              'OK' if not p else '*** พบ %d ปัญหา ***' % len(p)))
    for line in p:
        print('      ' + line)
    bad += len(p)
sys.exit(1 if bad else 0)
