# -*- coding: utf-8 -*-
"""สร้างผังการเชื่อมต่ออุปกรณ์ (schematic) ของ Smart IV Alert สำหรับ KiCad 7

หมายเลขขาทุกเส้นอ่านจากไฟล์เฟิร์มแวร์โดยตรง ไม่ได้กรอกด้วยมือ
ถ้าเฟิร์มแวร์ย้ายขา ให้รันสคริปต์นี้ใหม่ ผังจะตามไปเอง

ข้อความในผังเป็นภาษาอังกฤษโดยตั้งใจ เพราะ KiCad 7 ไม่รองรับการจัดวาง
สระและวรรณยุกต์ไทย (ทดสอบแล้วเครื่องหมายบน-ล่างหายทั้งหมด) คำอธิบายภาษาไทย
ฉบับเต็มอยู่ที่ hardware/README.md
"""
import os, re, sys, datetime
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ksch import Sym, Sheet, power_sym, res_sym, sw_sym, q, uid

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
OUT = os.path.join(ROOT, 'hardware')
DATE = datetime.date.today().isoformat()
COMPANY = 'Boromarajonani College of Nursing Phrae / Sung Men Hospital'


def pins_from(ino, names):
    src = open(ino, encoding='utf-8').read()
    got = {}
    for n in names:
        m = re.search(r'^#define\s+%s\s+(-?\d+)' % re.escape(n), src, re.M)
        if not m:
            raise SystemExit('หาไม่พบ: %s ใน %s' % (n, ino))
        got[n] = int(m.group(1))
    return got


PWR = {'+3V3': power_sym('power:+3V3', '+3V3'),
       '+5V':  power_sym('power:+5V', '+5V'),
       'GND':  power_sym('power:GND', 'GND', down=True)}


def mcu_sym(gpios):
    """ESP32-S3 Super Mini — แสดงเฉพาะขาที่วงจรนี้ใช้จริง"""
    left = [('5V', '5V', 'power_in'), ('3V3', '3V3', 'power_out'),
            ('GND', 'GND', 'power_in')]
    right = [('GPIO%d' % g, 'IO%d' % g, 'bidirectional') for g in gpios]
    return Sym('Module:ESP32-S3-SuperMini', 'U', 30.48, left, right)


def stub(sh, at, net, side='R', length=8.89):
    dx = length if side == 'R' else -length
    end = (round(at[0] + dx, 3), at[1])
    sh.wire(at, end)
    sh.label(net, end, justify='left' if side == 'R' else 'right')
    return end


def rail(sh, at, kind, length):
    """ต่อขาไฟเลี้ยง/กราวด์ทางซ้าย ใช้ความยาวสายต่างกันเพื่อไม่ให้สัญลักษณ์ชนกัน"""
    end = (round(at[0] - length, 3), at[1])
    sh.wire(at, end)
    sh.place_simple('power:' + kind, '#PWR' + uid()[:5].upper(), kind,
                    end[0], end[1], npins=1)


def mcu_power(sh, at):
    rail(sh, at('5V'), '+5V', 12.7)
    rail(sh, at('3V3'), '+3V3', 25.4)
    rail(sh, at('GND'), 'GND', 12.7)


def notes(sh, x, y, lines, w=252.0, h=None):
    h = h or (len(lines) * 6.35 + 6.35)
    sh.box(x, y, round(x + w, 3), round(y + h, 3))
    for i, (t, bold) in enumerate(lines):
        sh.text(t, round(x + 3.81, 3), round(y + 7.62 + i * 6.35, 3),
                size=1.778 if bold else 1.524, bold=bold)


def button(sh, ref, net, caption, x, y):
    sh.place_simple('Switch:SW_Push', ref, caption, x, y, vdy=-7.62)
    sh.wire((round(x - 5.08, 3), y), (round(x - 15.24, 3), y))
    sh.label(net, (round(x - 15.24, 3), y), justify='right')
    sh.wire((round(x + 5.08, 3), y), (round(x + 12.7, 3), y))
    sh.place_simple('power:GND', '#PWR' + ref, 'GND', round(x + 12.7, 3), y, npins=1)


