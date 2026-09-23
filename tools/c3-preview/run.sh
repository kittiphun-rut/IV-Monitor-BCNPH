#!/usr/bin/env bash
# เรนเดอร์หน้าจอ OLED 0.42" (72x40) ของเฟิร์มแวร์ Station-C3-OLED ทุกหน้า โดยไม่ต้องมีบอร์ด
# ใช้: run.sh [โฟลเดอร์สเก็ตช์] [โฟลเดอร์ผลลัพธ์]
set -euo pipefail
cd "$(dirname "$0")"
SKETCH_DIR="${1:-../../firmware/Station-C3-OLED}"
OUT="${2:-../../docs/screens-c3}"
NAME="$(basename "$SKETCH_DIR")"
mkdir -p "$OUT"

# ตรวจลำดับการประกาศชนิดข้อมูล (Arduino แทรก prototype ไว้ก่อนฟังก์ชันแรกของไฟล์)
python3 ../check-ino-types.py "$SKETCH_DIR/$NAME.ino"

cp "$SKETCH_DIR/$NAME.ino" station_c3.cpp
for h in "$SKETCH_DIR"/*.h; do [ -e "$h" ] && cp "$h" "$(basename "$h")"; done
g++ -std=gnu++17 -I../screen-preview -I. -DESP_ARDUINO_VERSION_MAJOR=3 \
    -Wall -Wno-unused-variable -Wno-format-truncation -o c3preview driver.cpp
./c3preview ops.txt
OLED_W=72 OLED_H=40 python3 ../oled-preview/render_oled.py ops.txt "$OUT" 8
python3 make_overview.py "$OUT"
rm -f station_c3.cpp StationScreen.h drop_detector.h c3preview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
