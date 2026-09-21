#!/usr/bin/env python3
"""แปลงคู่มือ .docx เป็นไฟล์ข้อความล้วน (.txt)

อ่านจากไฟล์ .docx ที่สร้างเสร็จแล้ว จึงได้เนื้อหาตรงกับเล่มจริงเสมอ
รวมทั้งตารางและเลขหน้าในสารบัญ

ใช้: python3 make_plaintext.py ไฟล์.docx ไฟล์ผลลัพธ์.txt [ความกว้าง]
"""
import io, sys, unicodedata
from docx import Document
from docx.table import Table
from docx.text.paragraph import Paragraph

WIDTH = int(sys.argv[3]) if len(sys.argv) > 3 else 78

# ภาษาไทยไม่มีช่องว่างระหว่างคำ ถ้าตัดบรรทัดตามจำนวนอักขระจะได้คำขาดกลางคำ
# ถ้ามี pythainlp ให้ใช้ตัดคำ ถ้าไม่มีก็ยอมให้บรรทัดยาวเกิน ดีกว่าตัดคำผิด
try:
    from pythainlp import word_tokenize as _tok
    def segments(s):
        return [w for w in _tok(s, engine='newmm') if w != '']
except Exception:
    def segments(s):
        return [s]

# หัวข้อของส่วนนำที่ไม่ได้ใช้สไตล์ Heading แต่ควรเด่นในไฟล์ข้อความ
FRONT_TITLES = {'คำนำ', 'สารบัญ', 'สารบัญตาราง', 'สารบัญภาพ', 'ภาคผนวก', 'เอกสารอ้างอิง'}


def dwidth(s):
    """ความกว้างที่ตาเห็น — สระบนล่างและวรรณยุกต์ไทยไม่กินที่"""
    return sum(0 if unicodedata.combining(c) else 1 for c in s)


def pad(s, n):
    return s + ' ' * max(0, n - dwidth(s))


def wrap(text, width, indent=''):
    """ตัดบรรทัดโดยนับความกว้างแบบที่ตาเห็น และไม่ตัดกลางคำไทย"""
    out, avail = [], width - len(indent)
    for para in text.split('\n'):
        line = ''
        for chunk in para.split(' '):
            for i, w in enumerate(segments(chunk)):
                cand = line + w
                if dwidth(cand) <= avail or line == '':
                    line = cand
                else:
                    out.append(indent + line.rstrip())
                    line = w
            # คืนช่องว่างระหว่างกลุ่มที่คั่นด้วยเว้นวรรคในต้นฉบับ
            if dwidth(line) + 1 <= avail:
                line += ' '
            else:
                out.append(indent + line.rstrip())
                line = ''
        if line.strip():
            out.append(indent + line.rstrip())
    return out or ['']


def para_text(p):
    """ข้อความของย่อหน้า แปลงแท็บเป็นตัวคั่น เพื่อให้บรรทัดสารบัญยังอ่านออก"""
    parts = []
    for r in p.runs:
        parts.append(r.text)
        if r._element.findall('{http://schemas.openxmlformats.org/wordprocessingml/2006/main}tab'):
            parts.append('\t')
    t = ''.join(parts)
    return t.replace('\xa0', ' ').rstrip()


def render_table(tbl, width):
    # เก็บทีละย่อหน้าในช่อง เพื่อไม่ให้บรรทัดที่ตั้งใจแยกไว้ถูกเชื่อมติดกัน
    rows = [[[para_text(p) for p in c.paragraphs if para_text(p).strip()] for c in r.cells]
            for r in tbl.rows]
    if not rows:
        return []
    ncol = max(len(r) for r in rows)
    rows = [r + [[]] * (ncol - len(r)) for r in rows]

    # แบ่งความกว้างตามสัดส่วนของเนื้อหาจริง แต่ไม่ให้คอลัมน์ไหนแคบกว่า ๘
    need = [max((max((dwidth(t) for t in r[i]), default=1)) for r in rows) or 1
            for i in range(ncol)]
    inner = width - (ncol + 1) * 3
    total = sum(need) or 1
    col = [max(8, int(inner * n / total)) for n in need]

    def rule(ch='-'):
        return '+' + '+'.join(ch * (c + 2) for c in col) + '+'

    out = [rule('=')]
    for ri, r in enumerate(rows):
        cells = []
        for i, paras in enumerate(r):
            lines = []
            for t in paras:
                lines += wrap(t, col[i])
            cells.append(lines or [''])
        for k in range(max(len(c) for c in cells)):
            out.append('| ' + ' | '.join(
                pad(cells[i][k] if k < len(cells[i]) else '', col[i]) for i in range(ncol)) + ' |')
        out.append(rule('=' if ri == 0 else '-'))
    return out


