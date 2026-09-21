# -*- coding: utf-8 -*-
"""ตัวช่วยสร้างไฟล์ schematic ของ KiCad 7 (.kicad_sch) แบบ S-expression

ไฟล์ที่สร้างมี "นิยามสัญลักษณ์ฝังอยู่ในตัว" (lib_symbols) ทั้งหมด จึงเปิดได้ทันที
บนเครื่องที่ไม่ได้ติดตั้งไลบรารีมาตรฐานของ KiCad

ระบบพิกัด: ในนิยามสัญลักษณ์แกน Y ชี้ขึ้น แต่ในหน้ากระดาษแกน Y ชี้ลง
เวลาวางสัญลักษณ์ที่ (X, Y) จุด (lx, ly) ในนิยามจึงไปอยู่ที่ (X + lx, Y - ly)
"""
import uuid as _uuid

VERSION = 20230121
GRID = 1.27


def uid():
    return str(_uuid.uuid4())


def q(s):
    return '"' + str(s).replace('\\', '\\\\').replace('"', '\\"') + '"'


def eff(size=1.27, hide=False, justify=None, bold=False):
    out = '(effects (font (size %s %s)%s)' % (size, size, ' bold' if bold else '')
    if justify:
        out += ' (justify %s)' % justify
    out += ' hide)' if hide else ')'
    return out


# --------------------------------------------------------------- นิยามสัญลักษณ์
class Sym:
    """สัญลักษณ์รูปสี่เหลี่ยมพร้อมขาซ้าย/ขวา ใช้แทนโมดูลสำเร็จรูปทุกตัวในผังนี้"""

    def __init__(self, lib_id, ref_prefix, width, left, right,
                 pin_len=2.54, show_pin_numbers=False):
        self.lib_id = lib_id
        self.ref_prefix = ref_prefix
        self.w = width
        self.pin_len = pin_len
        self.show_numbers = show_pin_numbers
        # ขา: (ชื่อ, เลขขา, ชนิดทางไฟฟ้า)
        self.left = left
        self.right = right
        n = max(len(left), len(right))
        self.h = max(n * 2.54 + 2.54, 7.62)

    @property
    def name(self):
        return self.lib_id.split(':')[1]

    def _rows(self, pins):
        """ตำแหน่ง y ของแต่ละขา จัดให้สมมาตรรอบจุดกึ่งกลางและตกบนกริด 2.54"""
        n = len(pins)
        top = (n - 1) * 2.54 / 2.0
        return [top - i * 2.54 for i in range(n)]

    def pin_xy(self, number):
        """พิกัดปลายขา (จุดที่ใช้ต่อสาย) ในระบบพิกัดของนิยามสัญลักษณ์"""
        hw = self.w / 2.0
        for pins, sign in ((self.left, -1), (self.right, 1)):
            for y, (_, num, _) in zip(self._rows(pins), pins):
                if str(num) == str(number):
                    return (sign * (hw + self.pin_len), y)
        raise KeyError('%s: ไม่มีขาหมายเลข %s' % (self.lib_id, number))

    def defn(self):
        hw, hh = self.w / 2.0, self.h / 2.0
        s = ['  (symbol %s (pin_names (offset 0.508)) (in_bom yes) (on_board yes)' % q(self.lib_id)]
        if not self.show_numbers:
            s[0] = s[0].replace('(pin_names', '(pin_numbers hide) (pin_names')
        s.append('    (property "Reference" %s (at 0 %s 0) %s)'
                 % (q(self.ref_prefix), round(hh + 2.54, 3), eff(justify='left')))
        s.append('    (property "Value" %s (at 0 %s 0) %s)'
                 % (q(self.name), round(-hh - 2.54, 3), eff(justify='left')))
        s.append('    (property "Footprint" "" (at 0 0 0) %s)' % eff(hide=True))
        s.append('    (property "Datasheet" "" (at 0 0 0) %s)' % eff(hide=True))
        s.append('    (symbol %s' % q(self.name + '_0_1'))
        s.append('      (rectangle (start %s %s) (end %s %s) '
                 '(stroke (width 0.254) (type default)) (fill (type background)))'
                 % (-hw, hh, hw, -hh))
        s.append('    )')
        s.append('    (symbol %s' % q(self.name + '_1_1'))
        for pins, sign, rot in ((self.left, -1, 0), (self.right, 1, 180)):
            for y, (nm, num, etype) in zip(self._rows(pins), pins):
                s.append('      (pin %s line (at %s %s %s) (length %s)'
                         % (etype, sign * (hw + self.pin_len), y, rot, self.pin_len))
                s.append('        (name %s %s) (number %s %s))'
                         % (q(nm), eff(1.016), q(num), eff(1.016)))
        s.append('    )')
        s.append('  )')
        return '\n'.join(s)


