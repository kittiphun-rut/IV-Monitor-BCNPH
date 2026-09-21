#!/usr/bin/env bash
# ทดสอบเส้นทางรับข้อมูล ESP-NOW ของ Host บนเครื่อง PC (ไม่ต้องมีบอร์ด)
set -euo pipefail
cd "$(dirname "$0")"
SRC="${1:-../../firmware/ESP32-S3-Host-OLED-V_4_7_4}"
NAME="$(basename "$SRC")"
cp "$SRC/$NAME.ino" host_oled.cpp
cp "$SRC/web_dashboard.h" web_dashboard.h
g++ -std=gnu++17 -I../screen-preview -I. -DESP_ARDUINO_VERSION_MAJOR=3 -w -o hostrx driver.cpp
./hostrx; rc=$?
rm -f host_oled.cpp web_dashboard.h hostrx
exit $rc
