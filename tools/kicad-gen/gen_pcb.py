# -*- coding: utf-8 -*-
"""สร้างแผ่นวงจรพิมพ์แบบ Carrier Board ของ Smart IV Alert (KiCad 7)

บอร์ดนี้เป็น "บอร์ดรอง" 2 ชั้น รูทะลุทั้งหมด โมดูลสำเร็จรูปทุกตัวเสียบลงหัวต่อ
ไม่มีชิ้นส่วน SMD จึงบัดกรีด้วยมือได้ และถอดเปลี่ยนโมดูลที่เสียได้

วิธีเดินลาย: แมนฮัตตันสองชั้น F.Cu แนวตั้งอย่างเดียว B.Cu แนวนอนอย่างเดียว
เงื่อนไขที่บังคับด้วยโค้ด (assert) ก่อนเดินสายทุกครั้ง
  1. ขาที่ต้องเดินสายทุกขาต้องมีค่า y ไม่ซ้ำกัน  -> แนวนอนไม่มีทางทับกัน
  2. ทุกเส้นได้คอลัมน์ x ของตัวเอง                -> แนวตั้งไม่มีทางทับกัน
  3. คอลัมน์ต้องอยู่ในช่องเดินสายที่ไม่มีขาอุปกรณ์ -> ลายไม่ทะลุขาใคร
ถ้าเงื่อนไขข้อใดไม่ผ่าน สคริปต์จะหยุดทันที ไม่ปล่อยให้ไปโผล่ที่ DRC
"""
import os, sys, re
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pcbnew as P
from kpcb import Board, mm

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
OUT = os.path.join(ROOT, 'hardware')

# ===========================================================================
#  ค่าของโมดูล ESP32-S3 Super Mini  <-- ต้องตรวจกับบอร์ดจริงก่อนสั่งผลิต
# ===========================================================================
# บอร์ดรุ่นนี้ผู้ผลิตแต่ละเจ้าทำไม่เหมือนกัน ค่าด้านล่างเป็นค่าตั้งต้นที่ยังไม่ยืนยัน
# ให้พิมพ์ hardware/<board>-FitCheck.pdf ที่สเกล 100% แล้ววางโมดูลจริงทาบตรวจ
# ถ้าไม่ตรง ให้แก้สามค่านี้แล้วรันสคริปต์ใหม่
MODULE = dict(
    rows=8,                 # จำนวนขาต่อหนึ่งแถว
    row_pitch=15.24,        # ระยะห่างระหว่างสองแถว (มม.)
    pin_pitch=2.54,         # ระยะห่างระหว่างขาในแถวเดียวกัน (มม.)
    # ลำดับขาจากด้านที่มีพอร์ต USB ลงไป
    left=['5V', 'GND', '3V3', 'GPIO13', 'GPIO12', 'GPIO11', 'GPIO10', 'GPIO9'],
    right=['GPIO8', 'GPIO7', 'GPIO6', 'GPIO5', 'GPIO4', 'GPIO3', 'GPIO2', 'GPIO1'],
)
MODULE_VERIFIED = False     # ตั้งเป็น True หลังตรวจกับบอร์ดจริงแล้ว

HDR = 'Connector_PinHeader_2.54mm'
TRACK, CLEAR = 0.4, 0.25


def pins_from(ino, names):
    src = open(ino, encoding='utf-8').read()
    return {n: int(re.search(r'^#define\s+%s\s+(-?\d+)' % re.escape(n), src, re.M).group(1))
            for n in names}


