#!/usr/bin/env bash
# สร้างภาพพรีวิวหน้าจอ TFT จากไฟล์ .ino โดยตรง (ไม่ต้องแฟลชลงบอร์ด)
#   ใช้: ./run.sh [ไดเรกทอรีปลายทางของภาพ]
# หลักการ: คอมไพล์สเก็ตช์บนเครื่อง PC ด้วย Arduino API จำลอง บันทึกคำสั่งวาดทุกคำสั่ง
#          แล้วให้ render.py เรนเดอร์ออกเป็นภาพ PNG ขนาด 172x320 (ขยาย 3 เท่า)
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/screens}"
SKETCH="../../firmware/ESP32-S3-FlowSim-Lab/ESP32-S3-FlowSim-Lab.ino"

cp "$SKETCH" sketch.cpp
g++ -std=gnu++17 -I. -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable -o preview driver.cpp
./preview ops.txt
python3 render.py ops.txt "$OUT"
rm -f sketch.cpp preview ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
