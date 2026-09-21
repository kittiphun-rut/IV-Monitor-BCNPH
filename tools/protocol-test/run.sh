#!/usr/bin/env bash
# ตรวจความเข้ากันได้ของโปรโตคอล ESP-NOW ระหว่าง Host กับ Station ทุกเวอร์ชัน
set -euo pipefail
cd "$(dirname "$0")"
python3 check_protocol.py