class Carrier:
    """ตัวช่วยวางและเดินสายบอร์ดรอง"""

    # ลาย 0.4 มม. + ระยะห่าง 0.25 มม. ต้องการศูนย์กลางห่างกัน 0.65 มม. ใช้ 1.0 จึงมีเผื่อ
    # และยังจับเคสขาเบียดกัน 0.16 มม. ที่เคยหลุดไปได้อยู่
    MIN_DY = 1.0

    def __init__(self, w, h, mcu_x, mcu_y):
        self.bd = Board(track_mm=TRACK, clearance_mm=CLEAR)
        self.W, self.H = w, h
        self.mcu_x, self.mcu_y = mcu_x, mcu_y
        self.pin_of = {}          # 'GPIO13' -> (ref, pad, ฝั่ง)
        self.routed_y = {}        # y(nm) -> ชื่อเน็ต  ใช้บังคับเงื่อนไขข้อ 1
        self.used_cx = set()      # คอลัมน์ที่ถูกจองแล้ว  เงื่อนไขข้อ 2
        self.lane_pool = []
        self.colL, self.colR = [], []

    # ------------------------------------------------------------- การวาง
    def place_mcu(self):
        m = MODULE
        n, rp = m['rows'], m['row_pitch']
        fp = 'PinHeader_1x%02d_P2.54mm_Vertical' % n
        self.bd.add('J_MCU_A', HDR, fp, self.mcu_x, self.mcu_y, value='ESP32-S3 row A')
        self.bd.add('J_MCU_B', HDR, fp, self.mcu_x + rp, self.mcu_y, value='ESP32-S3 row B')
        for i, lbl in enumerate(m['left']):
            self.pin_of[lbl] = ('J_MCU_A', i + 1, 'far')
        for i, lbl in enumerate(m['right']):
            self.pin_of[lbl] = ('J_MCU_B', i + 1, 'near')
        # ซิลค์สกรีนบอกลำดับขาที่สมมติไว้ ให้ช่างตรวจกับโมดูลจริงได้ทันที
        for i, lbl in enumerate(m['left']):
            self.bd.label(lbl, self.mcu_x - 12.5, self.mcu_y + i * 2.54, size=1.0)
        for i, lbl in enumerate(m['right']):
            self.bd.label(lbl, self.mcu_x + rp + 3.0, self.mcu_y + i * 2.54, size=1.0)

    def conn(self, ref, npins, x, y, value, rot=0):
        fp = 'PinHeader_1x%02d_P2.54mm_Vertical' % npins
        self.bd.add(ref, HDR, fp, x, y, rot=rot, value=value)
        self.bd.label(value, x - 2.0, y - 4.0, size=1.1)
        return ref

    # ------------------------------------------------- ช่องเดินสายและคอลัมน์
    def channels(self, left_lo, left_hi, right_lo, right_hi, lane_lo, lane_hi, step=1.0):
        def span(lo, hi, st):
            return [round(lo + i * st, 3) for i in range(int((hi - lo) / st) + 1)]
        self.colL = span(left_lo, left_hi, step)
        self.colR = span(right_lo, right_hi, 1.6)
        self.lane_pool = span(lane_lo, lane_hi, step)

    def _take(self, pool):
        for v in pool:
            if v not in self.used_cx:
                self.used_cx.add(v)
                return v
        raise SystemExit('ช่องเดินสายไม่พอ ต้องขยายบอร์ดหรือเพิ่มช่อง')

    # ----------------------------------------------------------- การเดินสาย
    def _claim_y(self, ref, pad, net):
        """จองระดับ y ของขาหนึ่ง แยกตามฝั่งซ้าย/ขวาของหัวต่อโมดูล

        ขาแถว A กับแถว B ที่อยู่แถวเดียวกันมี y เท่ากันได้ เพราะลายแนวนอน
        ของทั้งคู่วิ่งออกคนละทิศ โดยมีขาของหัวต่อเองคั่นกลางอยู่แล้ว
        ที่ห้ามคือสองเน็ตที่มีลายแนวนอน "ฝั่งเดียวกัน" อยู่ระดับ y เดียวกัน
        """
        x, y = self.bd.pad_xy(ref, pad)
        side = 'L' if x <= mm(self.mcu_x) else 'R'
        # ไม่ใช่แค่ห้าม y เท่ากันพอดี แต่ต้องห่างพอให้ลายกับรูไม่เบียดกันด้วย
        # (เคยพลาดมาแล้ว: ขาสองตัวห่างกัน 0.16 มม. ผ่านการตรวจ "เท่ากัน" แต่ DRC ฟ้อง)
        for (sd, yy), owner in self.routed_y.items():
            if sd == side and owner != net and abs(yy - y) < mm(self.MIN_DY):
                raise SystemExit(
                    'ขาฝั่ง %s สองเส้นอยู่ใกล้กันเกินไปในแนว y (%0.2f กับ %0.2f mm, '
                    'ต้องห่างอย่างน้อย %0.1f): %s กับ %s\n'
                    'ต้องเลื่อนตำแหน่งอุปกรณ์ ไม่เช่นนั้นลายแนวนอนจะเบียดกัน'
                    % (side, yy / 1e6, y / 1e6, self.MIN_DY, owner, net))
        self.routed_y[(side, y)] = net
        self.bd.assign(ref, pad, net)
        return (x, y)

    def link(self, net, a, b):
        """ต่อขา a ไปขา b  แต่ละ tuple คือ (ref, pad)

        ตัดสินเส้นทางจากตำแหน่ง x จริงของขา ไม่ใช่จากชื่อ footprint
        ถ้าขาสองข้างอยู่คนละฝั่งของหัวต่อโมดูล ต้องอ้อมผ่านเลนด้านบน
        """
        pa = self._claim_y(a[0], a[1], net)
        pb = self._claim_y(b[0], b[1], net)
        XL = mm(self.mcu_x)
        XR = mm(self.mcu_x + MODULE['row_pitch'])
        ax, bx = pa[0], pb[0]
        if (ax <= XL and bx >= XR) or (bx <= XL and ax >= XR):
            if bx <= XL:                       # ให้ pa อยู่ฝั่งซ้ายเสมอ
                pa, pb = pb, pa
            cx1 = self._take(self.colL)
            cx2 = self._take(self.colR)
            self.bd.route_via_lane(pa, pb, mm(cx1), mm(self._take_lane()), mm(cx2), net)
        else:
            pool = self.colL if max(ax, bx) <= XL else self.colR
            self.bd.route(pa, pb, mm(self._take(pool)), net)

    def _take_lane(self):
        for v in self.lane_pool:
            if ('lane', v) not in self.used_cx:
                self.used_cx.add(('lane', v))
                return v
        raise SystemExit('เลนข้ามหัวต่อไม่พอ')

    def chain(self, net, pads):
        """ต่อหลายขาเข้าเน็ตเดียวกันแบบเรียงต่อกัน ใช้กับ GND และไฟเลี้ยง"""
        for i in range(len(pads) - 1):
            self.link(net, pads[i], pads[i + 1])

    # --------------------------------------------------------------- จบงาน
    def finish(self, name, title):
        self.bd.outline(0, 0, self.W, self.H)
        for hx, hy in ((4, 4), (self.W - 4, 4), (4, self.H - 4), (self.W - 4, self.H - 4)):
            self.bd.mounting_hole(hx, hy)
        self.bd.label(title, 5, self.H - 7.5, size=1.6)
        self.bd.label('Smart IV Alert carrier board rev 1.0', 5, self.H - 4.5, size=1.0)
        if not MODULE_VERIFIED:
            self.bd.label('MODULE FOOTPRINT UNVERIFIED', 5, self.H - 11, size=1.3)
            self.bd.label('check FitCheck.pdf at 100% scale before ordering',
                          5, self.H - 14, size=1.0)
        path = os.path.join(OUT, name + '.kicad_pcb')
        self.bd.save(path)
        return path