# =============================================================== STATION
def build_station():
    ino = os.path.join(ROOT, 'firmware', 'Station',
                       'Station.ino')
    P = pins_from(ino, ['TFT_CS', 'TFT_DC', 'TFT_RST', 'TFT_MOSI', 'TFT_SCLK',
                        'TFT_BLK', 'SENSOR_AO_PIN', 'BUZZER_PIN', 'BTN_PIN',
                        'RGB_LED_PIN'])
    gpios = sorted(set(P.values()))

    sh = Sheet('Smart IV Alert - Bed Station Node v7.7.3',
               '1.0', COMPANY)

    U1 = mcu_sym(gpios)
    J1 = Sym('Module:TFT_ST7789_1V47', 'J', 27.94,
             [('GND', '1', 'passive'), ('VCC', '2', 'power_in'),
              ('SCL', '3', 'input'), ('SDA', '4', 'input'),
              ('RES', '5', 'input'), ('DC', '6', 'input'),
              ('CS', '7', 'input'), ('BLK', '8', 'input')], [])
    U2 = Sym('Module:TCRT5000_Module', 'U', 27.94,
             [('VCC', '1', 'power_in'), ('GND', '2', 'passive'),
              ('DO', '3', 'output'), ('AO', '4', 'output')], [])
    BZ = Sym('Device:Buzzer_Passive', 'BZ', 17.78,
             [('+', '1', 'passive')], [('-', '2', 'passive')])

    for s in (U1, J1, U2, BZ):
        sh.add_def(s.defn())
    sh.add_def(sw_sym())
    for d in PWR.values():
        sh.add_def(d)

    u1 = sh.place(U1, 'U1', 'ESP32-S3 Super Mini', 88.9, 97.79)
    mcu_power(sh, u1)

    net_of = {P['TFT_SCLK']: 'TFT_SCLK', P['TFT_MOSI']: 'TFT_MOSI',
              P['TFT_RST']: 'TFT_RST', P['TFT_DC']: 'TFT_DC',
              P['TFT_CS']: 'TFT_CS', P['TFT_BLK']: 'TFT_BLK',
              P['SENSOR_AO_PIN']: 'DROP_AO', P['BUZZER_PIN']: 'BUZZER',
              P['BTN_PIN']: 'BTN', P['RGB_LED_PIN']: 'RGB_LED_ONBOARD'}
    for g in gpios:
        stub(sh, u1('IO%d' % g), net_of[g], 'R')

    j1 = sh.place(J1, 'J1', 'TFT ST7789 172x320 1.47in', 218.44, 71.12)
    rail(sh, j1('1'), 'GND', 12.7)
    rail(sh, j1('2'), '+3V3', 25.4)
    for num, net in (('3', 'TFT_SCLK'), ('4', 'TFT_MOSI'), ('5', 'TFT_RST'),
                     ('6', 'TFT_DC'), ('7', 'TFT_CS'), ('8', 'TFT_BLK')):
        stub(sh, j1(num), net, 'L')

    u2 = sh.place(U2, 'U2', 'TCRT5000 reflective drop sensor', 218.44, 130.81)
    rail(sh, u2('1'), '+3V3', 25.4)
    rail(sh, u2('2'), 'GND', 12.7)
    stub(sh, u2('4'), 'DROP_AO', 'L')
    sh.wire(u2('3'), (round(u2('3')[0] - 6.35, 3), u2('3')[1]))   # DO: not used
    sh.text('DO not connected', round(u2('3')[0] - 39.37, 3),
            round(u2('3')[1] + 1.27, 3), size=1.27)

    bz = sh.place(BZ, 'BZ1', 'Passive buzzer', 218.44, 163.83)
    stub(sh, bz('1'), 'BUZZER', 'L')
    rail(sh, bz('2'), 'GND', -12.7)

    button(sh, 'SW1', 'BTN', 'User button', 218.44, 186.69)

    notes(sh, 25.4, 207.01, [
        ('ASSEMBLY NOTES', True),
        ('1. All GPIO numbers are read directly from firmware v7.7.3 by the generator script;', False),
        ('   they are never typed in by hand. Re-run tools/kicad-gen/gen_schematics.py after a pin change.', False),
        ('2. SW1 uses pinMode(INPUT_PULLUP), so it wires straight to GND. No external pull-up resistor.', False),
        ('3. RGB_LED (GPIO%d) is the WS2812 already fitted on the ESP32-S3 Super Mini board.' % P['RGB_LED_PIN'], False),
        ('   Nothing is wired externally - the net is shown for reference only.', False),
        ('4. TCRT5000 DO pin is unused. The firmware reads the analog AO pin only.', False),
        ('5. Power the sensor and the display from 3V3, never 5V: ESP32-S3 GPIO pins are 3.3V only.', False),
        ('6. BZ1 is a PASSIVE buzzer driven by tone(). A magnetic buzzer needs an NPN driver stage.', False),
        ('7. Station battery sensing is disabled in firmware (STATION_BAT_ADC_PIN = -1), so it is not drawn.', False),
    ])
    return sh


