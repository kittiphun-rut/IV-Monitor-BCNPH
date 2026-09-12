#!/usr/bin/env python3
"""ตรวจว่าโครงสร้างแพ็กเก็ต ESP-NOW ของทุกเฟิร์มแวร์ตรงกันจริง (Protocol v3)

Host กับ Station ส่งโครงสร้างนี้หากันตรง ๆ ถ้าลำดับหรือชนิดของฟิลด์ต่างกันแม้ไบต์เดียว
ข้อมูลจะเพี้ยนทั้งระบบโดยไม่มีอะไรฟ้อง สคริปต์นี้ดึงโครงสร้างจากไฟล์ .ino ทุกตัว
แล้วเทียบกันทีละฟิลด์ ใช้เป็นด่านกันพลาดก่อน merge

ใช้: python3 check_protocol.py     (คืนค่า 1 เมื่อพบความต่าง)
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
STRUCTS = ("struct_message", "struct_host_sync")
SIZES = {"uint8_t": 1, "int8_t": 1, "uint16_t": 2, "int16_t": 2,
         "uint32_t": 4, "int32_t": 4, "float": 4}


def extract(path: Path, name: str):
    """ดึงรายชื่อ (ชนิด, ฟิลด์) ของโครงสร้างจากไฟล์ .ino"""
    text = path.read_text(encoding="utf-8")
    m = re.search(r"typedef struct __attribute__\(\(packed\)\) " + name +
                  r"\s*\{(.*?)\}\s*" + name + r"\s*;", text, re.S)
    if not m:
        return None
    fields = []
    for line in m.group(1).splitlines():
        line = line.split("//")[0].strip()
        if not line or not line.endswith(";"):
            continue
        parts = line[:-1].split()
        if len(parts) >= 2:
            fields.append((parts[0], parts[1]))
    return fields


def size_of(fields):
    return sum(SIZES.get(t, -9999) for t, _ in fields)


def main() -> int:
    inos = sorted(ROOT.glob("firmware/*/*.ino"))
    problems = 0

    for name in STRUCTS:
        print(f"\n=== {name}")
        reference = None
        ref_file = None
        for ino in inos:
            fields = extract(ino, name)
            if fields is None:
                continue
            label = ino.parent.name
            if reference is None:
                reference, ref_file = fields, label
                print(f"  {label:34s} {size_of(fields):3d} ไบต์  (ใช้เป็นตัวอ้างอิง)")
                continue
            if fields == reference:
                print(f"  {label:34s} {size_of(fields):3d} ไบต์  ตรงกัน")
            else:
                problems += 1
                print(f"  {label:34s} {size_of(fields):3d} ไบต์  ** ไม่ตรงกับ {ref_file} **")
                for i in range(max(len(fields), len(reference))):
                    a = reference[i] if i < len(reference) else ("-", "-")
                    b = fields[i] if i < len(fields) else ("-", "-")
                    if a != b:
                        print(f"      ฟิลด์ที่ {i}: อ้างอิง {a[0]} {a[1]}  <->  พบ {b[0]} {b[1]}")

    print()
    if problems:
        print(f"พบความต่าง {problems} จุด — Host กับ Station จะสื่อสารกันเพี้ยน")
        return 1
    print("โครงสร้างแพ็กเก็ตตรงกันทุกเฟิร์มแวร์")
    return 0


if __name__ == "__main__":
    sys.exit(main())
