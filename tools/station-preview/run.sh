#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอของเฟิร์มแวร์ Station v7.5.0 ทุกหน้า (ใช้ Arduino API จำลองบน PC)
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-station}"
SKETCH="../../firmware/ESP32-S3-Station-V_7_5_0/ESP32-S3-Station-V_7_5_0.ino"

cp "$SKETCH" station.cpp
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable -o stpreview driver.cpp
./stpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT"
rm -f station.cpp stpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