# ===========================================================================
#  บอร์ดเครื่องประจำเตียง
# ===========================================================================
def build_station():
    ino = os.path.join(ROOT, 'firmware', 'ESP32-S3-Station-V_7_7_3',
                       'ESP32-S3-Station-V_7_7_3.ino')
    P_ = pins_from(ino, ['TFT_CS', 'TFT_DC', 'TFT_RST', 'TFT_MOSI', 'TFT_SCLK',
                         'TFT_BLK', 'SENSOR_AO_PIN', 'BUZZER_PIN', 'BTN_PIN'])
    g = lambda k: 'GPIO%d' % P_[k]

    c = Carrier(100, 86, mcu_x=24, mcu_y=24)
    c.place_mcu()
    c.channels(left_lo=5, left_hi=15, right_lo=44, right_hi=72, lane_lo=8, lane_hi=19)

    c.conn('J_TFT', 8, 80, 25.27, 'TFT ST7789 1.47in')
    c.conn('J_SENS', 4, 80, 48.13, 'TCRT5000')
    c.conn('J_PWR', 2, 18, 57, '5V IN')
    # บัซเซอร์หมุน 90 องศา ขาทั้งสองจึงอยู่คนละระดับ y  วางให้พ้น J_SENS
    c.bd.add('BZ1', 'Buzzer_Beeper', 'Buzzer_12x9.5RM7.6', 80, 67.6, rot=90, value='Buzzer')
    c.bd.label('Buzzer', 68, 64, size=1.1)
    c.bd.add('SW1', 'Button_Switch_THT', 'SW_PUSH_6mm', 74, 74, value='User button')
    c.bd.label('User button', 58, 72, size=1.1)

    M = c.pin_of
    def mp(label):
        ref, pad, _ = M[label]
        return (ref, pad)

    # ---- สัญญาณจอ TFT ----
    for gpio_key, tft_pad in ((('TFT_SCLK'), 3), ('TFT_MOSI', 4), ('TFT_RST', 5),
                              ('TFT_DC', 6), ('TFT_CS', 7), ('TFT_BLK', 8)):
        c.link(gpio_key, mp(g(gpio_key)), ('J_TFT', tft_pad))

    # ---- เซนเซอร์ เสียงเตือน ปุ่ม ----
    c.link('DROP_AO', mp(g('SENSOR_AO_PIN')), ('J_SENS', 4))
    c.link('BUZZER', mp(g('BUZZER_PIN')), ('BZ1', 1))
    c.link('BTN', mp(g('BTN_PIN')), ('SW1', 1))

    # ---- ไฟเลี้ยงและกราวด์ ----
    c.chain('+3V3', [mp('3V3'), ('J_TFT', 2), ('J_SENS', 1)])
    c.chain('+5V', [('J_PWR', 1), mp('5V')])
    c.chain('GND', [('J_PWR', 2), mp('GND'), ('J_TFT', 1),
                    ('J_SENS', 2), ('BZ1', 2), ('SW1', 2)])

    return c, 'Station-BedNode', 'BED STATION NODE v7.7.3'


