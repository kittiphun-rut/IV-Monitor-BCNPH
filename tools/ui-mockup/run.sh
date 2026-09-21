#!/usr/bin/env bash
# เรนเดอร์แบบร่าง UI ทั้ง 3 แนวทางเป็นภาพ PNG
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/ui-concepts}"
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable \
    -o mockup station_ui_concepts.cpp mockup_runtime.cpp
./mockup ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT"
rm -f mockup ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