# =============================================================== HOST OLED
def build_host():
    ino = os.path.join(ROOT, 'firmware', 'Host-OLED',
                       'Host-OLED.ino')
    P = pins_from(ino, ['HOST_BAT_ADC_PIN', 'POWER_BTN_PIN', 'PAGE_BTN_PIN',
                        'BUZZER_PIN', 'OLED_SDA_PIN', 'OLED_SCL_PIN'])
    gpios = sorted(set(P.values()))

    sh = Sheet('Smart IV Alert - Central Host v4.7.5-OLED',
               '1.0', COMPANY)

    U1 = mcu_sym(gpios)
    J1 = Sym('Module:OLED_SH1106_I2C', 'J', 27.94,
             [('GND', '1', 'passive'), ('VCC', '2', 'power_in'),
              ('SCL', '3', 'input'), ('SDA', '4', 'bidirectional')], [])
    BZ = Sym('Device:Buzzer_Passive', 'BZ', 17.78,
             [('+', '1', 'passive')], [('-', '2', 'passive')])
    J2 = Sym('Connector:Battery_1S', 'J', 22.86,
             [('VBAT+', '1', 'passive')], [('GND', '2', 'passive')])

    for s in (U1, J1, BZ, J2):
        sh.add_def(s.defn())
    sh.add_def(sw_sym())
    sh.add_def(res_sym())
    for d in PWR.values():
        sh.add_def(d)

    u1 = sh.place(U1, 'U1', 'ESP32-S3 Super Mini', 88.9, 91.44)
    mcu_power(sh, u1)

    net_of = {P['OLED_SDA_PIN']: 'OLED_SDA', P['OLED_SCL_PIN']: 'OLED_SCL',
              P['POWER_BTN_PIN']: 'BTN_POWER', P['PAGE_BTN_PIN']: 'BTN_PAGE',
              P['BUZZER_PIN']: 'BUZZER', P['HOST_BAT_ADC_PIN']: 'VBAT_SENSE'}
    for g in gpios:
        stub(sh, u1('IO%d' % g), net_of[g], 'R')

    j1 = sh.place(J1, 'J1', 'OLED SH1106 128x64 I2C', 218.44, 71.12)
    rail(sh, j1('1'), 'GND', 12.7)
    rail(sh, j1('2'), '+3V3', 25.4)
    stub(sh, j1('3'), 'OLED_SCL', 'L')
    stub(sh, j1('4'), 'OLED_SDA', 'L')

    bz = sh.place(BZ, 'BZ1', 'Passive buzzer', 218.44, 110.49)
    stub(sh, bz('1'), 'BUZZER', 'L')
    rail(sh, bz('2'), 'GND', -12.7)

    button(sh, 'SW1', 'BTN_POWER', 'Power / snooze (RTC GPIO)', 218.44, 133.35)
    button(sh, 'SW2', 'BTN_PAGE', 'Page / screensaver', 218.44, 151.13)

    # ---- ตัวแบ่งแรงดันแบตเตอรี่ 1:1 ----
    j2 = sh.place(J2, 'J2', 'Li-ion 1S battery', 218.44, 176.53)
    sh.wire(j2('1'), (round(j2('1')[0] - 15.24, 3), j2('1')[1]))
    sh.label('VBAT+', (round(j2('1')[0] - 15.24, 3), j2('1')[1]), justify='right')
    rail(sh, j2('2'), 'GND', -12.7)

    X, MID = 320.04, 176.53
    sh.place_simple('Device:R', 'R1', '100k', X, round(MID - 11.43, 3), npins=2)
    sh.wire((X, round(MID - 15.24, 3)), (X, round(MID - 24.13, 3)))
    sh.label('VBAT+', (X, round(MID - 24.13, 3)), rot=90, justify='left')
    sh.wire((X, round(MID - 7.62, 3)), (X, MID))

    sh.place_simple('Device:R', 'R2', '100k', X, round(MID + 11.43, 3), npins=2)
    sh.wire((X, round(MID + 7.62, 3)), (X, MID))
    sh.items.append('  (junction (at %s %s) (diameter 0) (color 0 0 0 0) (uuid %s))'
                    % (X, MID, q(uid())))
    sh.wire((X, MID), (round(X - 20.32, 3), MID))
    sh.label('VBAT_SENSE', (round(X - 20.32, 3), MID), justify='right')
    sh.wire((X, round(MID + 15.24, 3)), (X, round(MID + 20.32, 3)))
    sh.place_simple('power:GND', '#PWRR2', 'GND', X, round(MID + 20.32, 3), npins=1)
    sh.text('1:1 divider - both resistors MUST be equal', round(X - 19.05, 3),
            round(MID + 29.21, 3), size=1.27)

    notes(sh, 25.4, 207.01, [
        ('ASSEMBLY NOTES', True),
        ('1. All GPIO numbers are read directly from firmware v4.7.5-OLED by the generator script;', False),
        ('   they are never typed in by hand. Re-run tools/kicad-gen/gen_schematics.py after a pin change.', False),
        ('2. Both buttons use pinMode(INPUT_PULLUP), so they wire straight to GND. No external pull-ups.', False),
        ('3. SW1 MUST sit on an RTC GPIO (0-21): it wakes the board from deep sleep via ext0 at logic level 0.', False),
        ('4. Most OLED breakout modules already carry the I2C pull-ups. If yours does not,', False),
        ('   add 4.7k from SDA to 3V3 and 4.7k from SCL to 3V3.', False),
        ('5. The I2C bus runs at 400 kHz per the SH1106 datasheet. Keep both wires as short as practical.', False),
        ('6. R1 and R2 must be equal. The firmware computes (raw/4095) x 3.3 x 2.0, which fixes the ratio at 1:1.', False),
        ('7. A full 4.2V cell divides down to 2.1V, comfortably inside the ESP32-S3 ADC input range.', False),
    ])
    return sh


if __name__ == '__main__':
    os.makedirs(OUT, exist_ok=True)
    for name, fn in (('Station-BedNode', build_station), ('Host-OLED', build_host)):
        path = os.path.join(OUT, name + '.kicad_sch')
        open(path, 'w', encoding='utf-8').write(fn().render(DATE))
        print('เขียนแล้ว:', os.path.relpath(path, ROOT))
        pro = os.path.join(OUT, name + '.kicad_pro')
        open(pro, 'w', encoding='utf-8').write(
            '{\n  "board": {},\n  "meta": {"filename": "%s.kicad_pro", "version": 1},\n'
            '  "schematic": {},\n  "sheets": [],\n  "text_variables": {}\n}\n' % name)
        print('เขียนแล้ว:', os.path.relpath(pro, ROOT))
