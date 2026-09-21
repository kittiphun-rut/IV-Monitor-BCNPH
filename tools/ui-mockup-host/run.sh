#!/usr/bin/env bash
# เรนเดอร์แบบร่าง UI ของเครื่อง Host (จอ TFT 2.8" ST7789V) ทั้ง 3 แนวทาง
# แบบ A/B เป็นจอแนวนอน 320x240, แบบ C เป็นจอแนวตั้ง 240x320 จึงเรนเดอร์แยกกัน
set -euo pipefail
cd "$(dirname "$0")"
OUT="${1:-../../docs/ui-concepts-host}"
mkdir -p "$OUT"
g++ -std=gnu++17 -I../screen-preview -DESP_ARDUINO_VERSION_MAJOR=3 -Wall -Wno-unused-variable \
    -o mockup host_ui_concepts.cpp mockup_runtime.cpp
./mockup ops.txt

# แยกคำสั่งวาดของแต่ละแบบตามเครื่องหมาย MARK แล้วเรนเดอร์ด้วยขนาดจอที่ถูกต้อง
python3 - "$OUT" << 'PY'
import subprocess, sys
out = sys.argv[1]
ops = open("ops.txt", encoding="utf-8").read().split("\n")
groups, cur = [], []
for line in ops:
    cur.append(line)
    if line.startswith("MARK "):
        groups.append((line.split()[1], cur)); cur = []
for name, lines in groups:
    size = (240, 320) if name.startswith("C") else (320, 240)
    open(f"_{name}.txt", "w", encoding="utf-8").write("\n".join(lines) + "\n")
    subprocess.run(["python3", "../screen-preview/render.py", f"_{name}.txt", out,
                    str(size[0]), str(size[1]), "3"], check=True)
    subprocess.run(["rm", "-f", f"_{name}.txt"], check=True)
PY
rm -f mockup ops.txt
echo "เสร็จแล้ว: ภาพอยู่ใน $OUT"
