#!/usr/bin/env bash
# ทดสอบเส้นทางรับข้อมูล ESP-NOW ของ Host บนเครื่อง PC (ไม่ต้องมีบอร์ด)
set -euo pipefail
cd "$(dirname "$0")"
SRC="${1:-../../firmware/Host-OLED}"
NAME="$(basename "$SRC")"
cp "$SRC/$NAME.ino" host_oled.cpp
# คัดลอกไฟล์ .h ทุกตัวของสเก็ตช์ ไม่ใช่แค่ web_dashboard.h
# เพราะโค้ดวาดจอถูกแยกไปอยู่ HostScreen.h / StationScreen.h แล้ว
for h in "$SRC"/*.h; do [ -e "$h" ] && cp "$h" "$(basename "$h")"; done
g++ -std=gnu++17 -I../screen-preview -I. -DESP_ARDUINO_VERSION_MAJOR=3 -w -o hostrx driver.cpp
./hostrx; rc=$?
rm -f host_oled.cpp web_dashboard.h HostScreen.h hostrx
exit $rc
