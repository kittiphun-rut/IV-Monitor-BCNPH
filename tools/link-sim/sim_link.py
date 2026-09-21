#!/usr/bin/env python3
"""จำลอง "จังหวะเวลา" ของ ESP-NOW เพื่อวัดว่าทำไม Host ถึงขึ้น OFFLINE เมื่อมีหลายเตียง

ขอบเขตของแบบจำลอง (อ่านก่อนเชื่อผล)
  - จำลองเฉพาะ "การชนกันของจังหวะส่ง" ซึ่งเป็นกลไกที่เราแก้ในเวอร์ชันนี้
  - ไม่ได้จำลองสัญญาณอ่อน สิ่งกีดขวาง หรือสัญญาณรบกวนจาก Wi-Fi อื่น
  - สองเฟรมที่เวลาทับกันถือว่าหายทั้งคู่ (เป็นการประมาณแบบระวังไว้ก่อน)
ผลที่ได้จึงเป็น "ขอบล่าง" ของปัญหา ของจริงมักแย่กว่านี้ ไม่ใช่ดีกว่า

ใช้: sim_link.py [จำนวนเตียง] [นาทีที่จำลอง]
"""
import random
import sys

AIRTIME_MS      = 0.8      # เฟรม ESP-NOW 23-24 ไบต์ รวม preamble/header
CRYSTAL_PPM     = 20       # ความคลาดเคลื่อนคริสตัลของบอร์ดแต่ละตัว (+-)
SEND_INTERVAL   = 1000.0
JITTER_MS       = 60.0     # ของใหม่: สุ่มบวกลบเท่านี้ทุกครั้ง
HOST_GAP_OLD    = 4.0      # ของเดิม: delay(4) ระหว่างแพ็กเก็ต Sync แต่ละใบ


def station_periods(n, rng):
    """คาบส่งจริงของแต่ละบอร์ด ต่างกันตามคริสตัล"""
    return [SEND_INTERVAL * (1.0 + rng.uniform(-CRYSTAL_PPM, CRYSTAL_PPM) * 1e-6) for _ in range(n)]


def host_tx_windows(n, t_end, mode):
    """ช่วงเวลาที่ Host กำลังส่ง (ระหว่างนี้รับข้อมูลจาก Station ไม่ได้)"""
    out = []
    if mode == "old":                      # ยิงรวดเดียว N ใบทุก 1 วินาที
        t = 0.0
        while t < t_end:
            for k in range(n):
                s = t + k * (AIRTIME_MS + HOST_GAP_OLD)
                out.append((s, s + AIRTIME_MS))
            t += SEND_INTERVAL
    else:                                  # เฉลี่ยช่องเวลา ใบละ 1000/N ms
        slot = max(SEND_INTERVAL / n, 60.0)
        t = 0.0
        while t < t_end:
            out.append((t, t + AIRTIME_MS))
            t += slot
    out.sort()
    return out


def simulate(n, minutes, mode, seed=1):
    rng = random.Random(seed)
    t_end = minutes * 60_000.0
    periods = station_periods(n, rng)

    # เวลาที่แต่ละเตียงส่งแต่ละใบ
    tx = []                                 # (เวลา, เตียง)
    for i in range(n):
        if mode == "old":
            t = rng.uniform(0, SEND_INTERVAL)       # เฟสตอนบูตสุ่ม แต่หลังจากนั้นคงที่
            while t < t_end:
                tx.append((t, i))
                t += periods[i]
        else:
            t = (i + 1) * (SEND_INTERVAL / 9.0)     # รอบแรกเหลื่อมตามเลขเตียง
            while t < t_end:
                tx.append((t, i))
                t += periods[i] - JITTER_MS + rng.uniform(0, 2 * JITTER_MS)
    tx.sort()

    host = host_tx_windows(n, t_end, mode)

    # ---- หาว่าใบไหนหาย ----
    lost = [False] * len(tx)
    for k in range(len(tx) - 1):            # ชนกันเองระหว่างสเตชัน
        if tx[k + 1][0] - tx[k][0] < AIRTIME_MS:
            lost[k] = lost[k + 1] = True
    hi = 0
    for k, (t, _) in enumerate(tx):         # ชนกับตอน Host กำลังส่ง
        while hi < len(host) and host[hi][1] < t:
            hi += 1
        if hi < len(host) and host[hi][0] < t + AIRTIME_MS:
            lost[k] = True

    # ---- Host เห็นเตียงไหน OFFLINE นานแค่ไหน ----
    timeout = 8000.0 if mode == "new" else 5000.0   # ONLINE_TIMEOUT_MS ของแต่ละรุ่น
    last_ok = [0.0] * n
    offline_ms = [0.0] * n
    worst_gap = [0.0] * n
    rx = [0] * n
    sent = [0] * n
    for k, (t, i) in enumerate(tx):
        sent[i] += 1
        if lost[k]:
            continue
        rx[i] += 1
        gap = t - last_ok[i]
        if gap > worst_gap[i]:
            worst_gap[i] = gap
        if gap > timeout:
            offline_ms[i] += gap - timeout
        last_ok[i] = t

    return {
        "loss_pct": 100.0 * (1 - sum(rx) / max(sum(sent), 1)),
        "offline_s": sum(offline_ms) / 1000.0,
        "worst_gap_s": max(worst_gap) / 1000.0,
        "beds_hit": sum(1 for v in offline_ms if v > 0),
    }


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 8
    minutes = int(sys.argv[2]) if len(sys.argv) > 2 else 60

    print("จำลอง %d เตียง นาน %d นาที (เฉลี่ยจาก 5 เมล็ดสุ่ม)\n" % (n, minutes))
    print("%-28s %10s %14s %14s %10s" % ("", "ใบที่หาย", "เวลาที่ขึ้น", "ช่องว่างยาวสุด", "เตียงที่"))
    print("%-28s %10s %14s %14s %10s" % ("", "(%)", "OFFLINE (วินาที)", "(วินาที)", "โดนผล"))
    print("-" * 82)

    rows = []
    for label, mode in [("v4.7.2 + Station เดิม", "old"), ("v4.7.3 + Station v7.x.x ใหม่", "new")]:
        acc = [simulate(n, minutes, mode, seed=s) for s in range(1, 6)]
        avg = {k: sum(a[k] for a in acc) / len(acc) for k in acc[0]}
        rows.append(avg)
        print("%-28s %10.2f %14.1f %14.1f %10.1f"
              % (label, avg["loss_pct"], avg["offline_s"], avg["worst_gap_s"], avg["beds_hit"]))

    print("-" * 82)
    old, new = rows
    if new["offline_s"] < 0.05 * max(old["offline_s"], 1e-9) or new["offline_s"] < 1.0:
        print("ผ่าน: เวลาที่ Host ขึ้น OFFLINE ผิดพลาด ลดลงจาก %.1f วินาที เหลือ %.1f วินาที"
              % (old["offline_s"], new["offline_s"]))
        return 0
    print("ไม่ผ่าน: ยังมีช่วง OFFLINE เหลืออยู่มาก")
    return 1


if __name__ == "__main__":
    sys.exit(main())
