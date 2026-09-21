#!/usr/bin/env python3
"""ตรวจว่าชนิดข้อมูลที่ใช้ในลายเซ็นฟังก์ชันถูกประกาศไว้ก่อน "จุดแทรก prototype" ของ Arduino

Arduino IDE สร้าง prototype ของทุกฟังก์ชันแล้วแทรกไว้ก่อนฟังก์ชันแรกของไฟล์ .ino
ถ้า enum/struct ถูกประกาศไว้กลางไฟล์ prototype จะอ้างถึงชนิดที่ยังไม่รู้จัก
แล้วขึ้น error ว่า 'XXX does not name a type' ทั้งที่คอมไพล์เป็น .cpp ธรรมดาผ่าน

ใช้: python3 check-ino-types.py [ไฟล์ .ino ...]     (ไม่ใส่ = ตรวจทุกไฟล์ใน ../firmware)
คืนค่า 1 เมื่อพบปัญหา
"""
import re
import sys
from pathlib import Path

FUNC_DEF = re.compile(r'^[A-Za-z_][A-Za-z0-9_:<>\*& ]*\**[A-Za-z_][A-Za-z0-9_]*\s*\(([^;{]*)\)\s*\{')
TYPE_DEF = re.compile(r'^\s*(?:enum|struct|class|union)\s+([A-Za-z_][A-Za-z0-9_]*)')
TYPEDEF_END = re.compile(r'^\s*\}\s*([A-Za-z_][A-Za-z0-9_]*)\s*;')
WORD = re.compile(r'[A-Za-z_][A-Za-z0-9_]*')


def check(path: Path) -> list:
    lines = path.read_text(encoding='utf-8').splitlines()

    types = {}                      # ชื่อชนิด -> บรรทัดที่ประกาศ
    in_typedef = False
    for n, line in enumerate(lines, 1):
        m = TYPE_DEF.match(line)
        if m and not line.lstrip().startswith('//'):
            types.setdefault(m.group(1), n)
        if line.startswith('typedef struct'):
            in_typedef = True
        elif in_typedef:
            m = TYPEDEF_END.match(line)
            if m:
                types.setdefault(m.group(1), n)
                in_typedef = False

    funcs = [(n, m.group(0)) for n, line in enumerate(lines, 1)
             if (m := FUNC_DEF.match(line)) and not line.lstrip().startswith('//')]
    if not funcs:
        return []
    insert_at = funcs[0][0]         # Arduino แทรก prototype ไว้ก่อนบรรทัดนี้

    problems = []
    for n, sig in funcs:
        for word in set(WORD.findall(sig)):
            decl = types.get(word)
            if decl is not None and decl >= insert_at:
                problems.append(
                    f"{path}:{n}: ใช้ชนิด '{word}' ในลายเซ็นฟังก์ชัน "
                    f"แต่ประกาศไว้บรรทัด {decl} ซึ่งอยู่หลังจุดแทรก prototype (บรรทัด {insert_at})")
    return sorted(set(problems))


def main() -> int:
    args = sys.argv[1:]
    files = [Path(a) for a in args] if args else sorted(
        (Path(__file__).resolve().parent.parent / 'firmware').glob('*/*.ino'))
    bad = []
    for f in files:
        found = check(f)
        bad += found
        print(("พบปัญหา " if found else "ผ่าน     ") + str(f))
        for line in found:
            print("   " + line)
    if bad:
        print(f"\nรวม {len(bad)} จุดที่ต้องย้ายการประกาศชนิดข้อมูลขึ้นไปไว้ตอนต้นไฟล์")
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
