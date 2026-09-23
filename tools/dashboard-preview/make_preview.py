#!/usr/bin/env python3
"""ดึงหน้าเว็บ Dashboard ออกจาก web_dashboard.h แล้วใส่ข้อมูลจำลอง เพื่อเปิดดูบน PC

ใช้: python3 make_preview.py [โฟลเดอร์เฟิร์มแวร์] [ไฟล์ .html ปลายทาง]
ได้ไฟล์ HTML ที่เปิดดูได้เลยโดยไม่ต้องมีบอร์ด — ใช้ตรวจหน้าตาและพฤติกรรมของ Dashboard
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
src = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "firmware/Host-OLED"
out = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(__file__).resolve().parent / "preview.html"

text = (src / "web_dashboard.h").read_text(encoding="utf-8")
m = re.search(r'PAGE_INDEX\[\]\s*PROGMEM\s*=\s*R"rawliteral\((.*?)\)rawliteral";', text, re.S)
if not m:
    print("ไม่พบ PAGE_INDEX ในไฟล์", file=sys.stderr)
    sys.exit(1)
html = m.group(1)

# ---- ข้อมูลจำลอง: ครบทุกสถานะที่ต้องตรวจ ----
MOCK = """
<script>
(function () {
  const beds = [
    { id:1, name:'สมชาย ใจดี',   alert:0, vol:180,  rate:98.4,  plan:1000, link:100 },
    { id:2, name:'มาลี ศรีสุข',  alert:3, vol:820,  rate:96.0,  plan:1000, link:100 },
    { id:3, name:'ประสิทธิ์ พูนผล', alert:2, vol:410, rate:48.0, plan:1000, link:90  },
    { id:4, name:'จันทร์ เพ็ญศรี', alert:5, vol:640,  rate:0.0,   plan:1000, link:40  },
    { id:5, name:'อนงค์ วงศ์ทอง', alert:6, vol:0,    rate:0.0,   plan:1000, link:100 }
  ];
  const stations = beds.map(b => ({
    id:b.id, online:b.id!==3, running:true, rssi:-62, battery:3.95, link:b.link, rxTotal:1234,
    idConflict:(b.id===1), mac:'28:AB:'+(0x10+b.id).toString(16).toUpperCase(),
    totalDrops:Math.round(b.vol*20), volumeMl:b.vol, flowRateHr:b.rate,
    msSinceLastDrop:1200, targetRate:100, planVolume:b.plan, dropFactor:20,
    patientName:b.name, alertCode:b.alert, nearEndPct:80, nearEndAck:false,
    caseActive:true, caseStart:'18/09/2569 08:30'
  }));
  const data = {
    version:'4.7.6-OLED', activeCount:beds.length, currentTime:'18/09/2569 09:15:20',
    timeSynced:true, timeApprox:false, apClients:2, apMaxClients:8, channel:6,
    // v4.7.6: ฟิลด์ของโหมดเครือข่ายและจังหวะ poll ต้องตรงกับที่ handleApiData ส่งจริง
    pollMs:1000, netMode:'ROUTER', netIP:'192.168.1.42', staRssi:-58,
    hostBatVolts:4.05, hostBatPct:92, snoozed:false, linkWeakPct:60, syncFail:0, heardBeyond:0, stations
  };
  // ประวัติ 60 นาทีแบบสมจริง ใช้ให้กราฟและตารางในคู่มือมีข้อมูลให้ดู
  // เตียง 1 ไหลนิ่งราว 80 mL/h แล้วช่วงท้ายค่อย ๆ ช้าลง (สายเริ่มพับ) เพื่อให้เห็นรูปกราฟจริง
  const logRows = [];
  for (let i = 0; i < 60; i++) {
    const mm = String(15 + i).padStart(2, '0');
    const slow = i > 46 ? (1 - (i - 46) * 0.055) : 1;
    const rate = Math.max(0, (80 + Math.sin(i / 3.3) * 3.4) * slow);
    const drops = Math.round(rate * 20 / 60);
    logRows.push({
      min: i + 1,
      time: '18/09/2569 ' + String(8 + Math.floor((15 + i) / 60)).padStart(2, '0') + ':' + String((15 + i) % 60).padStart(2, '0') + ':00',
      drops: drops,
      vol: +(drops * (i + 1) / 20).toFixed(2),
      rate: +rate.toFixed(2),
      target: 80,
      alert: i > 55 ? 2 : 0,
      rssi: -58 - (i % 7),
      battery: +(4.05 - i * 0.0015).toFixed(2)
    });
  }
  const logs = { logs: logRows, dropFactor: 20 };
  window.fetch = function (url) {
    let body = {};
    if (String(url).includes('/api/data')) body = data;
    else if (String(url).includes('/api/logs')) body = logs;
    else if (String(url).includes('/api/ap/status'))
      body = { mode:'ROUTER', ip:'192.168.1.42', staSsid:'WARD-WIFI', staConfigured:true, staRssi:-58,
               ssid:'ESP32_Liquid_Monitor', password:'12345678', apIP:'192.168.4.1',
               channel:6, clients:2, maxClients:8, currentTime:data.currentTime };
    else if (String(url).includes('/api/wifi/scan'))
      body = { scanning:false, nets:[ {ssid:'WARD-WIFI',rssi:-52,lock:true},
                                      {ssid:'HOSPITAL-GUEST',rssi:-71,lock:false} ] };
    return Promise.resolve({ ok:true, json:() => Promise.resolve(body), text:() => Promise.resolve('') });
  };
  // ปิดเสียงในโหมดพรีวิว
  window.AudioContext = window.webkitAudioContext = function () {
    return { currentTime:0, state:'running', resume(){}, destination:{},
             createOscillator(){ return { type:'', frequency:{ setValueAtTime(){} },
                                          connect(){}, start(){}, stop(){} }; },
             createGain(){ return { gain:{ setValueAtTime(){}, exponentialRampToValueAtTime(){} },
                                    connect(){} }; } };
  };
})();
</script>
"""
html = html.replace("<script>", MOCK + "<script>", 1)

# เลือกหน้าที่จะถ่ายภาพ ส่งมาทาง argv[3] (live | graphs | logs | about | wifi)
TAB = sys.argv[3] if len(sys.argv) > 3 else "live"
if TAB == "wifi":
    # เปิดกล่อง "จุดเชื่อมต่อ" แล้วสั่งค้นหาเครือข่าย เพื่อถ่ายหน้าตั้งค่าเราเตอร์
    html = html.replace("</body>", """
<script>
  window.addEventListener('load', function () {
    setTimeout(function () { openApModal(); scanWifi(); }, 900);
  });
</script>
</body>""", 1)
elif TAB != "live":
    html = html.replace("</body>", """
<script>
  window.addEventListener('load', function () {
    setTimeout(function () {
      var btns = document.querySelectorAll('.main-nav .nav-btn');
      var idx = { live:0, graphs:1, logs:2, about:3 }['%s'];
      if (btns[idx]) btns[idx].click();
    }, 900);
  });
</script>
</body>""" % TAB, 1)
out.write_text(html, encoding="utf-8")
print("เขียนแล้ว:", out, f"({len(html)} ตัวอักษร)")
