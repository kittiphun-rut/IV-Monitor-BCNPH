#!/usr/bin/env bash
# สร้างคู่มือพยาบาล (.docx) ตามรูปแบบเอกสารราชการไทย
#
# สารบัญเป็น "ฟิลด์ TOC จริงของ Word" ที่มีผลลัพธ์คำนวณไว้ล่วงหน้าอยู่ข้างใน
#   - เปิดแล้วสั่งพิมพ์ได้ทันที เพราะมีรายการและเลขหน้าครบอยู่แล้ว
#   - กด Ctrl+A แล้ว F9 หรือคลิกขวาที่สารบัญ > Update Field เพื่อให้ Word สร้างใหม่
#   - ตั้ง updateFields ไว้ Word จึงอัปเดตให้เองตั้งแต่ตอนเปิดไฟล์
#
# ทำสองรอบเพื่อให้เลขหน้าที่ฝังไว้ตรงกับเล่มจริง
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/คู่มือพยาบาล_Smart_IV_Alert.docx}"
TXT="${OUT%.docx}.txt"

[ -d node_modules ] || npm install docx --no-audit --no-fund >/dev/null

# ต้องมีฟอนต์ TH Sarabun ก่อน มิฉะนั้น PDF จะถูกแทนด้วยฟอนต์อื่นที่ผิดรูปแบบราชการ
fc-list :lang=th family 2>/dev/null | grep -q 'TH Sarabun' || bash install_fonts.sh

python3 make_images.py >/dev/null

build() {                       # สร้าง .docx แล้วแปลงสัญลักษณ์เป็นฟิลด์ TOC จริง
  node build_manual.js "$OUT" pagemap.json
  python3 inject_toc_field.py "$OUT"
}

topdf() {                       # soffice อ่านชื่อไฟล์ภาษาไทยไม่ได้ จึงคัดลอกก่อน
  local tmp="$1"
  cp "$OUT" "$tmp/manual.docx"
  soffice --headless --norestore -env:UserInstallation="file://$tmp/profile" \
          --convert-to pdf --outdir "$tmp" "$tmp/manual.docx" >/dev/null 2>&1
}

build
if command -v soffice >/dev/null && command -v pdftotext >/dev/null; then
  TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
  topdf "$TMP"
  python3 resolve_pages.py "$TMP/manual.pdf" pagemap.json part_front.js
  build                                   # รอบที่สอง ใช้เลขหน้าจริง

  topdf "$TMP"                            # ตรวจว่าเลขหน้านิ่งแล้ว
  python3 resolve_pages.py "$TMP/manual.pdf" /tmp/pagemap_check.json part_front.js >/dev/null
  if python3 -c "
import json,sys
a=json.load(open('pagemap.json')); b=json.load(open('/tmp/pagemap_check.json'))
d=[k for k in a if a.get(k)!=b.get(k)]
print('เลขหน้าที่ยังไม่นิ่ง:', len(d))
sys.exit(1 if d else 0)"; then
    echo "เลขหน้าในสารบัญตรงกับเล่มแล้ว"
  else
    echo "คำเตือน: เลขหน้ายังไม่นิ่ง ให้รัน run.sh ซ้ำอีกครั้ง"
  fi
else
  echo "ข้าม: ไม่มี soffice หรือ pdftotext จึงใช้เลขหน้าเดิมใน pagemap.json"
fi
# ---- ฉบับข้อความล้วน สร้างจากไฟล์ .docx ที่เสร็จแล้ว จึงตรงกับเล่มจริงเสมอ ----
python3 make_plaintext.py "$OUT" "$TXT"

echo "เสร็จแล้ว: $OUT"
echo "          $TXT"