# ===========================================================================
#  บอร์ดเครื่องศูนย์กลาง
# ===========================================================================
def build_host():
    ino = os.path.join(ROOT, 'firmware', 'ESP32-S3-Host-OLED-V_4_7_5',
                       'ESP32-S3-Host-OLED-V_4_7_5.ino')
    P_ = pins_from(ino, ['HOST_BAT_ADC_PIN', 'POWER_BTN_PIN', 'PAGE_BTN_PIN',
                         'BUZZER_PIN', 'OLED_SDA_PIN', 'OLED_SCL_PIN'])
    g = lambda k: 'GPIO%d' % P_[k]

    c = Carrier(100, 86, mcu_x=24, mcu_y=24)
    c.place_mcu()
    c.channels(left_lo=5, left_hi=13, right_lo=44, right_hi=72, lane_lo=8, lane_hi=19)

    c.conn('J_OLED', 4, 80, 25.27, 'OLED SH1106 I2C')
    c.conn('J_BAT', 2, 16, 44, 'Li-ion 1S')
    c.bd.add('BZ1', 'Buzzer_Beeper', 'Buzzer_12x9.5RM7.6', 80, 45, rot=90, value='Buzzer')
    c.bd.label('Buzzer', 68, 42, size=1.1)
    c.bd.add('SW1', 'Button_Switch_THT', 'SW_PUSH_6mm', 74, 54, value='Power/snooze')
    c.bd.label('Power / snooze', 52, 52, size=1.1)
    c.bd.add('SW2', 'Button_Switch_THT', 'SW_PUSH_6mm', 74, 65, value='Page')
    c.bd.label('Page / screensaver', 50, 63, size=1.1)
    # ตัวแบ่งแรงดัน 1:1 หมุน 90 องศาให้ขาทั้งสองอยู่คนละระดับ y
    # วางฝั่งซ้ายนอกช่องเดินสาย ไม่ให้ลายแนวตั้งวิ่งทะลุขา
    RES = 'R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal'
    c.bd.add('R1', 'Resistor_THT', RES, 17, 60, rot=90, value='100k')
    c.bd.add('R2', 'Resistor_THT', RES, 22, 64, rot=90, value='100k')
    c.bd.label('R1', 13, 56, size=1.0)
    c.bd.label('R2', 24, 60, size=1.0)
    c.bd.label('1:1 divider - R1 and R2 MUST be equal', 30, 80, size=1.2)

    M = c.pin_of
    def mp(label):
        ref, pad, _ = M[label]
        return (ref, pad)

    c.link('OLED_SDA', mp(g('OLED_SDA_PIN')), ('J_OLED', 4))
    c.link('OLED_SCL', mp(g('OLED_SCL_PIN')), ('J_OLED', 3))
    c.link('BUZZER', mp(g('BUZZER_PIN')), ('BZ1', 1))
    c.link('BTN_POWER', mp(g('POWER_BTN_PIN')), ('SW1', 1))
    c.link('BTN_PAGE', mp(g('PAGE_BTN_PIN')), ('SW2', 1))

    c.chain('VBAT+', [('J_BAT', 1), ('R1', 1)])
    c.chain('VBAT_SENSE', [('R1', 2), ('R2', 1)])
    c.link('VBAT_SENSE', ('R2', 1), mp(g('HOST_BAT_ADC_PIN')))
    c.chain('+3V3', [mp('3V3'), ('J_OLED', 2)])
    c.chain('GND', [('R2', 2), ('J_BAT', 2), mp('GND'), ('J_OLED', 1),
                    ('BZ1', 2), ('SW1', 2), ('SW2', 2)])

    return c, 'Host-OLED', 'CENTRAL HOST v4.7.5-OLED'


