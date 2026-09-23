#!/usr/bin/env bash
# ตรวจผังการเชื่อมต่อด้วย KiCad จริง แล้วเทียบทุกเน็ตกับ #define ในเฟิร์มแวร์
# ใช้: bash hardware/verify.sh
set -euo pipefail
cd "$(dirname "$0")"
command -v kicad-cli >/dev/null || { echo "ต้องติดตั้ง kicad ก่อน: apt-get install -y kicad"; exit 1; }

for f in Station-BedNode Host-OLED; do
  echo "== $f =="
  kicad-cli sch export pdf     --output "$f.pdf" "$f.kicad_sch" >/dev/null
  kicad-cli sch export netlist --output "/tmp/$f.net" "$f.kicad_sch" >/dev/null
  echo "  เปิดไฟล์และสร้าง PDF ได้"
done

python3 - <<'PY'
import re, sys

CASES = [
    ('/tmp/Station-BedNode.net',
     '../firmware/Station/Station.ino',
     {'TFT_SCLK': 'TFT_SCLK', 'TFT_MOSI': 'TFT_MOSI', 'TFT_RST': 'TFT_RST',
      'TFT_DC': 'TFT_DC', 'TFT_CS': 'TFT_CS', 'TFT_BLK': 'TFT_BLK',
      'DROP_AO': 'SENSOR_AO_PIN', 'BUZZER': 'BUZZER_PIN', 'BTN': 'BTN_PIN',
      'RGB_LED_ONBOARD': 'RGB_LED_PIN'}),
    ('/tmp/Host-OLED.net',
     '../firmware/Host-OLED/Host-OLED.ino',
     {'OLED_SDA': 'OLED_SDA_PIN', 'OLED_SCL': 'OLED_SCL_PIN',
      'BTN_POWER': 'POWER_BTN_PIN', 'BTN_PAGE': 'PAGE_BTN_PIN',
      'BUZZER': 'BUZZER_PIN', 'VBAT_SENSE': 'HOST_BAT_ADC_PIN'}),
]

bad = 0
for net_file, ino, mapping in CASES:
    src = open(ino, encoding='utf-8').read()
    nl = open(net_file, encoding='utf-8').read()
    print('== %s ==' % net_file.split('/')[-1])
    for net, define in mapping.items():
        want = int(re.search(r'^#define\s+%s\s+(-?\d+)' % define, src, re.M).group(1))
        m = re.search(r'\(net \(code "\d+"\) \(name "/%s"\)((?:\s*\(node [^\n]*)+)'
                      % re.escape(net), nl)
        if not m:
            print('  ไม่ผ่าน  ไม่พบเน็ต %s' % net); bad += 1; continue
        got = re.findall(r'\(ref "U1"\) \(pin "IO(\d+)"\)', m.group(1))
        if len(got) == 1 and int(got[0]) == want:
            print('  ผ่าน     %-18s GPIO %-3d = %s' % (net, want, define))
        else:
            print('  ไม่ผ่าน  %-18s ผังต่อ IO%s แต่เฟิร์มแวร์ระบุ GPIO %d'
                  % (net, got or '?', want)); bad += 1
print()
print('ไม่ผ่าน %d รายการ' % bad if bad else 'ผังตรงกับเฟิร์มแวร์ทุกเส้น')
sys.exit(1 if bad else 0)
PY
