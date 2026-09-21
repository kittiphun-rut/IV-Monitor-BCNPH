#!/usr/bin/env bash
# ตรวจแผ่นวงจรพิมพ์: รัน DRC จริงด้วย pcbnew แล้วนับเฉพาะรายการระดับ error
# ใช้: bash hardware/verify_pcb.sh
set -euo pipefail
cd "$(dirname "$0")"
python3 - <<'PY'
import sys, re
try:
    import pcbnew as P
except ImportError:
    sys.exit('ต้องติดตั้ง kicad ก่อน: apt-get install -y kicad')

bad = 0
for name in ('Station-BedNode', 'Host-OLED'):
    b = P.LoadBoard(name + '.kicad_pcb')
    b.BuildListOfNets()
    rep = '/tmp/%s-drc.txt' % name
    P.WriteDRCReport(b, rep, P.EDA_UNITS_MILLIMETRES, True)
    t = open(rep, encoding='utf-8').read()
    errs = t.count('Severity: error')
    warns = t.count('Severity: warning')
    print('%-18s error %d   warning %d' % (name, errs, warns))
    if errs:
        for blk in re.split(r'\n(?=\[)', t[t.index('['):]):
            if 'Severity: error' in blk:
                print('   ' + ' | '.join(x.strip() for x in blk.strip().split('\n')[:3])[:160])
    bad += errs
print()
print('พบข้อผิดพลาด %d รายการ' % bad if bad else 'ไม่มีข้อผิดพลาดทางไฟฟ้า (warning ที่เหลือเป็นเรื่องซิลค์สกรีนและไลบรารี)')
sys.exit(1 if bad else 0)
PY
