#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอของเฟิร์มแวร์ Host รุ่นจอสัมผัส (จอสัมผัส 240x320) ทุกหน้า
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-touch}"
SKETCH_DIR="${2:-../../firmware/Host-Touch24}"

# ตรวจลำดับการประกาศชนิดข้อมูล (Arduino แทรก prototype ไว้ก่อนฟังก์ชันแรกของไฟล์)
python3 ../check-ino-types.py "$SKETCH_DIR/$(basename "$SKETCH_DIR").ino"

cp "$SKETCH_DIR/$(basename "$SKETCH_DIR").ino" host.cpp
# คัดลอกไฟล์ .h ทุกตัวของสเก็ตช์ ไม่ใช่แค่ web_dashboard.h
# เพราะโค้ดวาดจอถูกแยกไปอยู่ HostScreen.h / StationScreen.h แล้ว
for h in "$SKETCH_DIR"/*.h; do [ -e "$h" ] && cp "$h" "$(basename "$h")"; done
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable -Wno-format-truncation -o touchpreview driver.cpp
./touchpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT" 240 320 3
rm -f host.cpp web_dashboard.h HostScreen.h touchpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
