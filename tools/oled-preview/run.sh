#!/usr/bin/env bash
# จำลองหน้าจอ OLED 128x64 ของเฟิร์มแวร์ Host ทุกหน้า โดยไม่ต้องมีบอร์ด
set -euo pipefail
cd "$(dirname "$0")"
SKETCH_DIR="${1:-../../firmware/ESP32-S3-Host-OLED-V_4_7_3}"
OUT="${2:-../../docs/screens-oled}"
NAME="$(basename "$SKETCH_DIR")"

cp "$SKETCH_DIR/$NAME.ino" host_oled.cpp
cp "$SKETCH_DIR/web_dashboard.h" web_dashboard.h
g++ -std=gnu++17 -I../screen-preview -I. -DESP_ARDUINO_VERSION_MAJOR=3 \
    -Wall -Wno-unused-variable -Wno-format-truncation -o oledpreview driver.cpp
./oledpreview ops.txt
python3 render_oled.py ops.txt "$OUT" 6
python3 make_overview.py "$OUT"
rm -f host_oled.cpp web_dashboard.h oledpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
