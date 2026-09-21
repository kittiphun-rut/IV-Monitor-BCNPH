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

// ย่อหน้าธรรมดา (ย่อหน้าแรกเยื้อง 1 ซม. ตามระเบียบงานสารบรรณ)
function p(text, o = {}) {
  return new Paragraph({
    alignment: o.align || AlignmentType.THAI_DISTRIBUTE,
    spacing: { after: o.after === undefined ? 60 : o.after, line: o.line || 300 },
    indent: o.indent === undefined ? { firstLine: convertInchesToTwip(0.4) } : o.indent,
    children: Array.isArray(text) ? text : [run(text, o)],
  });
}

function plain(text, o = {}) {
  return new Paragraph({
    alignment: o.align || AlignmentType.LEFT,
    spacing: { after: o.after === undefined ? 60 : o.after, line: o.line || 300 },
    indent: o.indent,
    children: Array.isArray(text) ? text : [run(text, o)],
  });
}

function center(text, o = {}) { return plain(text, { ...o, align: AlignmentType.CENTER }); }

// หัวบท: "บทที่ n" กลางหน้า แล้วชื่อบทกลางหน้า
function chapter(no, title) {
  return [
    new Paragraph({ children: [new PageBreak()] }),
    new Paragraph({
      alignment: AlignmentType.CENTER, heading: HeadingLevel.HEADING_1,
      spacing: { before: 240, after: 0 },
      children: [run('บทที่ ' + no, { bold: true, size: 40 })],
    }),
    new Paragraph({
      alignment: AlignmentType.CENTER, spacing: { after: 240 },
      children: [run(title, { bold: true, size: 40 })],
    }),
  ];
}

function h2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2, spacing: { before: 200, after: 80 },
    children: [run(text, { bold: true, size: 36 })],
  });
}

function h3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3, spacing: { before: 140, after: 60 },
    indent: { left: convertInchesToTwip(0.3) },
    children: [run(text, { bold: true, size: 32 })],
  });
}

// รายการลำดับด้วยมือ (คุมรูปแบบได้แน่นอนกว่า numbering config)
function item(marker, text, o = {}) {
  return new Paragraph({
    spacing: { after: o.after === undefined ? 40 : o.after, line: 300 },
    indent: { left: convertInchesToTwip(o.level ? 0.95 : 0.6), hanging: convertInchesToTwip(0.35) },
    children: [run(marker + '\t', o), run(text, o)],
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

function caption(text) {
  return new Paragraph({
    alignment: AlignmentType.CENTER, spacing: { before: 60, after: 160, line: 288 },
    children: [run(text, { size: 28 })],
  });
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
                     borders: bd, rows: [new TableRow({ children: [mk(a), mk(b)] })] });
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

module.exports = { d, FONT, SZ, CONTENT_DXA, setPageMap, tocKey, run, p, plain, center, chapter, h2, h3,
                   item, table, cell, caption, img, imgPair, toc, noteBox };
