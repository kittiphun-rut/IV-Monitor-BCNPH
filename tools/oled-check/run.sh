#!/usr/bin/env bash
# ตรวจว่าเฟิร์มแวร์ Host รุ่นจอ OLED คอมไพล์ผ่านบนเครื่อง PC
# (จอ OLED เป็นขาวดำ 128x64 และ U8g2 ใช้พิกัดเส้นฐานของตัวอักษร จึงไม่เรนเดอร์เป็นภาพ
#  เครื่องมือนี้เน้นจับข้อผิดพลาดตอนคอมไพล์ ซึ่งเป็นสิ่งที่พลาดบ่อยที่สุด)
set -euo pipefail
cd "$(dirname "$0")"
SKETCH_DIR="${1:-../../firmware/ESP32-S3-Host-OLED-V_4_7_1}"
NAME="$(basename "$SKETCH_DIR")"

cp "$SKETCH_DIR/$NAME.ino" host_oled.cpp
cp "$SKETCH_DIR/web_dashboard.h" web_dashboard.h
g++ -std=gnu++17 -I../screen-preview -I. -DESP_ARDUINO_VERSION_MAJOR=3 \
    -Wall -Wno-unused-variable -Wno-format-truncation -fsyntax-only driver.cpp
rm -f host_oled.cpp web_dashboard.h
echo "คอมไพล์ผ่าน: $NAME"
