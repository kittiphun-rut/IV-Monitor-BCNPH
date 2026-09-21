// ตัวช่วยสร้างเอกสารคู่มือตามรูปแบบราชการไทย
const d = require('docx');
const fs = require('fs');
const {
  Paragraph, TextRun, HeadingLevel, AlignmentType, Table, TableRow, TableCell,
  WidthType, ShadingType, BorderStyle, ImageRun, PageBreak, PositionalTab,
  PositionalTabAlignment, PositionalTabLeader, convertInchesToTwip,
} = d;

const FONT = 'TH SarabunPSK';
const SZ = 32;          // 16 pt (half-points)
const IMGDIR = __dirname + '/manual-img/';

const CONTENT_DXA = 11906 - 2160 - 1440;    // A4 - ขอบซ้าย 1.5" - ขอบขวา 1"

// เลขหน้าจริงของสารบัญ เติมจากการอ่าน PDF รอบแรก (ดู resolve_pages.py)
let PAGEMAP = {};
function setPageMap(m) { PAGEMAP = m || {}; }
function tocKey(t) { return t.replace(/\s+/g, ''); }

function run(text, o = {}) {
  return new TextRun({ text, font: FONT, size: o.size || SZ, bold: !!o.bold,
                       italics: !!o.italics, color: o.color, underline: o.underline ? {} : undefined });
}

// แปลงข้อความที่คร่อมด้วย ** ** ให้เป็นตัวหนา
// มีไว้กันพลาด เพราะเอกสาร Word ไม่รู้จักเครื่องหมายแบบ Markdown
// ถ้าเผลอพิมพ์ ** ลงไป จะกลายเป็นดอกจันบนหน้ากระดาษจริง
function runsFrom(text, o = {}) {
  if (Array.isArray(text)) return text;
  const parts = String(text).split('**');
  return parts
    .filter((t, i) => t !== '' || i === 0)
    .map((t, i) => run(t, { ...o, bold: o.bold || i % 2 === 1 }));
}

// ย่อหน้าธรรมดา (ย่อหน้าแรกเยื้อง 1 ซม. ตามระเบียบงานสารบรรณ)
function p(text, o = {}) {
  return new Paragraph({
    alignment: o.align || AlignmentType.THAI_DISTRIBUTE,
    spacing: { after: o.after === undefined ? 60 : o.after, line: o.line || 300 },
    indent: o.indent === undefined ? { firstLine: convertInchesToTwip(0.4) } : o.indent,
    children: runsFrom(text, o),
  });
}

function plain(text, o = {}) {
  return new Paragraph({
    alignment: o.align || AlignmentType.LEFT,
    spacing: { after: o.after === undefined ? 60 : o.after, line: o.line || 300 },
    indent: o.indent,
    children: runsFrom(text, o),
  });
}

function center(text, o = {}) { return plain(text, { ...o, align: AlignmentType.CENTER }); }

// ---- สมการแสดงผล ----
// รูปแบบเอกสารราชการ: ตัวสมการอยู่กึ่งกลางกรอบพิมพ์ เลขสมการอยู่ในวงเล็บชิดขอบขวา
// ใช้แท็บสองจุด (กึ่งกลาง + ชิดขวา) แทนการจัดย่อหน้าให้กึ่งกลาง เพราะถ้าจัดทั้งย่อหน้า
// ให้กึ่งกลาง เลขสมการจะถูกดึงเข้ามาชิดตัวสมการ ไม่ไปอยู่ที่ขอบขวาตามรูปแบบ
function eq(text, no, o = {}) {
  const kids = [run('\t', o), run(text, { bold: true, ...o })];
  if (no) kids.push(run('\t(' + no + ')', o));
  return new Paragraph({
    alignment: AlignmentType.LEFT,
    spacing: { before: o.before === undefined ? 140 : o.before,
               after:  o.after  === undefined ? 140 : o.after, line: 300 },
    tabStops: [
      { type: d.TabStopType.CENTER, position: Math.round(CONTENT_DXA / 2) },
      { type: d.TabStopType.RIGHT,  position: CONTENT_DXA },
    ],
    children: kids,
  });
}

// บรรทัดนิยามตัวแปรใต้สมการ — ย่อหน้าเข้ามาให้เห็นว่าเป็นส่วนขยายของสมการด้านบน
function eqWhere(lines) {
  return lines.map((t, i) => new Paragraph({
    alignment: AlignmentType.LEFT,
    spacing: { after: i === lines.length - 1 ? 140 : 40, line: 300 },
    indent: { left: convertInchesToTwip(1.0), hanging: convertInchesToTwip(0.45) },
    children: runsFrom(t),
  }));
}

