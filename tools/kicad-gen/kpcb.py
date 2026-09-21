# -*- coding: utf-8 -*-
"""ตัวช่วยสร้างแผ่นวงจรพิมพ์ (.kicad_pcb) ด้วย pcbnew API ของ KiCad 7

วิธีเดินลายที่ใช้: แมนฮัตตันสองชั้น
  F.Cu เดินเฉพาะแนวตั้ง   B.Cu เดินเฉพาะแนวนอน   ต่อกันด้วยเวีย
เมื่อบังคับให้ทุกขาที่ต้องเดินสายมีค่า y ไม่ซ้ำกัน และทุกเส้นได้คอลัมน์ x ของตัวเอง
ลายจะไม่มีทางตัดกันเลยโดยโครงสร้าง ไม่ต้องพึ่งการจัดวางด้วยสายตา
"""
import pcbnew as P

MM = P.FromMM


def mm(v):
    return P.FromMM(float(v))


class Board:
    def __init__(self, track_mm=0.4, via_mm=0.8, drill_mm=0.4, clearance_mm=0.25):
        self.b = P.BOARD()
        self.track_w = mm(track_mm)
        self.via_w = mm(via_mm)
        self.via_d = mm(drill_mm)
        self.clearance = mm(clearance_mm)
        self.fps = {}
        self._setup_rules()

    def _setup_rules(self):
        ds = self.b.GetDesignSettings()
        ds.SetCopperLayerCount(2)
        # ความกว้างลายและขนาดเวียกำหนดรายชิ้นตอนสร้าง จึงตั้งที่คลาสเน็ตเฉพาะระยะห่าง
        try:
            nc = ds.m_NetSettings.m_DefaultNetClass
            nc.SetClearance(self.clearance)
            nc.SetTrackWidth(self.track_w)
            nc.SetViaDiameter(self.via_w)
            nc.SetViaDrill(self.via_d)
        except AttributeError:
            pass          # KiCad บางรุ่นเข้าถึงคลาสเน็ตจาก Python ไม่ได้ ใช้ค่าปริยายแทน

    # ---------------------------------------------------------------- เน็ต
    def net(self, name):
        ni = self.b.FindNet(name)
        if ni is None:
            ni = P.NETINFO_ITEM(self.b, name)
            self.b.Add(ni)
        return ni

    # ------------------------------------------------------------ footprint
    def add(self, ref, lib, fp_name, x, y, rot=0, value=''):
        path = '/usr/share/kicad/footprints/%s.pretty' % lib
        fp = P.FootprintLoad(path, fp_name)
        if fp is None:
            raise SystemExit('โหลด footprint ไม่ได้: %s / %s' % (lib, fp_name))
        fp.SetPosition(P.VECTOR2I(mm(x), mm(y)))
        if rot:
            fp.SetOrientationDegrees(rot)
        fp.SetReference(ref)
        fp.SetValue(value or fp_name)
        fp.Reference().SetVisible(True)
        fp.Value().SetVisible(False)
        self.b.Add(fp)
        self.fps[ref] = fp
        return fp

    def pad(self, ref, number):
        fp = self.fps[ref]
        for p in fp.Pads():
            if p.GetNumber() == str(number):
                return p
        raise KeyError('%s ไม่มีขา %s' % (ref, number))

    def pad_xy(self, ref, number):
        pos = self.pad(ref, number).GetPosition()
        return (pos.x, pos.y)

    def assign(self, ref, number, net_name):
        # ปุ่มกดสัมผัสมีสองรูต่อหนึ่งเลขขา ถ้าผูกเน็ตให้รูเดียว อีกรูจะกลายเป็นเน็ตว่าง
        # แล้ว DRC จะฟ้องว่าลายของเน็ตนั้นเองเข้าใกล้ "ขาต่างเน็ต" เกินระยะ
        n = self.net(net_name)
        hit = 0
        for pd in self.fps[ref].Pads():
            if pd.GetNumber() == str(number):
                pd.SetNet(n); hit += 1
        if not hit:
            raise KeyError('%s ไม่มีขา %s' % (ref, number))

    # --------------------------------------------------------------- ลายทอง
    def track(self, a, b, layer, net_name):
        t = P.PCB_TRACK(self.b)
        t.SetStart(P.VECTOR2I(int(a[0]), int(a[1])))
        t.SetEnd(P.VECTOR2I(int(b[0]), int(b[1])))
        t.SetWidth(self.track_w)
        t.SetLayer(layer)
        t.SetNet(self.net(net_name))
        self.b.Add(t)

    def via(self, at, net_name):
        v = P.PCB_VIA(self.b)
        v.SetPosition(P.VECTOR2I(int(at[0]), int(at[1])))
        v.SetWidth(self.via_w)
        v.SetDrill(self.via_d)
        v.SetNet(self.net(net_name))
        v.SetLayer(P.F_Cu)
        v.SetBottomLayer(P.B_Cu)
        self.b.Add(v)

    def hseg(self, a, b, net_name):
        """แนวนอนบน B.Cu (a และ b ต้องมี y เท่ากัน)"""
        if a[0] != b[0]:
            self.track(a, b, P.B_Cu, net_name)

    def vseg(self, a, b, net_name):
        """แนวตั้งบน F.Cu"""
        if a[1] != b[1]:
            self.track(a, b, P.F_Cu, net_name)

    def route(self, p1, p2, cx, net_name):
        """เดินสายแบบ 3 ท่อน: นอน -> ตั้งที่คอลัมน์ cx -> นอน"""
        x1, y1 = p1
        x2, y2 = p2
        self.hseg((x1, y1), (cx, y1), net_name)
        self.via((cx, y1), net_name)
        self.vseg((cx, y1), (cx, y2), net_name)
        self.via((cx, y2), net_name)
        self.hseg((cx, y2), (x2, y2), net_name)

    def route_via_lane(self, p1, p2, cx1, ly, cx2, net_name):
        """เดินสาย 5 ท่อน สำหรับขาที่ต้องข้ามไปอีกฝั่งของหัวต่อ
        นอน -> ตั้งที่ cx1 -> นอนที่เลน ly -> ตั้งที่ cx2 -> นอน"""
        x1, y1 = p1
        x2, y2 = p2
        self.hseg((x1, y1), (cx1, y1), net_name)
        self.via((cx1, y1), net_name)
        self.vseg((cx1, y1), (cx1, ly), net_name)
        self.via((cx1, ly), net_name)
        self.hseg((cx1, ly), (cx2, ly), net_name)
        self.via((cx2, ly), net_name)
        self.vseg((cx2, ly), (cx2, y2), net_name)
        self.via((cx2, y2), net_name)
        self.hseg((cx2, y2), (x2, y2), net_name)

    # --------------------------------------------------------------- กราฟิก
    def outline(self, x1, y1, x2, y2, r=3.0):
        """ขอบบอร์ดสี่เหลี่ยมมุมมน"""
        import math
        pts = []
        for cx, cy, a0 in ((x2 - r, y1 + r, -90), (x2 - r, y2 - r, 0),
                           (x1 + r, y2 - r, 90), (x1 + r, y1 + r, 180)):
            for k in range(13):
                a = math.radians(a0 + k * 90 / 12.0)
                pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
        for i in range(len(pts)):
            a, b = pts[i], pts[(i + 1) % len(pts)]
            s = P.PCB_SHAPE(self.b)
            s.SetShape(P.SHAPE_T_SEGMENT)
            s.SetStart(P.VECTOR2I(mm(a[0]), mm(a[1])))
            s.SetEnd(P.VECTOR2I(mm(b[0]), mm(b[1])))
            s.SetLayer(P.Edge_Cuts)
            s.SetWidth(mm(0.1))
            self.b.Add(s)

    def label(self, text, x, y, size=1.0, layer=None, rot=0, center=False):
        t = P.PCB_TEXT(self.b)
        t.SetText(text)
        # ปริยายของ KiCad คือจัดกึ่งกลาง ข้อความยาว ๆ จึงล้นออกนอกขอบบอร์ด
        if not center:
            t.SetHorizJustify(P.GR_TEXT_H_ALIGN_LEFT)
        t.SetPosition(P.VECTOR2I(mm(x), mm(y)))
        t.SetLayer(layer if layer is not None else P.F_SilkS)
        t.SetTextSize(P.VECTOR2I(mm(size), mm(size)))
        t.SetTextThickness(mm(size * 0.15))
        if rot:
            t.SetTextAngleDegrees(rot)
        self.b.Add(t)

    def mounting_hole(self, x, y, drill=3.2):
        fp = P.FootprintLoad('/usr/share/kicad/footprints/MountingHole.pretty',
                             'MountingHole_3.2mm_M3')
        if fp is None:
            return
        fp.SetPosition(P.VECTOR2I(mm(x), mm(y)))
        self._holes = getattr(self, '_holes', 0) + 1
        fp.SetReference('H%d' % self._holes)
        fp.Reference().SetVisible(False)
        self.b.Add(fp)

    # ----------------------------------------------------------------- บันทึก
    def save(self, path):
        self.b.BuildListOfNets()
        P.Refresh
        self.b.Save(path)

    def drc(self, report_path):
        self.b.BuildListOfNets()
        ok = P.WriteDRCReport(self.b, report_path, P.EDA_UNITS_MILLIMETRES, True)
        return ok
