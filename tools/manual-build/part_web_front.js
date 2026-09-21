// ส่วนนำของคู่มือหน้าเว็บ: ปก คำนำ สารบัญ
const L = require('./manual_lib.js');
const { d, run, p, plain, center, table, tableCaption, toc, tocFieldBegin, tocFieldEnd, noteBox } = L;
const { Paragraph, PageBreak, AlignmentType } = d;

const cover = [
  plain('', { after: 1400 }),
  center('คู่มือการปฏิบัติงาน', { bold: true, size: 44, after: 120 }),
  center('การใช้งานหน้าเว็บเฝ้าระวังการให้สารน้ำ', { bold: true, size: 52, after: 120 }),
  center('ระบบ Smart IV Alert', { bold: true, size: 52, after: 500 }),
  center('สำหรับพยาบาลวิชาชีพ', { size: 36, after: 160 }),
  center('หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น อำเภอสูงเม่น จังหวัดแพร่', { size: 32, after: 1400 }),
  center('วิทยาลัยพยาบาลบรมราชชนนี แพร่', { bold: true, size: 34, after: 100 }),
  center('คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก', { size: 32, after: 100 }),
  center('พุทธศักราช ๒๕๖๙', { size: 32 }),
];

const preface = [
  new Paragraph({ children: [new PageBreak()] }),
  center('คำนำ', { bold: true, size: 40, after: 300 }),
  p('คู่มือฉบับนี้จัดทำขึ้นเพื่ออธิบาย “หน้าเว็บเฝ้าระวัง” ของระบบ Smart IV Alert เพียงส่วนเดียว ' +
    'โดยอธิบายลงลึกทุกหน้า ทุกปุ่ม และทุกบรรทัดที่ปรากฏบนจอ เพื่อให้พยาบาลวิชาชีพผู้ปฏิบัติงาน ' +
    'สามารถอ่านค่าบนหน้าจอได้อย่างถูกต้อง ตัดสินใจได้ทันเหตุการณ์ และไม่ตีความข้อมูลผิดพลาด'),
  p('การใช้งานเครื่องประจำเตียงและเครื่องศูนย์กลางโดยตรง ตลอดจนการติดตั้งเซนเซอร์และการคาลิเบรต ' +
    'อยู่ในคู่มือหลัก เรื่อง “การใช้งานระบบเฝ้าระวังการให้สารน้ำทางหลอดเลือดดำ Smart IV Alert” ' +
    'ซึ่งเป็นคนละเล่มกับฉบับนี้'),
  p('เนื้อหาทุกหัวข้อในคู่มือนี้จัดทำขึ้นจากการอ่านโปรแกรมของหน้าเว็บจริง จึงตรงกับสิ่งที่ปรากฏ ' +
    'บนหน้าจอทุกประการ ภาพประกอบทั้งหมดเป็นภาพถ่ายจากหน้าเว็บจริงที่ป้อนข้อมูลจำลองเข้าไป ' +
    'มิได้วาดขึ้นใหม่'),
  plain('', { after: 200 }),
  noteBox('ข้อพึงระลึกสำคัญที่สุด', [
    'ระบบนี้เป็นเครื่องช่วยเฝ้าระวัง ไม่ใช่เครื่องควบคุมการให้สารน้ำ',
    'ระบบไม่สามารถปรับอัตราการไหลเองได้ การปรับยังต้องทำที่โรลเลอร์แคลมป์ด้วยมือของพยาบาลเสมอ',
    'การที่หน้าจอไม่แจ้งเตือน ไม่ได้แปลว่าผู้ป่วยปลอดภัย การตรวจเยี่ยมผู้ป่วยตามมาตรฐานวิชาชีพยังต้องทำตามปกติทุกประการ',
  ], 'FCE4E4', 'C00000'),
  plain('', { after: 300 }),
  center('คณะผู้จัดทำ', { bold: true, after: 60 }),
  center('วิทยาลัยพยาบาลบรมราชชนนี แพร่', {}),
];

const contentsEntries = [
  new Paragraph({ children: [new PageBreak()] }),
  center('สารบัญ', { bold: true, size: 40, after: 240 }),
  tocFieldBegin('TOC \\o "1-2" \\h \\z \\u'),
  toc('บทที่ ๑  ภาพรวมและการเข้าใช้งานหน้าเว็บ', 1, { bold: true }),
  toc('บทที่ ๒  แถบหัวเรื่องและแถบเมนูหลัก', 5, { bold: true }),
  toc('บทที่ ๓  หน้าที่ ๑ Live Monitor', 9, { bold: true }),
  toc('บทที่ ๔  หน้าที่ ๒ Visual Graphs', 22, { bold: true }),
  toc('บทที่ ๕  หน้าที่ ๓ Log & Shift Report', 26, { bold: true }),
  toc('บทที่ ๖  หน้าที่ ๔ เกี่ยวกับงานวิจัย', 31, { bold: true }),
  toc('บทที่ ๗  สถานการณ์จริงและการแก้ปัญหา', 33, { bold: true }),
  toc('ภาคผนวก', 39, { bold: true }),
  tocFieldEnd(),
];

const tableEntries = [
  new Paragraph({ children: [new PageBreak()] }),
  center('สารบัญตาราง', { bold: true, size: 40, after: 240 }),
  tocFieldBegin('TOC \\t "CaptionTable,3" \\h \\z'),
  toc('ตารางที่ ๑  ความหมายของทุกบรรทัดบนการ์ดเตียง', 14, { level: 1 }),
  tocFieldEnd(),
];

const figureEntries = [
  new Paragraph({ children: [new PageBreak()] }),
  center('สารบัญภาพ', { bold: true, size: 40, after: 240 }),
  tocFieldBegin('TOC \\t "CaptionFigure,3" \\h \\z'),
  toc('ภาพที่ ๑  แถบหัวเรื่องและแถบเมนูหลัก', 5, { level: 1 }),
  tocFieldEnd(),
];

module.exports = { cover, preface, contentsEntries, tableEntries, figureEntries };
