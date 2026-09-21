#!/usr/bin/env bash
# สร้างภาพหน้าจอของหน้าเว็บ Dashboard ครบทุกหน้า พร้อมข้อมูลจำลอง
# ใช้: run.sh [โฟลเดอร์เฟิร์มแวร์] [โฟลเดอร์ผลลัพธ์]
set -euo pipefail
cd "$(dirname "$0")"
SRC="${1:-../../firmware/ESP32-S3-Host-OLED-V_4_7_5}"
OUT="${2:-../../docs/screens-dashboard}"
mkdir -p "$OUT"
CHROME=/opt/pw-browsers/chromium-1194/chrome-linux/chrome
[ -x "$CHROME" ] || CHROME=/opt/pw-browsers/chromium/chrome-linux/chrome
[ -x "$CHROME" ] || CHROME="$(command -v chromium || command -v chromium-browser)"

shot() {   # shot <ชื่อหน้า> <ไฟล์ออก> <สูงพิกเซล>
  python3 make_preview.py "$SRC" preview.html "$1"
  "$CHROME" --headless --disable-gpu --no-sandbox --hide-scrollbars \
    --virtual-time-budget=6000 --window-size=1400,"$3" \
    --screenshot="$OUT/$2" "file://$PWD/preview.html" >/dev/null 2>&1
  rm -f preview.html
  echo "  $OUT/$2"
}

shot live   live_monitor.png 2400
shot graphs visual_graphs.png 1500
shot logs   log_report.png   1300
shot about  about.png        1200
echo "เสร็จแล้ว"