def power_sym(lib_id, label, down=False):
    """สัญลักษณ์ไฟเลี้ยง/กราวด์ — ขาเดียว ชนิด power_in"""
    name = lib_id.split(':')[1]
    rot = 270 if down else 90
    s = ['  (symbol %s (power) (pin_numbers hide) (pin_names (offset 0) hide) '
         '(in_bom no) (on_board no)' % q(lib_id)]
    s.append('    (property "Reference" "#PWR" (at 0 -2.54 0) %s)' % eff(hide=True))
    s.append('    (property "Value" %s (at 0 %s 0) %s)'
             % (q(label), 3.556 if not down else -3.81, eff()))
    s.append('    (property "Footprint" "" (at 0 0 0) %s)' % eff(hide=True))
    s.append('    (property "Datasheet" "" (at 0 0 0) %s)' % eff(hide=True))
    s.append('    (symbol %s' % q(name + '_0_1'))
    if down:   # สัญลักษณ์กราวด์
        s.append('      (polyline (pts (xy 0 0) (xy 0 -1.27) (xy 1.27 -1.27) (xy 0 -2.54) '
                 '(xy -1.27 -1.27) (xy 0 -1.27)) '
                 '(stroke (width 0) (type default)) (fill (type none)))')
    else:      # สัญลักษณ์ไฟเลี้ยง
        s.append('      (polyline (pts (xy -0.762 1.27) (xy 0 2.54) (xy 0.762 1.27)) '
                 '(stroke (width 0) (type default)) (fill (type none)))')
        s.append('      (polyline (pts (xy 0 0) (xy 0 2.54)) '
                 '(stroke (width 0) (type default)) (fill (type none)))')
    s.append('    )')
    s.append('    (symbol %s' % q(name + '_1_1'))
    s.append('      (pin power_in line (at 0 0 %s) (length 0) (name %s %s) (number "1" %s))'
             % (rot, q(label), eff(hide=True), eff(hide=True)))
    s.append('    )')
    s.append('  )')
    return '\n'.join(s)


def res_sym():
    s = ['  (symbol "Device:R" (pin_numbers hide) (pin_names (offset 0)) (in_bom yes) (on_board yes)']
    s.append('    (property "Reference" "R" (at 2.032 0 90) %s)' % eff())
    s.append('    (property "Value" "R" (at 0 0 90) %s)' % eff())
    s.append('    (property "Footprint" "" (at -1.778 0 90) %s)' % eff(hide=True))
    s.append('    (property "Datasheet" "" (at 0 0 0) %s)' % eff(hide=True))
    s.append('    (symbol "R_0_1"')
    s.append('      (rectangle (start -1.016 -2.54) (end 1.016 2.54) '
             '(stroke (width 0.254) (type default)) (fill (type none)))')
    s.append('    )')
    s.append('    (symbol "R_1_1"')
    s.append('      (pin passive line (at 0 3.81 270) (length 1.27) (name "~" %s) (number "1" %s))'
             % (eff(hide=True), eff()))
    s.append('      (pin passive line (at 0 -3.81 90) (length 1.27) (name "~" %s) (number "2" %s))'
             % (eff(hide=True), eff()))
    s.append('    )')
    s.append('  )')
    return '\n'.join(s)


def sw_sym():
    """ปุ่มกดสัมผัส 2 ขา — เฟิร์มแวร์ใช้ INPUT_PULLUP จึงต่อลงกราวด์ได้เลย"""
    s = ['  (symbol "Switch:SW_Push" (pin_numbers hide) (pin_names (offset 0.254) hide) '
         '(in_bom yes) (on_board yes)']
    s.append('    (property "Reference" "SW" (at 1.27 3.81 0) %s)' % eff())
    s.append('    (property "Value" "SW_Push" (at 0 -2.54 0) %s)' % eff())
    s.append('    (property "Footprint" "" (at 0 5.08 0) %s)' % eff(hide=True))
    s.append('    (property "Datasheet" "" (at 0 0 0) %s)' % eff(hide=True))
    s.append('    (symbol "SW_Push_0_1"')
    s.append('      (circle (center -2.032 0) (radius 0.508) '
             '(stroke (width 0) (type default)) (fill (type none)))')
    s.append('      (circle (center 2.032 0) (radius 0.508) '
             '(stroke (width 0) (type default)) (fill (type none)))')
    s.append('      (polyline (pts (xy -1.524 1.016) (xy 1.524 1.778)) '
             '(stroke (width 0) (type default)) (fill (type none)))')
    s.append('      (polyline (pts (xy 0 1.524) (xy 0 3.048)) '
             '(stroke (width 0) (type default)) (fill (type none)))')
    s.append('    )')
    s.append('    (symbol "SW_Push_1_1"')
    s.append('      (pin passive line (at -5.08 0 0) (length 2.54) (name "1" %s) (number "1" %s))'
             % (eff(hide=True), eff()))
    s.append('      (pin passive line (at 5.08 0 180) (length 2.54) (name "2" %s) (number "2" %s))'
             % (eff(hide=True), eff()))
    s.append('    )')
    s.append('  )')
    return '\n'.join(s)


