#!/usr/bin/env bash
# ตรวจปัญหาที่เกิดจากการแยกโค้ดวาดจอออกไปไว้ในไฟล์ .h ของสเก็ตช์เดียวกัน
# เครื่องมือนี้ยกมาจากโครงการ MCUPR-CANTEEN ของผู้พัฒนาคนเดียวกัน
set -euo pipefail
cd "$(dirname "$0")/../.."
python3 tools/split-check/splitcheck.py "$@"
