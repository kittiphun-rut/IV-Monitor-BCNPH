#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอของเฟิร์มแวร์ Station v7.6.0 ทุกหน้า (ใช้ Arduino API จำลองบน PC)
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-station}"
SKETCH="../../firmware/ESP32-S3-Station-V_7_6_0/ESP32-S3-Station-V_7_6_0.ino"

# ตรวจลำดับการประกาศชนิดข้อมูล (Arduino แทรก prototype ไว้ก่อนฟังก์ชันแรกของไฟล์)
python3 ../check-ino-types.py "$SKETCH"

cp "$SKETCH" station.cpp
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable -o stpreview driver.cpp
./stpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT"
rm -f station.cpp stpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"

python3 make_overview.py "$OUT"
