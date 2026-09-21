const d = require('docx');
const fs = require('fs');
const L = require('./manual_lib.js');

// เติมเลขหน้าจริงที่อ่านได้จาก PDF ของรอบแรก
// ต้องทำ "ก่อน" require ไฟล์เนื้อหา เพราะบรรทัดสารบัญถูกสร้างตอนโหลดโมดูล
// ถ้าตั้งทีหลัง toc() จะถูกเรียกไปแล้วด้วยตารางเปล่า แล้วตกไปใช้เลขหน้าสำรองที่เขียนไว้ในโค้ด
const mapFile = process.argv[3];
if (mapFile && fs.existsSync(mapFile)) {
  L.setPageMap(JSON.parse(fs.readFileSync(mapFile, 'utf8')));
  console.log('ใช้เลขหน้าจริงจาก', mapFile);
}

const front = require('./part_front.js');
const c12 = require('./part_ch12.js');
const c34 = require('./part_ch34.js');
const c56 = require('./part_ch56.js');
const cCal = require('./part_calib.js');
const ap = require('./part_appendix.js');
const apEq = require('./part_equations.js');

const { Document, Packer, Paragraph, TextRun, Header, Footer, PageNumber,
        AlignmentType, NumberFormat, convertInchesToTwip, LevelFormat,
        TabStopType, LeaderType } = d;
const CONTENT_DXA = L.CONTENT_DXA;

// สไตล์ของบรรทัดสารบัญ — สำคัญมาก
// เมื่อผู้ใช้สั่งอัปเดตสารบัญ Word จะจัดรูปแบบบรรทัดใหม่ด้วยสไตล์ชื่อ TOC 1..TOC 9
// ถ้าไม่กำหนดไว้ Word จะใช้ค่าปริยายของตัวเอง (Calibri ๑๑ พอยต์) ซึ่งผิดรูปแบบราชการทันที
function tocStyle(id, name, indentInch, bold) {
  return {
    id, name, basedOn: 'Normal', next: 'Normal', quickFormat: false,
    run: { font: FONT, size: 32, bold },
    paragraph: {
      spacing: { before: bold ? 120 : 0, after: 40, line: 300 },
      indent: { left: convertInchesToTwip(indentInch) },
      tabStops: [{ type: TabStopType.RIGHT, position: CONTENT_DXA, leader: LeaderType.DOT }],
    },
  };
}
const FONT = L.FONT;

// ---- ขอบกระดาษตามรูปแบบเอกสารราชการ: บน/ซ้าย ๑.๕ นิ้ว  ล่าง/ขวา ๑ นิ้ว ----
const PAGE = {
  size: { width: 11906, height: 16838 },                  // A4
  margin: {
    top: convertInchesToTwip(1.5), left: convertInchesToTwip(1.5),
    bottom: convertInchesToTwip(1), right: convertInchesToTwip(1),
    header: convertInchesToTwip(0.8), footer: convertInchesToTwip(0.6),
  },
};

function pageNumHeader(format) {
  return new Header({
    children: [new Paragraph({
      alignment: AlignmentType.RIGHT,
      children: [new TextRun({ font: FONT, size: 32, children: [PageNumber.CURRENT] })],
    })],
  });
}

const doc = new Document({
  creator: 'กิตติพันธ์ รัตนคร',
  title: 'คู่มือการปฏิบัติงาน การใช้งานระบบเฝ้าระวังการให้สารน้ำทางหลอดเลือดดำ Smart IV Alert',
  description: 'คู่มือสำหรับพยาบาลวิชาชีพ หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่',
  features: { updateFields: true },      // ให้ Word อัปเดตสารบัญเองตอนเปิดไฟล์
  styles: {
    default: {
      document: { run: { font: FONT, size: 32 }, paragraph: { spacing: { line: 276 } } },
      heading1: { run: { font: FONT, size: 40, bold: true, color: '000000' } },
      heading2: { run: { font: FONT, size: 36, bold: true, color: '000000' } },
      heading3: { run: { font: FONT, size: 32, bold: true, color: '000000' } },
    },
    paragraphStyles: [
      // สไตล์ของคำบรรยาย ใช้เป็น "ที่หมาย" ให้ฟิลด์สารบัญตาราง/สารบัญภาพเก็บรายการเอง
      { id: 'CaptionTable', name: 'Caption Table', basedOn: 'Normal', next: 'Normal',
        run: { font: FONT, size: 30, bold: true },
        paragraph: { alignment: AlignmentType.CENTER, spacing: { before: 120, after: 100, line: 288 } } },
      { id: 'CaptionFigure', name: 'Caption Figure', basedOn: 'Normal', next: 'Normal',
        run: { font: FONT, size: 28 },
        paragraph: { alignment: AlignmentType.CENTER, spacing: { before: 60, after: 160, line: 288 } } },
      tocStyle('TOC1', 'toc 1', 0,    true),    // ชื่อบท / ภาคผนวก
      tocStyle('TOC2', 'toc 2', 0.35, false),   // หัวข้อย่อย
      tocStyle('TOC3', 'toc 3', 0,    false),   // รายการตารางและภาพ
    ],
  },
  sections: [
    // ---- ส่วนที่ ๑ ปกหน้า ไม่มีเลขหน้า ----
    { properties: { page: PAGE, titlePage: false }, children: front.cover },

    // ---- ส่วนที่ ๒ ส่วนนำ ใช้เลขหน้าเป็นพยัญชนะไทย ก ข ค ----
    {
      properties: {
        page: { ...PAGE, pageNumbers: { start: 1, formatType: NumberFormat.THAI_LETTERS } },
      },
      headers: { default: pageNumHeader() },
      children: [
        ...front.preface.slice(1),          // ตัด PageBreak ตัวแรกออก เพราะขึ้นหน้าใหม่ด้วย section อยู่แล้ว
        ...front.contentsEntries,
        ...front.tableEntries,
        ...front.figureEntries,
      ],
    },

    // ---- ส่วนที่ ๓ เนื้อหา ใช้เลขหน้าอารบิกเริ่มที่ ๑ ----
    {
      properties: {
        page: { ...PAGE, pageNumbers: { start: 1, formatType: NumberFormat.DECIMAL } },
      },
      headers: { default: pageNumHeader() },
      children: [
        ...c12.ch1.slice(1),                // ตัด PageBreak ตัวแรกของบทที่ ๑
        ...c12.ch2,
        ...c34.ch3,
        ...cCal.chCalib,      // บทที่ ๔ การติดตั้งเซนเซอร์และการคาลิเบรต
        ...c34.ch4,
        ...c56.ch5,
        ...c56.ch6,
        ...ap.divider,
        ...ap.apA, ...ap.apB, ...ap.apC, ...ap.apD, ...ap.apE, ...apEq.apF,
        ...ap.refs,
      ],
    },
  ],
});

Packer.toBuffer(doc).then((buf) => {
  const out = process.argv[2] || 'manual.docx';
  fs.writeFileSync(out, buf);
  console.log('เขียนแล้ว:', out, (buf.length / 1024).toFixed(0) + ' KB');
});
