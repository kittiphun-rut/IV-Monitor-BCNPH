#!/usr/bin/env bash
# เรนเดอร์แบบร่าง UI แบบสมาร์ตโฟนสำหรับจอสัมผัส 2.4" (240x320)
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/ui-concepts-touch}"
mkdir -p "$OUT"
g++ -std=gnu++17 -I../screen-preview -Wall -Wno-unused-variable -o mockup touch_ui_concepts.cpp mockup_runtime.cpp
./mockup ops.txt
python3 ../screen-preview/render.py ops.txt "$OUT" 240 320 3
rm -f mockup ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