// หัวบท: แสดงเป็นสองบรรทัดกลางหน้า แต่เป็น "ย่อหน้าเดียว" ที่ใช้สไตล์ Heading 1
// รวมไว้ย่อหน้าเดียวเพราะสารบัญอัตโนมัติของ Word ดึงข้อความจากย่อหน้าหัวข้อทั้งย่อหน้า
// ถ้าแยกเป็นสองย่อหน้า สารบัญจะได้แค่ "บทที่ ๑" โดยไม่มีชื่อบท
function chapter(no, title) {
  return [
    new Paragraph({ children: [new PageBreak()] }),
    new Paragraph({
      alignment: AlignmentType.CENTER, heading: HeadingLevel.HEADING_1,
      // ระยะบรรทัดต้องมากกว่าขนาดตัวอักษร (20pt) มิฉะนั้นสองบรรทัดของหัวบทจะซ้อนกัน
      // เห็นชัดเมื่อชื่อบทมีตัวอักษรละตินซึ่งสูงกว่าตัวอักษรไทย
      spacing: { before: 240, after: 240, line: 480 },
      children: [
        run('บทที่ ' + no + ' ', { bold: true, size: 40 }),
        new TextRun({ text: title, font: FONT, size: 40, bold: true, break: 1 }),
      ],
    }),
  ];
}

function h2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2, spacing: { before: 280, after: 90 },
    children: [run(text, { bold: true, size: 36 })],
  });
}

function h3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3, spacing: { before: 200, after: 70 },
    indent: { left: convertInchesToTwip(0.35) },
    children: [run(text, { bold: true, size: 32 })],
  });
}

// รายการลำดับด้วยมือ (คุมรูปแบบได้แน่นอนกว่า numbering config)
function item(marker, text, o = {}) {
  return new Paragraph({
    spacing: { after: o.after === undefined ? 40 : o.after, line: 300 },
    indent: { left: convertInchesToTwip(o.level ? 0.95 : 0.6), hanging: convertInchesToTwip(0.35) },
    children: [run(marker + '\t', o), ...runsFrom(text, o)],
    tabStops: [{ type: d.TabStopType.LEFT, position: convertInchesToTwip(o.level ? 0.95 : 0.6) }],
  });
}

function cell(text, o = {}) {
  return new TableCell({
    width: { size: o.w, type: WidthType.DXA },
    shading: o.fill ? { type: ShadingType.CLEAR, fill: o.fill, color: 'auto' } : undefined,
    margins: { top: 60, bottom: 60, left: 90, right: 90 },
    verticalAlign: d.VerticalAlign.CENTER,
    // แตกบรรทัดด้วย \n เป็นย่อหน้าแยก เพราะ docx ไม่รองรับ \n ในข้อความ
    children: (Array.isArray(text) ? text : [text])
      .flatMap((t) => String(t).split('\n'))
      .map((t) => new Paragraph({
        alignment: o.align || AlignmentType.LEFT,
        spacing: { after: 0, line: 288 },
        children: [run(t, { bold: o.bold, size: o.size || 30 })],
      })),
  });
}

// ตารางมาตรฐาน: แถวหัวสีเทาอ่อน ตัวหนา
function table(cols, rows, o = {}) {
  const widths = cols.map((c) => Math.round(CONTENT_DXA * c.w));
  const head = new TableRow({
    tableHeader: true,
    children: cols.map((c, i) =>
      cell(c.t, { w: widths[i], bold: true, fill: 'DCE6F1', align: AlignmentType.CENTER, size: o.size || 30 })),
  });
  const body = rows.map((r) => new TableRow({
    children: r.map((v, i) =>
      cell(v, { w: widths[i], align: cols[i].align, size: o.size || 30 })),
  }));
  return new Table({
    columnWidths: widths,
    width: { size: CONTENT_DXA, type: WidthType.DXA },
    rows: [head, ...body],
  });
}

// คำบรรยายใต้ภาพ — ใช้สไตล์ CaptionFigure เพื่อให้ TOC ของสารบัญภาพเก็บได้เอง
function caption(text) {
  return new Paragraph({
    style: 'CaptionFigure',
    alignment: AlignmentType.CENTER, spacing: { before: 60, after: 160, line: 288 },
    children: [run(text, { size: 28 })],
  });
}

