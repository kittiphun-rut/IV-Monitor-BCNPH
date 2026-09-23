#!/usr/bin/env bash
# เรนเดอร์ภาพหน้าจอ TFT 2.8" ของเฟิร์มแวร์ Host รุ่นจอ TFT ทุกหน้า (จำลอง Arduino API บน PC)
# ใช้: run.sh [โฟลเดอร์ผลลัพธ์] [โฟลเดอร์สเก็ตช์]
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens-host}"
SKETCH_DIR="${2:-../../firmware/Host-TFT28}"

# ตรวจลำดับการประกาศชนิดข้อมูล (Arduino แทรก prototype ไว้ก่อนฟังก์ชันแรกของไฟล์)
python3 ../check-ino-types.py "$SKETCH_DIR/$(basename "$SKETCH_DIR").ino"

cp "$SKETCH_DIR/$(basename "$SKETCH_DIR").ino" host.cpp
# คัดลอกไฟล์ .h ทุกตัวของสเก็ตช์ ไม่ใช่แค่ web_dashboard.h
# เพราะโค้ดวาดจอถูกแยกไปอยู่ HostScreen.h / StationScreen.h แล้ว
for h in "$SKETCH_DIR"/*.h; do [ -e "$h" ] && cp "$h" "$(basename "$h")"; done
g++ -std=gnu++17 -I../screen-preview -Wall -Wno-unused-variable -Wno-format-truncation -o hostpreview driver.cpp
./hostpreview ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT" 320 240 3
rm -f host.cpp web_dashboard.h HostScreen.h hostpreview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
