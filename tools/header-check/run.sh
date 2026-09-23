#!/usr/bin/env bash
# ตรวจว่าหัวไฟล์ Doxygen และป้ายเวอร์ชันในเนื้อโค้ดเป็นไปตาม docs/CODE-STANDARD.md
# มาตรฐานและตัวตรวจยกมาจากโครงการ MCUPR-CANTEEN ของผู้พัฒนาคนเดียวกัน
set -euo pipefail
cd "$(dirname "$0")/../.."
python3 tools/header-check/headercheck.py "$@"
