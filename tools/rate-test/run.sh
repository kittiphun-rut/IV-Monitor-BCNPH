#!/usr/bin/env bash
# เทียบสูตรคำนวณอัตราการไหลแบบเดิม (EMA ของส่วนกลับ) กับแบบใหม่ (หน้าต่างเวลาสะสม)
# บนเครื่อง PC ไม่ต้องมีบอร์ด — ใช้ยืนยันว่าค่าที่อ่านได้อยู่ในกรอบ +-10% ที่คู่มือใช้
set -euo pipefail
cd "$(dirname "$0")"
g++ -O2 -std=gnu++17 -Wall -o ratetest rate_test.cpp
./ratetest; rc=$?
rm -f ratetest
exit $rc
