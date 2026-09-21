#!/usr/bin/env bash
# วัดผลของการแก้จังหวะส่ง ESP-NOW ตั้งแต่ 2 ถึง 8 เตียง
set -euo pipefail
cd "$(dirname "$0")"
for n in 2 4 6 8; do
  python3 sim_link.py "$n" 60
  echo
done
