#!/usr/bin/env bash
# ทดสอบตัวตรวจจับหยดบนเครื่อง PC (ไม่ต้องมีบอร์ด)
set -euo pipefail
cd "$(dirname "$0")"
g++ -std=gnu++17 -O2 -Wall -o detector_test test_detector.cpp -lm
./detector_test
rm -f detector_test
