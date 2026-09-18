#!/usr/bin/env bash
# สร้างไฟล์ HTML ของหน้า Dashboard พร้อมข้อมูลจำลอง แล้วถ่ายภาพหน้าจอด้วย Chromium
set -euo pipefail
cd "$(dirname "$0")"
SRC="${1:-../../firmware/ESP32-S3-Host-OLED-V_4_7_2}"
OUT="${2:-../../docs/screens-dashboard}"
mkdir -p "$OUT"
python3 make_preview.py "$SRC" preview.html
CHROME=/opt/pw-browsers/chromium-1194/chrome-linux/chrome
"$CHROME" --headless --disable-gpu --no-sandbox --hide-scrollbars \
  --virtual-time-budget=4000 --window-size=1400,2200 \
  --screenshot="$OUT/live_monitor.png" "file://$PWD/preview.html" >/dev/null 2>&1
rm -f preview.html
echo "เสร็จแล้ว: $OUT/live_monitor.png"
