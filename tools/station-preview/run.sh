#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอของเฟิร์มแวร์ Station ทุกหน้า (ใช้ Arduino API จำลองบน PC)
# ใช้: run.sh [โฟลเดอร์ผลลัพธ์] [ไฟล์ .ino]
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-station}"
SKETCH="${2:-../../firmware/Station/Station.ino}"

# ตรวจลำดับการประกาศชนิดข้อมูล (Arduino แทรก prototype ไว้ก่อนฟังก์ชันแรกของไฟล์)
python3 ../check-ino-types.py "$SKETCH"

cp "$SKETCH" station.cpp
# คัดลอกไฟล์ .h ทุกตัวของสเก็ตช์ เพราะโค้ดวาดจอถูกแยกไปอยู่ StationScreen.h แล้ว
for h in "$(dirname "$SKETCH")"/*.h; do [ -e "$h" ] && cp "$h" "$(basename "$h")"; done
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable -o stpreview driver.cpp
./stpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT"
rm -f station.cpp drop_detector.h StationScreen.h stpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"

python3 make_overview.py "$OUT"