# ------------------------------------------------------------------ หน้ากระดาษ
class Sheet:
    def __init__(self, title, rev, company, paper='A3'):
        self.uuid = uid()
        self.title, self.rev, self.company, self.paper = title, rev, company, paper
        self.defs, self.items, self.refs = [], [], {}

    def add_def(self, text):
        self.defs.append(text)

    def place(self, sym, ref, value, x, y, footnote=None):
        """วางสัญลักษณ์ คืนฟังก์ชันแปลงเลขขา -> พิกัดบนหน้ากระดาษ"""
        u = uid()
        hh = sym.h / 2.0
        s = ['  (symbol (lib_id %s) (at %s %s 0) (unit 1) '
             '(in_bom yes) (on_board yes) (dnp no) (uuid %s)' % (q(sym.lib_id), x, y, q(u))]
        s.append('    (property "Reference" %s (at %s %s 0) %s)'
                 % (q(ref), x, round(y - hh - 2.54, 3), eff(justify='left', bold=True)))
        s.append('    (property "Value" %s (at %s %s 0) %s)'
                 % (q(value), x, round(y + hh + 2.54, 3), eff(justify='left')))
        s.append('    (property "Footprint" "" (at %s %s 0) %s)' % (x, y, eff(hide=True)))
        s.append('    (property "Datasheet" "" (at %s %s 0) %s)' % (x, y, eff(hide=True)))
        if footnote:
            s.append('    (property "Note" %s (at %s %s 0) %s)'
                     % (q(footnote), x, y, eff(hide=True)))
        for pins in (sym.left, sym.right):
            for _, num, _ in pins:
                s.append('    (pin %s (uuid %s))' % (q(num), q(uid())))
        s.append('    (instances (project "smart-iv-alert" (path %s (reference %s) (unit 1))))'
                 % (q('/' + self.uuid), q(ref)))
        s.append('  )')
        self.items.append('\n'.join(s))
        self.refs[ref] = sym

        def at(number):
            lx, ly = sym.pin_xy(number)
            return (round(x + lx, 3), round(y - ly, 3))
        return at

    def place_simple(self, lib_id, ref, value, x, y, rot=0, npins=2, vdy=None):
        u = uid()
        s = ['  (symbol (lib_id %s) (at %s %s %s) (unit 1) '
             '(in_bom yes) (on_board yes) (dnp no) (uuid %s)' % (q(lib_id), x, y, rot, q(u))]
        hidden = lib_id.startswith('power:')
        s.append('    (property "Reference" %s (at %s %s 0) %s)'
                 % (q(ref), x, y - 5.08, eff(hide=hidden)))
        if vdy is not None:
            s.append('    (property "Value" %s (at %s %s 0) %s)'
                     % (q(value), x, round(y + vdy, 3), eff()))
        else:
            s.append('    (property "Value" %s (at %s %s 0) %s)'
                     % (q(value), x + (0 if hidden else 3.81), y, eff()))
        s.append('    (property "Footprint" "" (at %s %s 0) %s)' % (x, y, eff(hide=True)))
        s.append('    (property "Datasheet" "" (at %s %s 0) %s)' % (x, y, eff(hide=True)))
        for i in range(1, npins + 1):
            s.append('    (pin "%d" (uuid %s))' % (i, q(uid())))
        s.append('    (instances (project "smart-iv-alert" (path %s (reference %s) (unit 1))))'
                 % (q('/' + self.uuid), q(ref)))
        s.append('  )')
        self.items.append('\n'.join(s))

    def wire(self, a, b):
        self.items.append('  (wire (pts (xy %s %s) (xy %s %s)) '
                          '(stroke (width 0) (type default)) (uuid %s))'
                          % (a[0], a[1], b[0], b[1], q(uid())))

    def label(self, text, at, rot=0, justify='left'):
        self.items.append('  (label %s (at %s %s %s) %s (uuid %s))'
                          % (q(text), at[0], at[1], rot,
                             eff(justify=justify + ' bottom'), q(uid())))

    def text(self, body, x, y, size=1.27, bold=False):
        self.items.append('  (text %s (at %s %s 0) %s (uuid %s))'
                          % (q(body), x, y,
                             eff(size, justify='left', bold=bold), q(uid())))

    def box(self, x1, y1, x2, y2):
        self.items.append('  (rectangle (start %s %s) (end %s %s) '
                          '(stroke (width 0.1) (type dash)) (fill (type none)) (uuid %s))'
                          % (x1, y1, x2, y2, q(uid())))

    def render(self, date):
        out = ['(kicad_sch (version %d) (generator "smart_iv_alert_gen")' % VERSION]
        out.append('  (uuid %s)' % q(self.uuid))
        out.append('  (paper %s)' % q(self.paper))
        out.append('  (title_block')
        out.append('    (title %s)' % q(self.title))
        out.append('    (date %s)' % q(date))
        out.append('    (rev %s)' % q(self.rev))
        out.append('    (company %s)' % q(self.company))
        out.append('  )')
        out.append('  (lib_symbols')
        out.extend(self.defs)
        out.append('  )')
        out.extend(self.items)
        out.append('  (sheet_instances (path "/" (page "1")))')
        out.append(')')
        return '\n'.join(out) + '\n'
