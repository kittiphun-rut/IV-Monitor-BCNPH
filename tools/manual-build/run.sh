#!/usr/bin/env bash
# สร้างคู่มือพยาบาล (.docx) ตามรูปแบบเอกสารราชการไทย
#
# ทำสองรอบเพื่อให้เลขหน้าในสารบัญตรงกับเล่มจริง
#   รอบที่ ๑ สร้างเล่มด้วยเลขหน้าเดิม แล้วแปลงเป็น PDF เพื่ออ่านเลขหน้าจริง
#   รอบที่ ๒ สร้างใหม่โดยเติมเลขหน้าที่อ่านได้ลงในสารบัญ
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/คู่มือพยาบาล_Smart_IV_Alert.docx}"

[ -d node_modules ] || npm install docx --no-audit --no-fund >/dev/null

python3 make_images.py >/dev/null
node build_manual.js "$OUT" pagemap.json

# ---- ปรับเลขหน้าในสารบัญให้ตรงกับเล่มจริง ----
if command -v soffice >/dev/null && command -v pdftotext >/dev/null; then
  TMP="$(mktemp -d)"
  cp "$OUT" "$TMP/manual.docx"          # soffice อ่านชื่อไฟล์ภาษาไทยไม่ได้ จึงคัดลอกก่อน
  soffice --headless --norestore -env:UserInstallation="file://$TMP/profile" \
          --convert-to pdf --outdir "$TMP" "$TMP/manual.docx" >/dev/null 2>&1
  python3 resolve_pages.py "$TMP/manual.pdf" pagemap.json part_front.js
  node build_manual.js "$OUT" pagemap.json
  rm -rf "$TMP"
else
  echo "ข้าม: ไม่มี soffice หรือ pdftotext จึงใช้เลขหน้าเดิมใน pagemap.json"
fi
echo "เสร็จแล้ว: $OUT"