def body_items(doc):
    """เดินตามลำดับจริงในเอกสาร ทั้งย่อหน้าและตาราง"""
    ns = '{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
    for child in doc.element.body.iterchildren():
        if child.tag == ns + 'p':
            yield Paragraph(child, doc)
        elif child.tag == ns + 'tbl':
            yield Table(child, doc)


def main():
    src, dst = sys.argv[1], sys.argv[2]
    doc = Document(src)
    out = []

    for it in body_items(doc):
        if isinstance(it, Table):
            out.append('')
            out.extend(render_table(it, WIDTH))
            out.append('')
            continue

        style = ((it.style.name if it.style is not None else '') or '').lower()
        text = para_text(it)
        if not text:
            if out and out[-1] != '':
                out.append('')
            continue

        # สมการ: ย่อหน้าขึ้นต้นด้วยแท็บ (ตัวสมการกึ่งกลาง เลขสมการชิดขวา)
        # ต้องตรวจก่อนบรรทัดสารบัญ เพราะทั้งคู่ใช้แท็บเหมือนกัน แต่สมการไม่มีจุดไข่ปลา
        if text.startswith('\t'):
            body, _, num = text.lstrip('\t').rpartition('\t')
            body, num = body.strip(), num.strip()
            if not body:
                body, num = num, ''
            line = ' ' * max(0, (WIDTH - dwidth(body)) // 2) + body
            if num:
                line += ' ' * max(1, WIDTH - dwidth(line) - dwidth(num)) + num
            out.append(line)
            continue

        # บรรทัดสารบัญ: ข้อความ ... เลขหน้า
        if '\t' in text:
            left, _, right = text.rpartition('\t')
            left, right = left.strip(), right.strip()
            dots = max(3, WIDTH - dwidth(left) - dwidth(right) - 2)
            out.append(left + ' ' + '.' * dots + ' ' + right)
            continue

        if text in FRONT_TITLES and style not in ('heading 1', 'heading 2', 'heading 3'):
            out += ['', '=' * WIDTH, '  ' + text, '=' * WIDTH, '']
        elif style == 'heading 1':
            out += ['', '=' * WIDTH]
            out += ['  ' + l for l in text.split('\n')]
            out += ['=' * WIDTH, '']
        elif style == 'heading 2':
            out += ['', text, '-' * min(WIDTH, dwidth(text) + 6), '']
        elif style == 'heading 3':
            out += ['', '  ' + text, '  ' + '~' * min(WIDTH - 2, dwidth(text) + 4), '']
        elif style == 'caption table':
            out += ['', '[ตาราง] ' + text]
        elif style == 'caption figure':
            out += ['[ภาพ]   ' + text, '']
        else:
            out += wrap(text, WIDTH, '  ')

    # ยุบบรรทัดว่างที่ติดกันเกินสองบรรทัด
    clean, blank = [], 0
    for l in out:
        blank = blank + 1 if l.strip() == '' else 0
        if blank <= 2:
            clean.append(l.rstrip())

    with io.open(dst, 'w', encoding='utf-8', newline='\r\n') as f:
        f.write('\n'.join(clean).lstrip('\n') + '\n')
    print('เขียนแล้ว:', dst, len(clean), 'บรรทัด')


if __name__ == '__main__':
    main()