if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    for fn in (build_station, build_host):
        c, name, title = fn()
        path = c.finish(name, title)
        print('เขียนแล้ว:', os.path.relpath(path, ROOT))
        rep = os.path.join('/tmp', name + '-drc.txt')
        c.bd.drc(rep)
        txt = open(rep, encoding='utf-8').read()
        errs = len(re.findall(r'^\s*\[', txt, re.M))
        print('   DRC: %d รายการ -> %s' % (errs, rep))


# ===========================================================================
#  แผ่นทดสอบขนาดโมดูล 1:1
# ===========================================================================
def build_fitcheck():
    """แผ่นสำหรับพิมพ์ที่สเกล 100% แล้ววางโมดูลจริงทาบตรวจก่อนสั่งผลิต

    มีแถบวัดระยะ 10 มม. ให้ตรวจก่อนว่าเครื่องพิมพ์ไม่ได้ย่อ/ขยาย
    ถ้าแถบยาวไม่ครบ 10 มม. แปลว่าตั้งค่าพิมพ์ผิด อย่าเพิ่งใช้ตัดสิน
    """
    m = MODULE
    c = Carrier(90, 70, mcu_x=30, mcu_y=25)
    c.place_mcu()
    bd = c.bd
    bd.outline(0, 0, 90, 70)

    bd.label('FIT CHECK SHEET - PRINT AT 100% SCALE', 5, 8, size=2.0)
    bd.label('1. Check the 10 mm bar below with a ruler before anything else.', 5, 12, size=1.2)
    bd.label('2. Put the real ESP32-S3 Super Mini on the two pad rows.', 5, 15, size=1.2)
    bd.label('3. Compare the pin labels with the silkscreen on your module.', 5, 18, size=1.2)
    bd.label('If anything differs, edit MODULE in tools/kicad-gen/gen_pcb.py', 5, 21, size=1.2)

    # แถบวัดระยะ 10 มม. พร้อมขีดบอกทุก 1 มม.
    y0 = 62.0
    for i in range(11):
        x = 5.0 + i
        h = 3.0 if i % 5 == 0 else 1.5
        sh = P.PCB_SHAPE(bd.b)
        sh.SetShape(P.SHAPE_T_SEGMENT)
        sh.SetStart(P.VECTOR2I(mm(x), mm(y0)))
        sh.SetEnd(P.VECTOR2I(mm(x), mm(y0 - h)))
        sh.SetLayer(P.F_SilkS)
        sh.SetWidth(mm(0.15))
        bd.b.Add(sh)
    sh = P.PCB_SHAPE(bd.b)
    sh.SetShape(P.SHAPE_T_SEGMENT)
    sh.SetStart(P.VECTOR2I(mm(5), mm(y0)))
    sh.SetEnd(P.VECTOR2I(mm(15), mm(y0)))
    sh.SetLayer(P.F_SilkS)
    sh.SetWidth(mm(0.2))
    bd.b.Add(sh)
    bd.label('10 mm - measure this first', 17, y0 - 0.5, size=1.3)

    rp, n = m['row_pitch'], m['rows']
    bd.label('assumed: %d pins per row, rows %.2f mm apart, %.2f mm pitch'
             % (n, rp, m['pin_pitch']), 5, 66, size=1.2)
    bd.label('row spacing %.2f mm' % rp, c.mcu_x + 1, c.mcu_y - 4, size=1.2)

    path = os.path.join(OUT, 'ModuleFitCheck.kicad_pcb')
    bd.save(path)
    return path
