#!/usr/bin/env bash
# ติดตั้งฟอนต์ TH Sarabun (ชุดฟอนต์แห่งชาติ) สำหรับแปลงคู่มือเป็น PDF
#
# เอกสารราชการไทยกำหนดให้ใช้ฟอนต์ TH SarabunPSK ซึ่งไม่มีในคลังแพ็กเกจของระบบ
# สคริปต์นี้ดึงไฟล์ TH Sarabun New (ฟอนต์เดียวกันรุ่นปรับปรุง โดยผู้ออกแบบคนเดียวกัน)
# จาก npm แล้วตั้ง fontconfig ให้ชื่อ "TH SarabunPSK" ในเอกสารชี้มาที่ไฟล์นี้
#
# ถ้าไม่ติดตั้ง LibreOffice จะแทนด้วยฟอนต์อื่น (เช่น Loma) ซึ่งผิดรูปแบบราชการ
set -euo pipefail

if fc-list :lang=th family | grep -q 'TH Sarabun'; then
  echo "มีฟอนต์ TH Sarabun อยู่แล้ว ข้ามการติดตั้ง"
  exit 0
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
( cd "$TMP" && npm pack font-th-sarabun-new >/dev/null 2>&1 && tar xzf ./*.tgz )

DEST=/usr/local/share/fonts/thsarabun
mkdir -p "$DEST"
cp "$TMP"/package/fonts/*.ttf "$DEST/"

cat > /etc/fonts/conf.d/99-th-sarabun-psk.conf <<'CONF'
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<!-- เอกสารระบุชื่อฟอนต์ว่า "TH SarabunPSK" ตามมาตรฐานเอกสารราชการ
     ไฟล์ที่ติดตั้งจริงชื่อ "TH Sarabun New" ซึ่งเป็นฟอนต์เดียวกันรุ่นปรับปรุง
     จึงตั้ง alias ให้ชื่อในเอกสารชี้มาที่ไฟล์นี้ -->
<fontconfig>
  <alias binding="strong">
    <family>TH SarabunPSK</family>
    <accept><family>TH Sarabun New</family></accept>
  </alias>
  <alias binding="strong">
    <family>TH Sarabun PSK</family>
    <accept><family>TH Sarabun New</family></accept>
  </alias>
</fontconfig>
CONF

fc-cache -f >/dev/null
echo "ติดตั้งแล้ว: $(fc-match 'TH SarabunPSK')"
