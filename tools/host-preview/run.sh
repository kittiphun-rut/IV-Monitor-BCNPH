#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอ TFT 2.8" ของเฟิร์มแวร์ Host v4.8.0-TFT ทุกหน้า (จำลอง Arduino API บน PC)
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-host}"
SKETCH_DIR="../../firmware/ESP32-S3-Host-TFT-V_4_8_0"

cp "$SKETCH_DIR/ESP32-S3-Host-TFT-V_4_8_0.ino" host.cpp
cp "$SKETCH_DIR/web_dashboard.h" web_dashboard.h
g++ -std=gnu++17 -I../screen-preview -Wall -Wno-unused-variable -Wno-format-truncation -o hostpreview driver.cpp
./hostpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT" 320 240 3
rm -f host.cpp web_dashboard.h hostpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
