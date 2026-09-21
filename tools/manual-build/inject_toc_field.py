#!/usr/bin/env python3
"""แปลงสัญลักษณ์ @@TOC_BEGIN:...@@ / @@TOC_END@@ ในไฟล์ .docx ให้เป็นฟิลด์ TOC จริงของ Word

ทำไมต้องมีขั้นตอนนี้
  ไลบรารี docx ที่ใช้สร้างเล่มสร้างฟิลด์ TOC ได้ แต่สร้างแบบ "ว่างเปล่า" เท่านั้น
  ผู้ใช้ต้องกดอัปเดตก่อนจึงจะเห็นรายการ ถ้าเปิดแล้วสั่งพิมพ์เลยจะได้หน้าว่าง

  สคริปต์นี้จึงประกอบฟิลด์เอง โดยเก็บ "ผลลัพธ์ที่คำนวณไว้แล้ว" ไว้ในฟิลด์ด้วย
  ผลคือ
    - เปิดแล้วพิมพ์ได้ทันที เพราะมีรายการและเลขหน้าอยู่ครบ
    - กด Ctrl+A แล้ว F9 (หรือคลิกขวา > Update Field) Word จะสร้างใหม่ให้เอง
    - ตั้ง w:dirty ไว้ด้วย Word จึงอัปเดตให้อัตโนมัติตั้งแต่ตอนเปิดไฟล์

ใช้: python3 inject_toc_field.py ไฟล์.docx
"""
import re, shutil, sys, tempfile, zipfile, os

RPR = '<w:rPr><w:rFonts w:ascii="TH SarabunPSK" w:cs="TH SarabunPSK" w:hAnsi="TH SarabunPSK"/><w:sz w:val="32"/><w:szCs w:val="32"/></w:rPr>'

def xml_escape(t):
    return t.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;').replace('"', '&quot;')

def field_begin_runs(instr):
    return ('<w:r>' + RPR + '<w:fldChar w:fldCharType="begin" w:dirty="true"/></w:r>'
            '<w:r>' + RPR + '<w:instrText xml:space="preserve"> ' + xml_escape(instr) + ' </w:instrText></w:r>'
            '<w:r>' + RPR + '<w:fldChar w:fldCharType="separate"/></w:r>')

FIELD_END_RUN = '<w:r>' + RPR + '<w:fldChar w:fldCharType="end"/></w:r>'

PARA = re.compile(r'<w:p\b[^>]*>.*?</w:p>', re.S)
PPR  = re.compile(r'^(<w:p\b[^>]*>)(<w:pPr>.*?</w:pPr>)?', re.S)


def inject(xml):
    """ย้ายจุดเริ่ม/จบฟิลด์เข้าไปในย่อหน้าแรกและย่อหน้าสุดท้ายของรายการ"""
    paras = PARA.findall(xml)
    # ตรวจว่าจำนวนสัญลักษณ์ครบคู่
    begins = [i for i, p in enumerate(paras) if '@@TOC_BEGIN:' in p]
    ends   = [i for i, p in enumerate(paras) if '@@TOC_END@@' in p]
    if len(begins) != len(ends):
        raise SystemExit('สัญลักษณ์ TOC ไม่ครบคู่: begin %d, end %d' % (len(begins), len(ends)))
    if not begins:
        print('ไม่พบสัญลักษณ์ TOC — ข้ามขั้นตอนนี้')
        return xml, 0

    out = []
    for b, e in zip(begins, ends):
        if not (b + 1 < e):
            raise SystemExit('ช่วงรายการสารบัญว่างเปล่า')
        m = re.search(r'@@TOC_BEGIN:(.*?)@@', paras[b], re.S)
        instr = re.sub(r'<[^>]+>', '', m.group(1))          # ตัดแท็กที่อาจคั่นกลางข้อความออก
        instr = instr.replace('&quot;', '"').replace('&amp;', '&')
        out.append((b, e, instr))

    for b, e, instr in out:
        first, last = paras[b + 1], paras[e - 1]
        mm = PPR.match(first)
        head = mm.group(0)
        paras[b + 1] = head + field_begin_runs(instr) + first[len(head):]
        paras[e - 1] = paras[e - 1] if e - 1 != b + 1 else paras[b + 1]
        paras[e - 1] = paras[e - 1][:-len('</w:p>')] + FIELD_END_RUN + '</w:p>'
        paras[b] = None          # ลบย่อหน้าสัญลักษณ์ทิ้ง
        paras[e] = None

    # ประกอบ XML ใหม่โดยแทนย่อหน้าตามลำดับเดิม
    it = iter(paras)
    def repl(_m):
        return next(it)
    body = PARA.sub(lambda m: (lambda v: '' if v is None else v)(next(it)), xml)
    return body, len(out)


def main():
    path = sys.argv[1]
    with tempfile.TemporaryDirectory() as tmp:
        with zipfile.ZipFile(path) as z:
            names = z.namelist()
            z.extractall(tmp)
        doc = os.path.join(tmp, 'word', 'document.xml')
        xml = open(doc, encoding='utf-8').read()
        xml, n = inject(xml)
        open(doc, 'w', encoding='utf-8').write(xml)

        out = path + '.tmp'
        with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED) as z:
            for name in names:
                z.write(os.path.join(tmp, name), name)
        shutil.move(out, path)
    print('แทรกฟิลด์สารบัญอัตโนมัติแล้ว %d ชุด' % n)


if __name__ == '__main__':
    main()