// คำบรรยายเหนือตาราง — ใช้สไตล์ CaptionTable เพื่อให้ TOC ของสารบัญตารางเก็บได้เอง
function tableCaption(text) {
  return new Paragraph({
    style: 'CaptionTable',
    alignment: AlignmentType.CENTER, spacing: { before: 120, after: 100, line: 288 },
    children: [run(text, { bold: true, size: 30 })],
  });
}

// ---- จุดหมายสำหรับแทรกฟิลด์สารบัญอัตโนมัติ ----
// docx-js รุ่นนี้สร้างฟิลด์พร้อมผลลัพธ์แคชไม่ได้ จึงวางข้อความสัญลักษณ์ไว้ก่อน
// แล้วให้ inject_toc_field.py แปลงเป็นฟิลด์ TOC จริงหลังแพ็กไฟล์เสร็จ
function tocFieldBegin(instr) {
  return new Paragraph({ children: [run('@@TOC_BEGIN:' + instr + '@@')] });
}
function tocFieldEnd() {
  return new Paragraph({ children: [run('@@TOC_END@@')] });
}

function img(file, w, h) {
  return new Paragraph({
    alignment: AlignmentType.CENTER,
    spacing: { before: 120, after: 0, line: 240, lineRule: d.LineRuleType.AUTO },
    children: [new ImageRun({ type: 'png', data: fs.readFileSync(IMGDIR + file),
                              transformation: { width: w, height: h } })],
  });
}

// ภาพสองรูปเรียงข้างกันโดยไม่มีเส้นตาราง
function imgPair(a, b, w, h) {
  const none = { style: BorderStyle.NONE, size: 0, color: 'FFFFFF' };
  const bd = { top: none, bottom: none, left: none, right: none, insideHorizontal: none, insideVertical: none };
  const half = Math.round(CONTENT_DXA / 2);
  const mk = (f) => new TableCell({
    width: { size: half, type: WidthType.DXA }, borders: bd,
    children: [img(f.file, w, h), caption(f.cap)],
  });
  return new Table({ columnWidths: [half, half], width: { size: CONTENT_DXA, type: WidthType.DXA },
                     borders: bd,
                     rows: [new TableRow({ cantSplit: true, children: [mk(a), mk(b)] })] });
}

// บรรทัดสารบัญพร้อมจุดไข่ปลาและเลขหน้าชิดขวา
function toc(text, page, o = {}) {
  const left = convertInchesToTwip(o.level ? 0.35 * o.level : 0);
  return new Paragraph({
    spacing: { after: 40, line: 300 },
    indent: { left },
    tabStops: [{ type: d.TabStopType.RIGHT, position: CONTENT_DXA, leader: d.LeaderType.DOT }],
    children: [run(text, { bold: o.bold }),
               run('\t' + String(PAGEMAP[tocKey(text)] !== undefined ? PAGEMAP[tocKey(text)] : page),
                   { bold: o.bold })],
  });
}

// กล่องคำเตือน: กรอบเดียว พื้นอ่อน
function noteBox(title, lines, fill = 'FFF2CC', border = 'BF8F00') {
  const bd = { style: BorderStyle.SINGLE, size: 8, color: border };
  return new Table({
    columnWidths: [CONTENT_DXA],
    width: { size: CONTENT_DXA, type: WidthType.DXA },
    borders: { top: bd, bottom: bd, left: bd, right: bd,
               insideHorizontal: bd, insideVertical: bd },
    rows: [new TableRow({ children: [new TableCell({
      width: { size: CONTENT_DXA, type: WidthType.DXA },
      shading: { type: ShadingType.CLEAR, fill, color: 'auto' },
      margins: { top: 120, bottom: 120, left: 160, right: 160 },
      children: [
        new Paragraph({ spacing: { after: 60 }, children: [run(title, { bold: true })] }),
        ...lines.map((l) => new Paragraph({
          spacing: { after: 40, line: 300 },
          indent: { left: convertInchesToTwip(0.25), hanging: convertInchesToTwip(0.25) },
          children: [run(l)] })),
      ],
    })] })],
  });
}

module.exports = { d, FONT, SZ, CONTENT_DXA, setPageMap, tocKey, tocFieldBegin, tocFieldEnd, runsFrom, eq, eqWhere,
                   tableCaption, run, p, plain, center, chapter, h2, h3,
                   item, table, cell, caption, img, imgPair, toc, noteBox };
