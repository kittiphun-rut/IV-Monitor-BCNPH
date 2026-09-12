#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอของเฟิร์มแวร์ Host v4.9.0-TOUCH (จอสัมผัส 240x320) ทุกหน้า
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-touch}"
SKETCH_DIR="../../firmware/ESP32-S3-Host-Touch-V_4_9_0"

# ตรวจลำดับการประกาศชนิดข้อมูล (Arduino แทรก prototype ไว้ก่อนฟังก์ชันแรกของไฟล์)
python3 ../check-ino-types.py "$SKETCH_DIR/ESP32-S3-Host-Touch-V_4_9_0.ino"

cp "$SKETCH_DIR/ESP32-S3-Host-Touch-V_4_9_0.ino" host.cpp
cp "$SKETCH_DIR/web_dashboard.h" web_dashboard.h
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable -Wno-format-truncation -o touchpreview driver.cpp
./touchpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT" 240 320 3
rm -f host.cpp web_dashboard.h touchpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
