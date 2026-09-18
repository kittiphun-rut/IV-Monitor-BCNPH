#pragma once
/**
 * ============================================================================
 * หน้าเว็บ Dashboard ของระบบ Smart IV Alert (Central Host)
 * ----------------------------------------------------------------------------
 * แยกออกมาจากไฟล์ .ino หลัก เพื่อให้ส่วนเฟิร์มแวร์ (การวัด การแจ้งเตือน ESP-NOW
 * และการแสดงผลบนจอ) อ่านและแก้ไขได้ง่ายขึ้น — ไฟล์นี้เก็บเฉพาะ HTML/CSS/JavaScript
 * ของหน้าเว็บที่ส่งให้เบราว์เซอร์เท่านั้น ไม่มีตรรกะของเครื่อง
 *
 * วิธีแก้ไข
 *  - แก้หน้าตา/ข้อความของหน้าเว็บ ให้แก้ในไฟล์นี้ไฟล์เดียว
 *  - ข้อความทั้งก้อนอยู่ใน raw string R"rawliteral( ... )rawliteral"
 *    จึงใส่เครื่องหมาย " ได้ตามปกติ แต่ห้ามมีข้อความ )rawliteral" อยู่ข้างใน
 *  - ตัวแปรชื่อ PAGE_INDEX ถูกส่งออกด้วย server.send_P() ในไฟล์ .ino
 *  - ไฟล์นี้ต้องอยู่ในโฟลเดอร์เดียวกับไฟล์ .ino (Arduino IDE จะคอมไพล์ให้เอง)
 *
 * ขนาดโดยประมาณ 90 KB เก็บใน Flash (PROGMEM) ไม่กิน RAM ระหว่างทำงาน
 * ============================================================================
 */

#include <Arduino.h>

const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Smart IV Alert System v4.6.0</title>
  <style>
    :root {
      --primary: #0072ff;
      --primary-gradient: linear-gradient(135deg, #0072ff 0%, #00c6ff 100%);
      --bg-color: #f0f4f9;
      --card-bg: #ffffff;
      --text-main: #2d3748;
      --text-muted: #718096;
      --border-color: #e2e8f0;
      --green-bg: #e6fffa;
      --green-text: #234e52;
      --alarm-red: #fff5f5;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Sarabun", sans-serif; }
    body { background-color: var(--bg-color); color: var(--text-main); padding-bottom: 35px; }

    .header {
      background: var(--primary-gradient);
      color: white;
      padding: 16px 22px;
      box-shadow: 0 4px 12px rgba(0, 114, 255, 0.25);
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 10px;
    }
    .header-title { font-size: 20px; font-weight: 800; display: flex; align-items: center; }
    .header-badge { font-size: 12px; background: rgba(255,255,255,0.25); padding: 3px 9px; border-radius: 6px; margin-left: 8px; }
    .header-subtitle { font-size: 13.5px; opacity: 0.9; margin-top: 3px; display:flex; gap:12px; align-items:center; }
    .host-bat-badge {
      background: rgba(0,0,0,0.2);
      border: 1px solid rgba(255,255,255,0.3);
      padding: 5px 12px;
      border-radius: 8px;
      font-size: 13.5px;
      font-weight: 700;
      display: flex;
      align-items: center;
      gap: 4px;
    }
    .wifi-btn {
      background: rgba(255,255,255,0.25);
      border: 1px solid rgba(255,255,255,0.4);
      color: white;
      padding: 9px 14px;
      border-radius: 10px;
      cursor: pointer;
      font-size: 14.5px;
      font-weight: 600;
    }

    .main-nav {
      background: #ffffff;
      display: flex;
      justify-content: center;
      gap: 10px;
      padding: 12px 16px;
      border-bottom: 1px solid var(--border-color);
      box-shadow: 0 2px 5px rgba(0,0,0,0.02);
      position: sticky;
      top: 0;
      z-index: 100;
      flex-wrap: wrap;
    }
    .nav-btn {
      padding: 10px 18px;
      border-radius: 12px;
      border: none;
      background: #f7fafc;
      color: #4a5568;
      font-weight: 700;
      font-size: 15px;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 6px;
      transition: all 0.2s;
    }
    .nav-btn.active {
      background: var(--primary);
      color: white;
      box-shadow: 0 4px 10px rgba(0, 114, 255, 0.3);
    }

    .main-wrap { max-width: 1350px; margin: 22px auto; padding: 0 16px; }
    .tab-content { display: none; }
    .tab-content.active { display: block; }

    .station-control-bar {
      background: #ffffff;
      padding: 14px 20px;
      border-radius: 14px;
      margin-bottom: 20px;
      border: 1px solid var(--border-color);
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 12px;
    }
    .scale-btn-group { display: flex; align-items: center; gap: 8px; }
    .scale-btn {
      background: #edf2f7;
      border: 1px solid var(--border-color);
      padding: 8px 16px;
      border-radius: 10px;
      cursor: pointer;
      font-weight: 700;
      font-size: 14.5px;
      color: #2d3748;
      transition: all 0.2s;
    }
    .scale-btn:hover { background: #e2e8f0; }

    .station-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
      gap: 16px;
      margin-bottom: 26px;
    }
    .station-card {
      background: var(--card-bg);
      border-radius: 16px;
      padding: 18px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.05);
      border: 2px solid transparent;
      transition: all 0.2s;
      cursor: pointer;
    }
    .station-card:hover { transform: translateY(-2px); }
    .station-card.selected {
      border-color: var(--primary);
      box-shadow: 0 6px 16px rgba(0, 114, 255, 0.2);
      background: #f8fbff;
    }
    /* (1) สถานะปกติ = กรอบสีเขียว มองแวบเดียวรู้ว่าเตียงไหนเรียบร้อย */
    .station-card.normal-state {
      border-color: #38a169 !important;
      box-shadow: 0 4px 12px rgba(0,0,0,0.05), 0 0 0 3px rgba(56,161,105,0.16);
    }
    .station-card.paused-state {
      border-color: #f59e0b !important;
      background: #fffbeb !important;
    }
    .station-card.alarm-state {
      background: var(--alarm-red) !important;
      border-color: #e53e3e !important;
      animation: pulseAlert 1.5s infinite;
    }
    @keyframes pulseAlert {
      0% { box-shadow: 0 0 0 0 rgba(229, 62, 62, 0.4); }
      70% { box-shadow: 0 0 0 10px rgba(229, 62, 62, 0); }
      100% { box-shadow: 0 0 0 0 rgba(229, 62, 62, 0); }
    }

    .station-header { display: flex; justify-content: space-between; align-items: flex-start; margin-bottom: 10px; }
    .station-title { font-weight: 800; font-size: 17px; display: flex; align-items: center; gap: 5px; color: #1a202c; }
    .patient-label { font-size: 14.5px; font-weight: 600; color: var(--primary); margin-top: 3px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; max-width: 170px; }

    .badge-status { font-size: 11.5px; padding: 4px 9px; border-radius: 20px; font-weight: 700; }
    .badge-online { background: #c6f6d5; color: #22543d; }
    .badge-offline { background: #fed7d7; color: #742a2a; }
    .badge-paused { background: #fef3c7; color: #92400e; border: 1px solid #f59e0b; }
    .badge-alarm { background: #e53e3e; color: white; }

    .telemetry-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      background: #f7fafc;
      padding: 7px 12px;
      border-radius: 8px;
      margin-bottom: 11px;
      font-size: 12px;
    }
    .signal-bars { display: flex; align-items: flex-end; gap: 2px; height: 13px; }
    .signal-bars .bar { width: 4px; background: #e2e8f0; border-radius: 1px; }
    .signal-bars .b1 { height: 25%; }
    .signal-bars .b2 { height: 50%; }
    .signal-bars .b3 { height: 75%; }
    .signal-bars .b4 { height: 100%; }

    .signal-bars.lvl-4 .b1, .signal-bars.lvl-4 .b2, .signal-bars.lvl-4 .b3, .signal-bars.lvl-4 .b4 { background: #38a169; }
    .signal-bars.lvl-3 .b1, .signal-bars.lvl-3 .b2, .signal-bars.lvl-3 .b3 { background: #3182ce; }
    .signal-bars.lvl-2 .b1, .signal-bars.lvl-2 .b2 { background: #dd6b20; }
    .signal-bars.lvl-1 .b1 { background: #e53e3e; }

    .stat-row { display: flex; justify-content: space-between; margin-bottom: 6px; font-size: 14px; }
    .stat-row .val { font-weight: 700; color: #1a202c; }

    .compare-box {
      background: #edf2f7;
      border-radius: 8px;
      padding: 7px 10px;
      margin-top: 9px;
      font-size: 12px;
      display: flex;
      justify-content: space-between;
    }

    .calc-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(330px, 1fr));
      gap: 22px;
      margin-bottom: 26px;
    }
    .card {
      background: var(--card-bg);
      border-radius: 16px;
      padding: 22px;
      box-shadow: 0 4px 15px rgba(0, 0, 0, 0.05);
      border: 1px solid var(--border-color);
      margin-bottom: 22px;
    }
    .card-title { font-size: 18px; font-weight: 700; margin-bottom: 18px; display: flex; justify-content: space-between; align-items: center; }
    .form-group { margin-bottom: 16px; }
    .form-group label { display: block; font-size: 14.5px; font-weight: 600; margin-bottom: 7px; color: var(--text-muted); }
    .input-field {
      width: 100%;
      padding: 11px 15px;
      border-radius: 10px;
      border: 1px solid var(--border-color);
      font-size: 16.5px;
      background: #fafafa;
      outline: none;
    }
    .pill-group { display: flex; gap: 8px; flex-wrap: wrap; margin-bottom: 15px; }
    .pill-btn {
      flex: 1;
      min-width: 55px;
      padding: 9px 13px;
      border-radius: 10px;
      border: 1px solid var(--border-color);
      background: #fff;
      cursor: pointer;
      font-weight: 600;
      font-size: 14.5px;
      text-align: center;
    }
    .pill-btn.active { background: var(--primary); color: #fff; border-color: var(--primary); }

    .btn-group { display: flex; gap: 11px; margin-top: 18px; }
    .btn {
      flex: 1;
      padding: 13px;
      border-radius: 12px;
      font-size: 15.5px;
      font-weight: 600;
      border: none;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
    }
    .btn-primary { background: var(--primary); color: white; }
    .btn-secondary { background: #edf2f7; color: #4a5568; }
    .btn-green { background: #38a169; color: white; }
    .btn-orange { background: #dd6b20; color: white; }
    .btn-danger { background: #e53e3e; color: white; }
    .badge-warn { background: #feebc8; color: #7b341e; border: 1px solid #dd6b20; }
    .station-card.warn-state { border-color: #dd6b20 !important; background: #fffaf0 !important; }
    .progress-track { height: 7px; background: #e2e8f0; border-radius: 4px; overflow: hidden; margin: 4px 0 8px; }
    .progress-fill { height: 100%; background: linear-gradient(90deg, #0072ff, #00c6ff); }
    .ack-btn { margin-top: 10px; width: 100%; padding: 9px; border: 0; border-radius: 10px; background: #dd6b20; color: #fff; font-weight: 700; font-size: 14.5px; cursor: pointer; }
    .near-line { font-size: 12.5px; color: #9c4221; margin-top: 4px; }

    /* (3) แถบแจ้งเตือนรวมบนหน้า Live Monitor — บอกว่าเตียงไหนเป็นอะไร */
    .alert-banner {
      display: none;
      background: linear-gradient(135deg, #e53e3e, #c53030);
      color: #fff;
      border-radius: 14px;
      padding: 14px 18px;
      margin-bottom: 16px;
      box-shadow: 0 6px 18px rgba(229, 62, 62, 0.35);
      animation: bannerPulse 1.2s ease-in-out infinite;
    }
    .alert-banner.show { display: block; }
    .alert-banner .ab-title { font-size: 17px; font-weight: 800; margin-bottom: 8px; letter-spacing: .3px; }
    .alert-banner .ab-list { display: flex; flex-wrap: wrap; gap: 8px; }
    .alert-banner .ab-item {
      background: rgba(255,255,255,0.18);
      border: 1px solid rgba(255,255,255,0.45);
      border-radius: 10px;
      padding: 7px 13px;
      font-size: 15.5px;
      font-weight: 700;
    }
    .alert-banner .ab-item b { font-size: 17px; }
    @keyframes bannerPulse {
      0%, 100% { box-shadow: 0 6px 18px rgba(229,62,62,0.35); }
      50%      { box-shadow: 0 6px 26px rgba(229,62,62,0.75); }
    }
    .watch-banner {
      display: none;
      background: #fffaf0;
      border: 2px solid #dd6b20;
      color: #7b341e;
      border-radius: 14px;
      padding: 12px 18px;
      margin-bottom: 16px;
      font-size: 15.5px;
      font-weight: 700;
    }
    .watch-banner.show { display: block; }
    .sync-note { font-size: 12.5px; color: var(--text-muted); margin-top: 10px; line-height: 1.5; }

    .result-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 14px; margin-bottom: 15px; }
    .metric-card {
      background: #f7fafc;
      border: 1px solid var(--border-color);
      border-radius: 12px;
      padding: 14px;
      text-align: center;
    }
    .metric-card .title { font-size: 13.5px; color: var(--text-muted); }
    .metric-card .val { font-size: 24.5px; font-weight: 800; color: var(--primary); margin: 5px 0; }
    .metric-card .sub { font-size: 12px; color: #a0aec0; }

    .highlight-card {
      background: var(--green-bg);
      color: var(--green-text);
      border-radius: 12px;
      padding: 15px;
      margin-bottom: 14px;
      text-align: center;
    }
    .highlight-card .big-text { font-size: 20px; font-weight: 800; }
    .highlight-card .sub-text { font-size: 12px; }

    .chamber-box {
      height: 185px;
      background: #fff;
      border: 1px solid var(--border-color);
      border-radius: 14px;
      display: flex;
      flex-direction: column;
      align-items: center;
      position: relative;
      overflow: hidden;
      margin-top: 10px;
    }
    .iv-tube-top { width: 6px; height: 20px; background: #cbd5e0; }
    .iv-chamber { width: 40px; height: 80px; border: 2px solid #a0aec0; border-radius: 10px 10px 20px 20px; position: relative; }
    .iv-nozzle { width: 4px; height: 10px; background: #718096; margin: 0 auto; }
    .iv-pool { position: absolute; bottom: 0; width: 100%; height: 18px; background: #63b3ed; border-radius: 0 0 18px 18px; }
    .drop {
      width: 8px; height: 12px; background: #3182ce; border-radius: 50%; position: absolute; left: 16px; top: 12px; opacity: 0;
    }
    .drop.animate { animation: fall linear infinite; }
    @keyframes fall {
      0% { top: 12px; opacity: 1; }
      80% { top: 58px; opacity: 1; }
      100% { top: 62px; opacity: 0; }
    }
    .sim-status-text { font-size: 13.5px; color: var(--text-muted); margin-top: 9px; }

    .tab-bar {
      display: flex;
      gap: 9px;
      margin-bottom: 18px;
      overflow-x: auto;
      padding-bottom: 5px;
    }
    .tab-item {
      padding: 9px 18px;
      border-radius: 10px;
      background: #edf2f7;
      color: #4a5568;
      font-weight: 700;
      font-size: 14.5px;
      cursor: pointer;
      white-space: nowrap;
      transition: all 0.2s;
    }
    .tab-item.active { background: var(--primary); color: white; }

    .chart-container { position: relative; width: 100%; height: 290px; margin-bottom: 22px; }
    canvas { width: 100% !important; height: 100% !important; background: #fafcff; border-radius: 12px; border: 1px solid var(--border-color); }

    .table-container { max-height: 420px; overflow-y: auto; border: 1px solid var(--border-color); border-radius: 12px; }
    .log-table { width: 100%; border-collapse: collapse; font-size: 14.5px; text-align: left; }
    .log-table th { background: #f8fafc; color: #4a5568; font-weight: 700; padding: 13px 15px; position: sticky; top: 0; border-bottom: 2px solid var(--border-color); }
    .log-table td { padding: 11px 15px; border-bottom: 1px solid var(--border-color); color: #2d3748; }
    .log-table tr:hover { background: #f7fafc; }

    /* แท็บ About & Research Team */
    .about-header-box {
      background: #ffffff;
      padding: 24px;
      border-radius: 16px;
      border: 1px solid var(--border-color);
      margin-bottom: 20px;
    }
    .about-header-box h2 { font-size: 20px; color: var(--text-main); margin-bottom: 10px; }
    .about-header-box p { color: var(--text-muted); font-size: 14.5px; line-height: 1.6; }
    
    .team-grid {
      display: grid;
      grid-template-columns: 2fr 1fr;
      gap: 20px;
      margin-bottom: 20px;
    }
    @media (max-width: 900px) {
      .team-grid { grid-template-columns: 1fr; }
    }
    .team-card {
      background: #ffffff;
      border-radius: 14px;
      padding: 22px;
      border: 1px solid var(--border-color);
      box-shadow: 0 4px 10px rgba(0,0,0,0.02);
    }
    .team-card-title { font-size: 17px; font-weight: 800; color: var(--primary); margin-bottom: 14px; display: flex; align-items: center; gap: 6px; }
    
    .unified-member-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 8px 16px;
    }
    @media (max-width: 600px) {
      .unified-member-grid { grid-template-columns: 1fr; }
    }
    .member-item {
      padding: 8px 0;
      border-bottom: 1px dashed #edf2f7;
      font-size: 14px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    .member-item .sid { color: var(--text-muted); font-size: 12.5px; font-family: monospace; }

    .modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.5); z-index: 999; justify-content: center; align-items: center; }
    .modal-content { background: white; width: 90%; max-width: 440px; padding: 26px; border-radius: 16px; font-size: 14.5px; }
  </style>
</head>
<body>

  <div class="header">
    <div>
      <h1 class="header-title">
        🏥 Smart IV Alert Central System
        <span class="header-badge" id="versionBadge">—</span>
      </h1>
      <div class="header-subtitle">
        <span id="netStatusSubtitle">AP: 192.168.4.1 · กำลังตรวจสอบ...</span>
        <span>🕒 เวลา: <b id="hostClockDisplay">กำลังซิงก์...</b></span>
      </div>
    </div>
    <div style="display:flex; gap:9px; align-items:center;">
      <div class="host-bat-badge" id="hostBatDisplay">🔋 Host: 100%</div>
      <button class="wifi-btn" id="audioToggleBtn" onclick="toggleAudioMute()">🔔 เปิดเสียงเตือน</button>
      <button class="wifi-btn" onclick="openApModal()">📶 จุดเชื่อมต่อ</button>
    </div>
  </div>

  <div class="main-nav">
    <button class="nav-btn active" onclick="switchMainTab('tab-live', this)">🛏️ Live Monitor</button>
    <button class="nav-btn" onclick="switchMainTab('tab-graphs', this)">📈 Visual Graphs (1 ชม.)</button>
    <button class="nav-btn" onclick="switchMainTab('tab-logs', this)">📋 Log & Shift Report</button>
    <button class="nav-btn" onclick="switchMainTab('tab-about', this)">👥 เกี่ยวกับงานวิจัย & คณะผู้จัดทำ</button>
  </div>

  <div class="main-wrap">
    <div id="tab-live" class="tab-content active">
      <div class="station-control-bar">
        <span style="font-weight:700; font-size:16.5px; color:#2d3748;">
          🛏️ สถานะภาพรวมสารน้ำ (<span id="activeBedCountLabel">5</span> เตียง)
        </span>
        <div class="scale-btn-group">
          <span style="font-size:14px; color:var(--text-muted); margin-right:4px;">ปรับจำนวนเตียง:</span>
          <button class="scale-btn" onclick="changeBedScale(-1)">➖ ลดเตียง</button>
          <button class="scale-btn" onclick="changeBedScale(1)" style="background:var(--primary); color:#fff; border-color:var(--primary);">➕ เพิ่มเตียง</button>
        </div>
      </div>

      <!-- (3) แถบแจ้งเตือนรวม: ไหลช้า / ไหลเร็ว / ไม่ไหล / ให้ครบแล้ว / เซนเซอร์ไม่จับหยด -->
      <div class="alert-banner" id="alertBanner">
        <div class="ab-title">🚨 ต้องไปดูเตียงเหล่านี้</div>
        <div class="ab-list" id="alertBannerList"></div>
      </div>
      <div class="watch-banner" id="watchBanner"></div>

      <div class="station-grid" id="stationGridContainer"></div>

      <div style="background:#ffffff; padding:16px 20px; border-radius:14px; margin-bottom:22px; border:1px solid var(--border-color); display:flex; align-items:center; justify-content:space-between; flex-wrap:wrap; gap:12px;">
        <span style="font-weight:700; font-size:15.5px; color:#2d3748;">
          ⚙️ เลือกเตียงที่ต้องการตั้งค่า: <span style="color:var(--primary); font-size:17.5px;" id="activeBedBadgeText">เตียง 1</span>
        </span>
        <div class="tab-bar" style="margin-bottom:0; padding-bottom:0;" id="calcBedSelectorTabs"></div>
      </div>

      <div class="calc-grid">
        <div class="card">
          <div class="card-title">
            <span>📝 ข้อมูลผู้ป่วย & คำนวณอัตราหยด</span>
            <span style="font-size:13.5px; color:var(--primary);" id="cardBedTag1">[เตียง 1]</span>
          </div>
          <div class="form-group">
            <label>👤 ชื่อ-นามสกุล คนไข้ / หมายเหตุ</label>
            <input type="text" id="patientNameInput" class="input-field" value="" placeholder="เช่น นายสมชาย ใจดี" oninput="onPatientNameChanged(this.value)" style="border-color:#cbd5e0; font-weight:600;">
          </div>
          <div class="form-group">
            <label>ปริมาตรสารน้ำ (mL)</label>
            <input type="number" id="volumeInput" class="input-field" value="1000" oninput="onInputChanged()">
          </div>
          <div class="form-group">
            <label>เวลาให้สารน้ำ (ชั่วโมง)</label>
            <input type="number" step="0.1" id="hoursInput" class="input-field" value="12.5" oninput="onInputChanged()">
          </div>
          <div class="form-group">
            <label>Drop Factor (gtt/mL)</label>
            <div class="pill-group">
              <button class="pill-btn" onclick="setDropFactor(10, this)">10</button>
              <button class="pill-btn" onclick="setDropFactor(15, this)">15</button>
              <button class="pill-btn active" onclick="setDropFactor(20, this)">20</button>
              <button class="pill-btn" onclick="setDropFactor(60, this)">60</button>
            </div>
          </div>
          <div class="form-group">
            <label>เวลาเริ่มให้สารน้ำ</label>
            <input type="time" id="startTimeInput" class="input-field" value="10:00" onchange="onInputChanged()">
          </div>
          <div class="btn-group">
            <button class="btn btn-primary" onclick="calculateIVRate(true)">🧮 บันทึก & ส่งไป Host</button>
            <button class="btn btn-danger" onclick="resetBedCounter()">🔄 เริ่มถุงใหม่</button>
          </div>
          <div class="sync-note" id="hostCfgNote">ค่าที่ Host ใช้อยู่: -</div>
          <div class="sync-note" id="dirtyNote" style="color:#dd6b20; font-weight:600;"></div>
        </div>

        <div class="card">
          <div class="card-title">
            <span>⏰ คำนวณเวลาที่ IV หมด</span>
            <span style="font-size:13.5px; color:var(--primary);" id="cardBedTag2">[เตียง 1]</span>
          </div>
          <div class="form-group">
            <label>ปริมาณสารน้ำทั้งหมด (mL)</label>
            <input type="number" id="volTotalInput" class="input-field" value="1000" oninput="onInputChanged()">
          </div>
          <div class="form-group">
            <label>Rate ตามออเดอร์ (mL/hr)</label>
            <input type="number" id="orderRateInput" class="input-field" value="80" oninput="onInputChanged()">
          </div>
          <div class="form-group">
            <label>แจ้งเตือนใกล้หมด เมื่อให้ไปแล้ว (% ของปริมาตรตามแผน)</label>
            <input type="number" id="nearPctInput" class="input-field" value="80" min="50" max="95" step="5" oninput="onInputChanged()">
            <div class="near-line" id="nearPctHint">จะเตือนเมื่อให้ไปแล้ว 800 mL</div>
          </div>
          <div class="form-group">
            <label>เริ่มให้เวลา</label>
            <input type="time" id="startTime2Input" class="input-field" value="10:10" onchange="onInputChanged()">
          </div>
          <div class="btn-group">
            <button class="btn btn-primary" onclick="calculateEndTime(true)">🧮 บันทึก Rate & ส่งไป Host</button>
          </div>
          <div style="margin-top: 15px;">
            <div class="highlight-card">
              <div class="sub-text">สารน้ำจะหมดใน</div>
              <div class="big-text" id="durationText">12 ชั่วโมง 30 นาที</div>
              <div style="font-size: 14.5px; font-weight: bold; margin-top: 5px;" id="endTime2Text">หมดเวลา 22:40 น.</div>
            </div>
          </div>
        </div>

        <div class="card">
          <div class="card-title" id="simTitle">💧 จำลองการหยด: Bed / Station 1</div>
          <div class="result-grid">
            <div class="metric-card">
              <div class="title">เป้าหมาย (Target)</div>
              <div class="val" id="gttMinDisplay">26.67</div>
              <div class="sub">gtt/min</div>
            </div>
            <div class="metric-card">
              <div class="title">เวลา/หยด</div>
              <div class="val" id="secPerDropDisplay">2.25</div>
              <div class="sub">วินาที/หยด</div>
            </div>
          </div>
          <div class="chamber-box">
            <div class="iv-tube-top"></div>
            <div class="iv-chamber">
              <div class="iv-nozzle"></div>
              <div class="drop" id="dropElement"></div>
              <div class="iv-pool"></div>
            </div>
            <div class="sim-status-text" id="simStatus">สถานะ: จำลองตามค่าคำนวณ</div>
          </div>
          <div class="btn-group">
            <button class="btn btn-green" onclick="startDrip()">▶️ เริ่มจำลอง</button>
            <button class="btn btn-orange" onclick="stopDrip()">⏸️ หยุด</button>
          </div>
        </div>
      </div>
    </div>

    <div id="tab-graphs" class="tab-content">
      <div class="card">
        <div class="card-title">
          <span>📈 กราฟวิเคราะห์แนวโน้มย้อนหลัง 60 นาที (Offline Native Canvas)</span>
          <span style="font-size: 13.5px; font-weight: normal; color: var(--text-muted);">อัปเดตอัตโนมัติ</span>
        </div>
        <div class="tab-bar" id="graphBedSelectorTabs"></div>
        <div class="result-grid" style="grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); margin-bottom: 18px;">
          <div class="metric-card"><div class="title">Peak Rate</div><div class="val" style="color:#e53e3e;" id="graphPeakRate">0.0</div><div class="sub">mL/hr</div></div>
          <div class="metric-card"><div class="title">Min Rate</div><div class="val" style="color:#3182ce;" id="graphMinRate">0.0</div><div class="sub">mL/hr</div></div>
          <div class="metric-card"><div class="title">Avg Rate</div><div class="val" style="color:#2f855a;" id="graphAvgRate">0.0</div><div class="sub">mL/hr</div></div>
          <div class="metric-card"><div class="title">Total (1 ชม.)</div><div class="val" style="color:#805ad5;" id="graphTotalVol">0.00</div><div class="sub">mL</div></div>
        </div>
        <h3 style="font-size:15.5px; margin-bottom:10px; color:#4a5568; font-weight:700;">⚡ อัตราการไหลสารน้ำ Flow Rate (mL/hr)</h3>
        <div class="chart-container"><canvas id="flowRateCanvas"></canvas></div>
        <h3 style="font-size:15.5px; margin-bottom:10px; color:#4a5568; font-weight:700;">💧 ปริมาณหยดน้ำต่อนาที Drops (drops/min)</h3>
        <div class="chart-container"><canvas id="dropsCanvas"></canvas></div>
      </div>
    </div>

    <div id="tab-logs" class="tab-content">
      <div class="card">
        <div class="card-title">
          <span>📋 ตารางประวัติ 60 นาที & รายงานส่งเวร (Shift Report พร้อมเวลาจริง)</span>
          <button class="btn btn-primary" style="width:auto; padding:9px 18px; font-size:14.5px;" onclick="downloadShiftCSV()">📥 ดาวน์โหลด CSV</button>
        </div>
        <div class="tab-bar" id="logBedSelectorTabs"></div>
        <div class="result-grid" style="grid-template-columns: repeat(auto-fit, minmax(190px, 1fr)); margin-bottom: 15px;">
          <div class="metric-card"><div class="title">หยดรวมใน 60 นาที</div><div class="val" id="logSumDrops">0</div><div class="sub">drops</div></div>
          <div class="metric-card"><div class="title">ปริมาตรรวมใน 60 นาที</div><div class="val" id="logSumVolume">0.00</div><div class="sub">mL</div></div>
          <div class="metric-card"><div class="title">Flow Rate เฉลี่ย</div><div class="val" id="logAvgRate">0.00</div><div class="sub">mL/hr</div></div>
        </div>
        <div class="table-container">
          <table class="log-table">
            <thead>
              <tr>
                <th>ลำดับ</th>
                <th>วัน-เวลาบันทึก (Timestamp)</th>
                <th>หยดในนาทีนั้น</th>
                <th>ปริมาตรรวมสะสม</th>
                <th>Rate เฉลี่ย</th>
                <th>Target</th>
                <th>สถานะแจ้งเตือน</th>
                <th>สัญญาณ (RSSI)</th>
                <th>แบตเตอรี่เตียง</th>
              </tr>
            </thead>
            <tbody id="logTableBody">
              <tr><td colspan="9" style="text-align: center; color: var(--text-muted); padding: 22px;">กำลังรวบรวมข้อมูล...</td></tr>
            </tbody>
          </table>
        </div>
      </div>
    </div>

    <div id="tab-about" class="tab-content">
      <div class="about-header-box">
        <h2>🏥 โครงการวิจัย: ผลของการใช้นวัตกรรม Smart IV Alert ต่อความแม่นยำในการแจ้งเตือนและปริมาณสารน้ำที่ได้รับ</h2>
        <p>
          <b>พื้นที่ศึกษา:</b> หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น อำเภอสูงเม่น จังหวัดแพร่<br>
          <b>ระยะเวลาดำเนินการ:</b> 12 สัปดาห์ (มิถุนายน – กันยายน พ.ศ. 2569)<br>
          <b>สถาบัน:</b> วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
        </p>
      </div>

      <div class="team-grid">
        <div class="team-card">
          <div class="team-card-title">👩‍⚕️ คณะผู้จัดทำงานวิจัย (นักศึกษาพยาบาลศาสตร์)</div>
          <div class="unified-member-grid">
            <div class="member-item"><span>1. น.ส.นภารัตน์ กุลแก้ว (หัวหน้าโครงการ)</span><span class="sid">67130301040</span></div>
            <div class="member-item"><span>2. น.ส.จิราวรรณ ศรีสถาน</span><span class="sid">67130301015</span></div>
            <div class="member-item"><span>3. น.ส.ชญาดา สมศรีษะ</span><span class="sid">67130301019</span></div>
            <div class="member-item"><span>4. น.ส.ชนิสรา แก้วใส</span><span class="sid">67130301022</span></div>
            <div class="member-item"><span>5. น.ส.ชลธาร ฮาดดา</span><span class="sid">67130301024</span></div>
            <div class="member-item"><span>6. น.ส.นริสรา เครือคำมูล</span><span class="sid">67130301041</span></div>
            <div class="member-item"><span>7. น.ส.ปริยากร เจริญสุข</span><span class="sid">67130301046</span></div>
            <div class="member-item"><span>8. น.ส.ปิติพร เหล็กแจ้ง</span><span class="sid">67130301049</span></div>
            <div class="member-item"><span>9. น.ส.พิริสา แก้วกู่</span><span class="sid">67130301061</span></div>
            <div class="member-item"><span>10. น.ส.วรัญญา ผามารถเมือง</span><span class="sid">67130301077</span></div>
            <div class="member-item"><span>11. น.ส.อมรลดา นาขาม</span><span class="sid">67130301106</span></div>
            <div class="member-item"><span>12. น.ส.อารีรัตน์ จิตต์อารีย์</span><span class="sid">67130301110</span></div>
          </div>
        </div>

        <div class="team-card">
          <div class="team-card-title">🎓 อาจารย์ที่ปรึกษา & ฝ่ายวิศวกรรม</div>
          <div style="font-size:14px; line-height:1.7; color:#2d3748;">
            <b>อาจารย์ที่ปรึกษาโครงการวิจัย:</b><br>
            • <b>ดร.กรรณิการ์ กาศสมบูรณ์</b><br>
            <span style="color:var(--text-muted); font-size:12.5px;">รองผู้อำนวยการฝ่ายกิจการนักศึกษา<br>วิทยาลัยพยาบาลบรมราชชนนี แพร่</span>
            <hr style="border:0; border-top:1px dashed var(--border-color); margin:12px 0;">
            <b>ฝ่ายพัฒนาระบบวิศวกรรม & โครงข่าย IoT:</b><br>
            • <b>นายกิตติพันธ์ รัตนคร</b> (นักวิชาการคอมพิวเตอร์)<br>
            <span style="color:var(--text-muted); font-size:12.5px;">มหาวิทยาลัยมหาจุฬาลงกรณราชวิทยาลัย วิทยาเขตแพร่</span>
          </div>
        </div>
      </div>
    </div>
  </div>

  <div class="modal" id="apModal">
    <div class="modal-content">
      <h3 style="margin-bottom: 14px; font-size:17.5px;">📶 จุดเชื่อมต่อของเครื่อง Host</h3>
      <div style="margin-bottom: 15px; background:#f7fafc; padding:12px; border-radius:10px; font-size:14.5px;" id="apDetailBox">กำลังโหลด...</div>
      <div style="background:#fffaf0; border-left:4px solid #dd6b20; padding:11px 13px; border-radius:8px; font-size:13.5px; line-height:1.65;">
        เครื่องนี้ทำงานเป็นจุดเชื่อมต่อในตัว <b>ไม่ต้องต่อ Wi-Fi ของโรงพยาบาลหรืออินเทอร์เน็ต</b><br>
        ให้มือถือ/แท็บเล็ตเชื่อมต่อ Wi-Fi ชื่อข้างบน แล้วเปิด <b>http://192.168.4.1</b><br>
        หากมือถือแจ้งว่า "ไม่มีอินเทอร์เน็ต" ให้เลือก <b>ใช้งานต่อ / ยังคงเชื่อมต่อ</b>
      </div>
      <div class="btn-group" style="margin-top:14px;"><button class="btn btn-secondary" onclick="closeApModal()">ปิด</button></div>
    </div>
  </div>

  <script>
    let activeBedCount = 5;
    let maxSupportedBeds = 8;

    const generateDefaultConfigs = () => {
      const arr = [];
      for (let i = 1; i <= maxSupportedBeds; i++) {
        arr.push({
          id: i,
          patientName: '',
          volume: 1000,
          hours: 12.5,
          dropFactor: 20,
          startTime: "10:00",
          volTotal: 1000,
          orderRate: 80,
          startTime2: "10:10",
          gttMin: 26.67,
          secPerDrop: 2.25,
          durationText: "12 ชั่วโมง 30 นาที",
          endTime2Text: "หมดเวลา 22:40 น.",
          nearPct: 80
        });
      }
      return arr;
    };

    let bedConfigs = JSON.parse(localStorage.getItem('iv_bed_configs_v460')) || generateDefaultConfigs();
    function saveBedConfigs() { localStorage.setItem('iv_bed_configs_v460', JSON.stringify(bedConfigs)); }

    let selectedStation = 1;
    let activeLogTab = 1;
    let activeGraphStation = 1;
    let audioMuted = true;
    let audioCtx = null;
    let nameSyncTimer = null;
    let skipHostSyncUntil = 0;
    let lastHostData = null;
    const dirtyBeds = new Set();   // เตียงที่แก้ค่าในฟอร์มแต่ยังไม่ได้ส่งไป Host

    function updateNearPctHint() {
      const cfg = bedConfigs[selectedStation - 1];
      const el = document.getElementById('nearPctHint');
      if (!el || !cfg) return;
      const pct = cfg.nearPct || 80;
      el.innerText = cfg.volTotal > 0
        ? `จะเตือนเมื่อให้ไปแล้ว ${Math.round(cfg.volTotal * pct / 100)} mL (เหลือ ${Math.round(cfg.volTotal * (100 - pct) / 100)} mL)` +
          (cfg.orderRate > 0 ? ` ประมาณ ${Math.round(cfg.volTotal * (100 - pct) / 100 / cfg.orderRate * 60)} นาทีก่อนหมด` : '')
        : 'ยังไม่ได้ตั้งปริมาตรตามแผน';
    }

    function acknowledgeNearEnd(id, ev) {
      if (ev) ev.stopPropagation();
      fetch('/api/stations/ack', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `station=${id}`
      }).catch(e => alert('ส่งการรับทราบไม่สำเร็จ: ' + e.message));
    }

    function playNearEndChime() {
      if (audioMuted) return;
      try {
        if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        [0, 0.25, 0.5].forEach(t => {
          const osc = audioCtx.createOscillator(), gain = audioCtx.createGain();
          osc.frequency.value = 1046;
          gain.gain.setValueAtTime(0.15, audioCtx.currentTime + t);
          gain.gain.exponentialRampToValueAtTime(0.001, audioCtx.currentTime + t + 0.18);
          osc.connect(gain); gain.connect(audioCtx.destination);
          osc.start(audioCtx.currentTime + t); osc.stop(audioCtx.currentTime + t + 0.2);
        });
      } catch (e) { console.log(e); }
    }
    const nearChimed = {};

    function updateDirtyNote() {
      const el = document.getElementById('dirtyNote');
      if (el) el.innerText = dirtyBeds.has(selectedStation) ? '⚠️ มีค่าที่แก้ไขแต่ยังไม่ได้ส่งไป Host — กดปุ่ม "บันทึก ... ส่งไป Host"' : '';
    }

    const ALERT_META = {
      0: { text: '● ปกติ',               cls: 'badge-online', card: '',            alarm: false },
      1: { text: '⚠️ ไหลเร็วเกิน',        cls: 'badge-alarm',  card: 'alarm-state', alarm: true  },
      2: { text: '⚠️ ไหลช้าเกิน',         cls: 'badge-alarm',  card: 'alarm-state', alarm: true  },
      3: { text: '⏳ ใกล้หมด เตรียมถุงใหม่', cls: 'badge-warn',   card: 'warn-state',  alarm: false },
      4: { text: '🛑 ให้ครบตามแผนแล้ว',   cls: 'badge-alarm',  card: 'alarm-state', alarm: true  },
      5: { text: '⚠️ สายพับ/หยุดไหล',     cls: 'badge-alarm',  card: 'alarm-state', alarm: true  },
      6: { text: '🔍 เซนเซอร์ยังไม่จับหยด', cls: 'badge-alarm',  card: 'alarm-state', alarm: true  }
    };

    // ข้อความสั้น ๆ สำหรับแถบแจ้งเตือนรวม — ให้พยาบาลอ่านแวบเดียวรู้ว่าต้องไปทำอะไร
    const ALERT_SHORT = {
      1: 'ไหลเร็วเกิน', 2: 'ไหลช้าเกิน', 3: 'ใกล้หมด',
      4: 'ให้ครบแล้ว', 5: 'ไม่ไหล / สายพับ', 6: 'เซนเซอร์ไม่จับหยด'
    };

    function escapeHtml(str) {
      return String(str ?? '').replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
    }

    function recalcDerived(cfg) {
      cfg.gttMin = cfg.orderRate > 0 ? (cfg.orderRate * cfg.dropFactor) / 60 : 0;
      cfg.secPerDrop = cfg.gttMin > 0 ? (60 / cfg.gttMin) : 0;
    }

    function pushBedConfig(id, showAlert = false) {
      const cfg = bedConfigs[id - 1];
      skipHostSyncUntil = Date.now() + 3000;
      const body = new URLSearchParams({
        station: id,
        rate: (cfg.orderRate || 0).toFixed(1),
        volume: (cfg.volTotal || 0).toFixed(0),
        df: cfg.dropFactor,
        nearpct: cfg.nearPct || 80,
        name: cfg.patientName || ''
      });
      return fetch('/api/stations/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: body.toString()
      }).then(r => {
        if (!r.ok) return r.text().then(t => { throw new Error(t || ('HTTP ' + r.status)); });
        dirtyBeds.delete(id);
        updateDirtyNote();
        if (showAlert) alert(`ส่งค่าเตียง ${id} ไปยัง Host เรียบร้อยแล้ว\nRate ${cfg.orderRate.toFixed(1)} mL/hr | แผน ${cfg.volTotal} mL | ${cfg.dropFactor} gtt/mL\nเตือนใกล้หมดที่ ${cfg.nearPct || 80}%`);
      }).catch(e => alert('ส่งค่าไปยัง Host ไม่สำเร็จ: ' + e.message));
    }

    function resetBedCounter() {
      const id = selectedStation;
      if (!confirm(`เริ่มถุงใหม่สำหรับเตียง ${id}?\n\nระบบจะรีเซ็ตจำนวนหยด ปริมาตรสะสม และประวัติ 60 นาทีของเตียงนี้ ทั้งที่ Host และเครื่องประจำเตียง`)) return;
      fetch('/api/stations/reset', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `station=${id}`
      }).then(r => {
        if (!r.ok) throw new Error('HTTP ' + r.status);
        alert(`รีเซ็ตเตียง ${id} แล้ว (เครื่องประจำเตียงจะรีเซ็ตภายใน 1-2 วินาที)`);
        if (document.getElementById('tab-logs').classList.contains('active')) fetchLogs();
      }).catch(e => alert('รีเซ็ตไม่สำเร็จ: ' + e.message));
    }

    // Host เป็นแหล่งข้อมูลหลักของ Rate / แผน / Drop factor / ชื่อผู้ป่วย
    function syncConfigsFromHost(stationsData) {
      if (Date.now() < skipHostSyncUntil) return;
      const ae = document.activeElement;
      const editing = ae && (ae.tagName === 'INPUT' || ae.tagName === 'SELECT');
      let needTabs = false;
      stationsData.forEach(st => {
        const cfg = bedConfigs[st.id - 1];
        if (!cfg) return;
        if (editing && st.id === selectedStation) return;
        if (dirtyBeds.has(st.id)) return;
        let changed = false;
        if (Math.abs((cfg.orderRate || 0) - st.targetRate) > 0.05) { cfg.orderRate = st.targetRate; changed = true; }
        if (Math.abs((cfg.volTotal || 0) - st.planVolume) > 0.5) { cfg.volTotal = st.planVolume; changed = true; }
        if (cfg.dropFactor !== st.dropFactor) { cfg.dropFactor = st.dropFactor; changed = true; }
        if ((cfg.nearPct || 80) !== st.nearEndPct) { cfg.nearPct = st.nearEndPct; changed = true; }
        if ((cfg.patientName || '') !== (st.patientName || '')) { cfg.patientName = st.patientName || ''; changed = true; needTabs = true; }
        if (changed) {
          recalcDerived(cfg);
          if (st.id === selectedStation) loadBedToForm(selectedStation);
        }
      });
      if (needTabs) renderTabs();
      saveBedConfigs();
    }

    function updateHostCfgNote() {
      const el = document.getElementById('hostCfgNote');
      if (!el || !lastHostData) return;
      const st = lastHostData.stations.find(x => x.id === selectedStation);
      if (!st) { el.innerText = 'ค่าที่ Host ใช้อยู่: -'; return; }
      el.innerHTML = `ค่าที่ Host ใช้อยู่ (เตียง ${st.id}): <b>${st.targetRate} mL/hr</b> | แผน <b>${st.planVolume} mL</b> | <b>${st.dropFactor} gtt/mL</b> | เตือนใกล้หมด <b>${st.nearEndPct}%</b>` +
                     (st.caseStart ? `<br>เริ่มถุงปัจจุบัน: ${escapeHtml(st.caseStart)}` : '');
    }

    // (4) เสียงเตือนแบบกระตุ้นความสนใจ
    // เดิมเป็นเสียงไซน์ 880 Hz ครั้งเดียว 0.5 วินาที ซึ่งกลืนไปกับเสียงในหอผู้ป่วย
    // ของใหม่เป็นชุด 3 พัลส์สลับสองความถี่ (เสียงแบบรถพยาบาล) ใช้คลื่นสี่เหลี่ยม
    // ซึ่งมีฮาร์มอนิกมาก จึงแทรกผ่านเสียงรบกวนได้ดีกว่าเสียงไซน์มาก
    let lastAlarmBurst = 0;
    function playAlarmTone() {
      if (audioMuted) return;
      const now = Date.now();
      if (now - lastAlarmBurst < 2000) return;   // ปล่อยชุดเสียงทุก 2 วินาที ไม่ให้ซ้อนกัน
      lastAlarmBurst = now;
      try {
        if (!audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        if (audioCtx.state === 'suspended') audioCtx.resume();
        const t0 = audioCtx.currentTime;
        const pattern = [
          { at: 0.00, f: 1320, d: 0.16 },
          { at: 0.20, f:  990, d: 0.16 },
          { at: 0.40, f: 1320, d: 0.16 },
          { at: 0.60, f:  990, d: 0.22 }
        ];
        pattern.forEach(p => {
          const osc  = audioCtx.createOscillator();
          const gain = audioCtx.createGain();
          osc.type = 'square';
          osc.frequency.setValueAtTime(p.f, t0 + p.at);
          gain.gain.setValueAtTime(0.0001, t0 + p.at);
          gain.gain.exponentialRampToValueAtTime(0.32, t0 + p.at + 0.012);   // ขึ้นเร็ว = สะดุดหู
          gain.gain.setValueAtTime(0.32, t0 + p.at + p.d - 0.03);
          gain.gain.exponentialRampToValueAtTime(0.0001, t0 + p.at + p.d);
          osc.connect(gain); gain.connect(audioCtx.destination);
          osc.start(t0 + p.at);
          osc.stop(t0 + p.at + p.d + 0.02);
        });
      } catch (e) { console.log(e); }
    }

    function toggleAudioMute() {
      audioMuted = !audioMuted;
      document.getElementById('audioToggleBtn').innerText = audioMuted ? '🔕 ปิดเสียงเตือน' : '🔔 เปิดเสียงเตือน';
      if (!audioMuted && !audioCtx) audioCtx = new (window.AudioContext || window.webkitAudioContext)();
    }

    function switchMainTab(tabId, btnEl) {
      document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
      document.querySelectorAll('.nav-btn').forEach(el => el.classList.remove('active'));
      document.getElementById(tabId).classList.add('active');
      btnEl.classList.add('active');

      if (tabId === 'tab-graphs') renderGraphs();
      else if (tabId === 'tab-logs') fetchLogs();
    }

    function changeBedScale(delta) {
      const newCount = activeBedCount + delta;
      if (newCount < 1 || newCount > maxSupportedBeds) {
        alert(`สามารถตั้งจำนวนเตียงได้ระหว่าง 1 ถึง ${maxSupportedBeds} เตียง`);
        return;
      }
      fetch(`/api/stations/set?count=${newCount}`, { method: 'POST' })
        .then(res => {
          if (res.ok) {
            activeBedCount = newCount;
            document.getElementById('activeBedCountLabel').innerText = activeBedCount;
            if (selectedStation > activeBedCount) selectedStation = 1;
            if (activeGraphStation > activeBedCount) activeGraphStation = 1;
            if (activeLogTab > activeBedCount) activeLogTab = 1;
            renderTabs();
            loadBedToForm(selectedStation);
          }
        });
    }

    function renderTabs() {
      const calcBox = document.getElementById('calcBedSelectorTabs');
      calcBox.innerHTML = '';
      for (let i = 0; i < activeBedCount; i++) {
        const b = bedConfigs[i];
        const isAct = (b.id === selectedStation) ? 'active' : '';
        calcBox.innerHTML += `<div class="tab-item ${isAct}" onclick="selectStation(${b.id})">🛏️ เตียง ${b.id}: ${escapeHtml(b.patientName) || 'ว่าง'}</div>`;
      }

      const graphBox = document.getElementById('graphBedSelectorTabs');
      graphBox.innerHTML = '';
      for (let i = 0; i < activeBedCount; i++) {
        const b = bedConfigs[i];
        const isAct = (b.id === activeGraphStation) ? 'active' : '';
        graphBox.innerHTML += `<div class="tab-item ${isAct}" onclick="switchGraphStation(${b.id}, this)">🛏️ เตียง ${b.id}: ${escapeHtml(b.patientName) || 'ว่าง'}</div>`;
      }

      const logBox = document.getElementById('logBedSelectorTabs');
      logBox.innerHTML = '';
      for (let i = 0; i < activeBedCount; i++) {
        const b = bedConfigs[i];
        const isAct = (b.id === activeLogTab) ? 'active' : '';
        logBox.innerHTML += `<div class="tab-item ${isAct}" onclick="switchLogTab(${b.id}, this)">🛏️ เตียง ${b.id}: ${escapeHtml(b.patientName) || 'ว่าง'}</div>`;
      }
    }

    function onPatientNameChanged(val) {
      bedConfigs[selectedStation - 1].patientName = val;
      saveBedConfigs();
      renderTabs();
      updateBedTitles();
      clearTimeout(nameSyncTimer);
      const sid = selectedStation;
      skipHostSyncUntil = Date.now() + 3000;
      nameSyncTimer = setTimeout(() => pushBedConfig(sid, false), 1000);
    }

    function updateBedTitles() {
      const cfg = bedConfigs[selectedStation - 1];
      const pName = cfg.patientName ? `(${cfg.patientName})` : '';
      document.getElementById('activeBedBadgeText').innerText = `เตียง ${selectedStation} ${pName}`;
      document.getElementById('cardBedTag1').innerText = `[เตียง ${selectedStation} ${pName}]`;
      document.getElementById('cardBedTag2').innerText = `[เตียง ${selectedStation} ${pName}]`;
      document.getElementById('simTitle').innerText = `💧 จำลองการหยด: เตียง ${selectedStation} ${pName}`;
    }

    function loadBedToForm(id) {
      const cfg = bedConfigs[id - 1];
      document.getElementById('patientNameInput').value = cfg.patientName || '';
      document.getElementById('volumeInput').value = cfg.volume;
      document.getElementById('hoursInput').value = cfg.hours;
      document.getElementById('startTimeInput').value = cfg.startTime;
      document.getElementById('volTotalInput').value = cfg.volTotal;
      document.getElementById('orderRateInput').value = cfg.orderRate;
      document.getElementById('startTime2Input').value = cfg.startTime2;
      document.getElementById('nearPctInput').value = cfg.nearPct || 80;
      updateNearPctHint();

      document.querySelectorAll('.pill-btn').forEach(btn => {
        if (parseInt(btn.innerText) === cfg.dropFactor) btn.classList.add('active');
        else btn.classList.remove('active');
      });

      document.getElementById('gttMinDisplay').innerText = cfg.gttMin.toFixed(2);
      document.getElementById('secPerDropDisplay').innerText = cfg.secPerDrop.toFixed(2);
      document.getElementById('durationText').innerText = cfg.durationText;
      document.getElementById('endTime2Text').innerText = cfg.endTime2Text;

      updateBedTitles();
      renderTabs();
      updateHostCfgNote();
      updateDirtyNote();

      if (document.getElementById('dropElement').classList.contains('animate')) startDrip();
    }

    function onInputChanged() {
      const cfg = bedConfigs[selectedStation - 1];
      cfg.volume = parseFloat(document.getElementById('volumeInput').value) || 0;
      cfg.hours = parseFloat(document.getElementById('hoursInput').value) || 0;
      cfg.startTime = document.getElementById('startTimeInput').value;
      cfg.volTotal = parseFloat(document.getElementById('volTotalInput').value) || 0;
      cfg.orderRate = parseFloat(document.getElementById('orderRateInput').value) || 0;
      cfg.startTime2 = document.getElementById('startTime2Input').value;
      cfg.nearPct = Math.min(95, Math.max(50, parseInt(document.getElementById('nearPctInput').value) || 80));
      updateNearPctHint();
      dirtyBeds.add(selectedStation);
      updateDirtyNote();
      saveBedConfigs();
    }

    function setDropFactor(val, btn) {
      bedConfigs[selectedStation - 1].dropFactor = val;
      document.querySelectorAll('.pill-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      calculateIVRate(false);
      dirtyBeds.add(selectedStation);
      updateDirtyNote();
    }

    function calculateIVRate(save = false) {
      onInputChanged();
      const cfg = bedConfigs[selectedStation - 1];
      if (cfg.volume <= 0 || cfg.hours <= 0) {
        if (save) alert('กรุณากรอกปริมาตรและเวลาให้สารน้ำให้ถูกต้อง');
        return;
      }

      const totalMins = cfg.hours * 60;
      cfg.gttMin = (cfg.volume * cfg.dropFactor) / totalMins;
      cfg.secPerDrop = cfg.gttMin > 0 ? (60 / cfg.gttMin) : 0;

      document.getElementById('gttMinDisplay').innerText = cfg.gttMin.toFixed(2);
      document.getElementById('secPerDropDisplay').innerText = cfg.secPerDrop.toFixed(2);

      if (save) {
        cfg.orderRate = Math.round((cfg.volume / cfg.hours) * 10) / 10;
        cfg.volTotal = cfg.volume;
        document.getElementById('orderRateInput').value = cfg.orderRate;
        document.getElementById('volTotalInput').value = cfg.volTotal;
        if (!document.getElementById('startTime2Input').value) document.getElementById('startTime2Input').value = cfg.startTime;
        calculateEndTime(false);
        pushBedConfig(selectedStation, true);
      }
      saveBedConfigs();

      if (document.getElementById('dropElement').classList.contains('animate')) startDrip();
    }

    function calculateEndTime(save = false) {
      onInputChanged();
      const cfg = bedConfigs[selectedStation - 1];
      if (cfg.volTotal <= 0 || cfg.orderRate <= 0) {
        if (save) alert('กรุณากรอกปริมาณสารน้ำและ Rate ให้ถูกต้อง');
        return;
      }

      const durationHours = cfg.volTotal / cfg.orderRate;
      const totalMinsAll = Math.round(durationHours * 60);
      const h = Math.floor(totalMinsAll / 60);
      const m = totalMinsAll % 60;
      cfg.durationText = `${h} ชั่วโมง ${m} นาที`;

      if (cfg.startTime2) {
        const [stH, stM] = cfg.startTime2.split(':').map(Number);
        const endTotalMins = (stH * 60 + stM + totalMinsAll) % 1440;
        const endH = String(Math.floor(endTotalMins / 60)).padStart(2, '0');
        const endM = String(endTotalMins % 60).padStart(2, '0');
        cfg.endTime2Text = `หมดเวลา ${endH}:${endM} น.`;
      }

      recalcDerived(cfg);
      document.getElementById('gttMinDisplay').innerText = cfg.gttMin.toFixed(2);
      document.getElementById('secPerDropDisplay').innerText = cfg.secPerDrop.toFixed(2);
      document.getElementById('durationText').innerText = cfg.durationText;
      document.getElementById('endTime2Text').innerText = cfg.endTime2Text;
      saveBedConfigs();

      if (save) pushBedConfig(selectedStation, true);
    }

    function startDrip() {
      const cfg = bedConfigs[selectedStation - 1];
      const dropEl = document.getElementById('dropElement');
      dropEl.classList.remove('animate');
      void dropEl.offsetWidth;
      dropEl.style.animationDuration = `${cfg.secPerDrop}s`;
      dropEl.classList.add('animate');
      document.getElementById('simStatus').innerText = `เตียง ${selectedStation}: หยดทุก ๆ ${cfg.secPerDrop.toFixed(2)} วินาที`;
    }

    function stopDrip() {
      document.getElementById('dropElement').classList.remove('animate');
      document.getElementById('simStatus').innerText = "สถานะ: หยุดจำลอง";
    }

    function selectStation(id) {
      selectedStation = id;
      document.querySelectorAll('.station-card').forEach(c => c.classList.remove('selected'));
      const activeCard = document.getElementById(`st-card-${id}`);
      if (activeCard) activeCard.classList.add('selected');
      loadBedToForm(id);
    }

    function getSignalMeta(rssi, isOnline) {
      if (!isOnline) return { lvl: 0, text: 'Offline', color: '#a0aec0' };
      if (rssi >= -60) return { lvl: 4, text: `${rssi} dBm (ดีเยี่ยม)`, color: '#38a169' };
      if (rssi >= -70) return { lvl: 3, text: `${rssi} dBm (ดี)`, color: '#3182ce' };
      if (rssi >= -80) return { lvl: 2, text: `${rssi} dBm (ปานกลาง)`, color: '#dd6b20' };
      return { lvl: 1, text: `${rssi} dBm (อ่อน)`, color: '#e53e3e' };
    }

    function renderStations(stationsData) {
      const container = document.getElementById('stationGridContainer');
      if (container.querySelector('.ack-btn:hover, .ack-btn:active')) return;   // ไม่วาดทับขณะกำลังกดปุ่มรับทราบ
      container.innerHTML = '';
      let hasAlarm = false;
      const critList = [];   // เตียงที่ต้องไปดูเดี๋ยวนี้
      const watchList = [];  // เตียงที่ใกล้หมด (เฝ้าดู ไม่ใช่เหตุวิกฤต)

      stationsData.forEach(st => {
        const isSel = (st.id === selectedStation) ? 'selected' : '';
        const targetRate = st.targetRate;
        const targetGtt = targetRate > 0 ? ((targetRate * st.dropFactor) / 60).toFixed(1) : '-';
        const isPaused = st.online && !st.running;
        const meta = ALERT_META[st.alertCode] || ALERT_META[0];

        let statusBadge = '';
        let cardState = '';
        let isAlarm = false;

        if (!st.online) {
          statusBadge = '<span class="badge-status badge-offline">○ Offline</span>';
        } else if (isPaused) {
          statusBadge = '<span class="badge-status badge-paused">⏸️ หยุดชั่วคราว</span>';
          cardState = 'paused-state';
        } else {
          statusBadge = `<span class="badge-status ${meta.cls}">${meta.text}</span>`;
          // (1) ปกติ = กรอบสีเขียว / มีเหตุ = ใช้สีของเหตุนั้น
          cardState = meta.card || 'normal-state';
          isAlarm = meta.alarm;
        }
        if (isAlarm) {
          hasAlarm = true;
          critList.push(`<span class="ab-item">🛏️ <b>เตียง ${st.id}</b> — ${ALERT_SHORT[st.alertCode] || 'ต้องตรวจสอบ'}</span>`);
        } else if (st.online && st.running && st.alertCode === 3) {
          watchList.push(`เตียง ${st.id}`);
        }
        const nearPending = st.online && st.running && st.alertCode === 3 && !st.nearEndAck;
        if (nearPending && !nearChimed[st.id]) { nearChimed[st.id] = true; playNearEndChime(); }
        if (!nearPending) nearChimed[st.id] = false;
        // (2) ใกล้หมด: บอกเป็น "% ของสารน้ำ" แทนตัวเลข mL และบอกว่าเหลือเท่าไหร่ในกระปุก
        const givenPct  = st.planVolume > 0 ? Math.min(100, (st.volumeMl / st.planVolume) * 100) : 0;
        const remainMl  = st.planVolume > 0 ? Math.max(0, st.planVolume - st.volumeMl) : 0;
        const nearHtml = st.planVolume > 0
          ? (st.alertCode === 3
              ? `<div class="near-line">🔔 ใกล้หมดแล้ว — ให้ไปแล้ว <b>${givenPct.toFixed(0)}% ของสารน้ำ</b> · เหลือในกระปุก <b>${remainMl.toFixed(0)} mL</b>${st.nearEndAck ? ' — รับทราบแล้ว' : ''}</div>`
              : `<div class="near-line">🔔 จะเตือนเตรียมถุงใหม่เมื่อให้ไปแล้ว <b>${st.nearEndPct}% ของสารน้ำ</b></div>`)
          : '';
        const ackHtml = nearPending
          ? `<button class="ack-btn" onclick="acknowledgeNearEnd(${st.id}, event)">✅ รับทราบ เตรียมถุงใหม่ให้เตียง ${st.id}</button>`
          : '';

        const sig = getSignalMeta(st.rssi, st.online);
        const batText = st.battery > 0.5
          ? `🔋 ${st.battery.toFixed(2)}V (${Math.min(100, Math.max(0, Math.round(((st.battery - 3.2) / (4.2 - 3.2)) * 100)))}%)`
          : '🔋 N/A';
        const progress = st.planVolume > 0 ? Math.min(100, (st.volumeMl / st.planVolume) * 100) : 0;
        const planText = st.planVolume > 0 ? ` / ${st.planVolume} mL (${progress.toFixed(0)}%)` : ' mL (ยังไม่ตั้งแผน)';

        const card = document.createElement('div');
        card.className = `station-card ${isSel} ${cardState}`;
        card.id = `st-card-${st.id}`;
        card.onclick = () => selectStation(st.id);

        card.innerHTML = `
          <div class="station-header">
            <div>
              <div class="station-title">🛏️ เตียง ${st.id}</div>
              <div class="patient-label">👤 ${escapeHtml(st.patientName) || 'ระบุชื่อคนไข้'}</div>
            </div>
            ${statusBadge}
          </div>
          <div class="telemetry-row">
            <span style="color:${sig.color}; font-weight:600;">📶 ${sig.text}</span>
            <span style="font-weight:600; color:#4a5568;">${batText}</span>
            <div class="signal-bars lvl-${sig.lvl}">
              <div class="bar b1"></div>
              <div class="bar b2"></div>
              <div class="bar b3"></div>
              <div class="bar b4"></div>
            </div>
          </div>
          <div class="stat-row">
            <span>หยดสะสม:</span>
            <span class="val">${st.totalDrops} drops</span>
          </div>
          <div class="stat-row">
            <span>ปริมาตรให้แล้ว:</span>
            <span class="val" style="color:#2b6cb0;">${st.volumeMl.toFixed(1)}${planText}</span>
          </div>
          <div class="stat-row">
            <span>เหลือในกระปุก:</span>
            <span class="val" style="color:${remainMl > 0 && remainMl <= 100 ? '#c05621' : '#2d3748'};">
              ${st.planVolume > 0 ? remainMl.toFixed(0) + ' mL' : '-'}
            </span>
          </div>
          <div class="progress-track"><div class="progress-fill" style="width:${progress}%"></div></div>
          ${nearHtml}
          <div class="stat-row">
            <span>Actual Rate:</span>
            <span class="val" style="color:${isAlarm ? '#e53e3e' : (isPaused ? '#d97706' : '#2f855a')};">
              ${isPaused ? '0.0 (PAUSED)' : st.flowRateHr.toFixed(1) + ' mL/hr'}
            </span>
          </div>
          <div class="compare-box">
            <span>Target: <b>${targetGtt} gtt/min</b> (${targetRate > 0 ? targetRate + ' mL/hr' : 'ยังไม่ตั้ง'})</span>
            <span>${st.dropFactor} gtt/mL</span>
          </div>
          ${ackHtml}
        `;
        container.appendChild(card);
      });

      // (3) แถบแจ้งเตือนรวมบน Live Monitor — บอกชัดว่าเตียงไหนเป็นอะไร
      const banner = document.getElementById('alertBanner');
      const bannerList = document.getElementById('alertBannerList');
      if (critList.length) {
        bannerList.innerHTML = critList.join('');
        banner.classList.add('show');
      } else {
        banner.classList.remove('show');
      }

      const watchEl = document.getElementById('watchBanner');
      if (watchList.length) {
        watchEl.innerHTML = `⏳ ใกล้หมด เตรียมถุงใหม่ให้ ${watchList.join(' · ')}`;
        watchEl.classList.add('show');
      } else {
        watchEl.classList.remove('show');
      }

      if (hasAlarm) playAlarmTone();
    }

    function switchGraphStation(stationId, tabEl) {
      activeGraphStation = stationId;
      renderTabs();
      renderGraphs();
    }

    function drawLineChart(canvasId, labels, dataPoints, unit, lineColor, fillColor) {
      const canvas = document.getElementById(canvasId);
      if (!canvas) return;
      const ctx = canvas.getContext('2d');
      const dpr = window.devicePixelRatio || 1;
      const rect = canvas.getBoundingClientRect();
      canvas.width = rect.width * dpr;
      canvas.height = rect.height * dpr;
      ctx.scale(dpr, dpr);

      const w = rect.width, h = rect.height;
      ctx.clearRect(0, 0, w, h);
      const padL = 48, padR = 20, padT = 28, padB = 32;
      const plotW = w - padL - padR, plotH = h - padT - padB;

      if (dataPoints.length === 0) {
        ctx.fillStyle = '#a0aec0';
        ctx.font = '14.5px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText('ยังไม่มีข้อมูลประวัติย้อนหลังสำหรับเตียงนี้', w / 2, h / 2);
        return;
      }

      const maxVal = Math.max(...dataPoints, 10) * 1.15;
      const minVal = 0;

      ctx.strokeStyle = '#e2e8f0';
      ctx.lineWidth = 1;
      ctx.fillStyle = '#718096';
      ctx.font = '11px sans-serif';
      ctx.textAlign = 'right';

      const ySteps = 4;
      for (let i = 0; i <= ySteps; i++) {
        const val = minVal + ((maxVal - minVal) / ySteps) * i;
        const y = padT + plotH - (plotH / ySteps) * i;
        ctx.beginPath(); ctx.moveTo(padL, y); ctx.lineTo(w - padR, y); ctx.stroke();
        ctx.fillText(val.toFixed(0), padL - 6, y + 4);
      }

      const n = dataPoints.length;
      const stepX = n > 1 ? plotW / (n - 1) : plotW;
      const points = [];

      for (let i = 0; i < n; i++) {
        const x = padL + (n === 1 ? plotW / 2 : i * stepX);
        const y = padT + plotH - ((dataPoints[i] - minVal) / (maxVal - minVal)) * plotH;
        points.push({ x, y, val: dataPoints[i], label: labels[i] });
      }

      if (points.length > 1) {
        const grad = ctx.createLinearGradient(0, padT, 0, padT + plotH);
        grad.addColorStop(0, fillColor);
        grad.addColorStop(1, 'rgba(255, 255, 255, 0)');
        ctx.beginPath();
        ctx.moveTo(points[0].x, padT + plotH);
        for (let pt of points) ctx.lineTo(pt.x, pt.y);
        ctx.lineTo(points[points.length - 1].x, padT + plotH);
        ctx.closePath();
        ctx.fillStyle = grad;
        ctx.fill();
      }

      ctx.beginPath();
      ctx.strokeStyle = lineColor;
      ctx.lineWidth = 2.5;
      ctx.moveTo(points[0].x, points[0].y);
      for (let i = 1; i < points.length; i++) ctx.lineTo(points[i].x, points[i].y);
      ctx.stroke();

      points.forEach((pt, idx) => {
        ctx.beginPath();
        ctx.arc(pt.x, pt.y, 4, 0, Math.PI * 2);
        ctx.fillStyle = '#ffffff';
        ctx.fill();
        ctx.lineWidth = 2;
        ctx.strokeStyle = lineColor;
        ctx.stroke();

        if (idx === 0 || idx === points.length - 1 || idx % 10 === 0) {
          ctx.fillStyle = '#718096';
          ctx.textAlign = 'center';
          ctx.font = '11px sans-serif';
          ctx.fillText(`-${pt.label}m`, pt.x, h - 10);
        }
      });
    }

    function drawBarChart(canvasId, labels, dataPoints, barColor) {
      const canvas = document.getElementById(canvasId);
      if (!canvas) return;
      const ctx = canvas.getContext('2d');
      const dpr = window.devicePixelRatio || 1;
      const rect = canvas.getBoundingClientRect();
      canvas.width = rect.width * dpr;
      canvas.height = rect.height * dpr;
      ctx.scale(dpr, dpr);

      const w = rect.width, h = rect.height;
      ctx.clearRect(0, 0, w, h);
      const padL = 48, padR = 20, padT = 28, padB = 32;
      const plotW = w - padL - padR, plotH = h - padT - padB;

      if (dataPoints.length === 0) {
        ctx.fillStyle = '#a0aec0';
        ctx.font = '14.5px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText('ยังไม่มีข้อมูลหยดน้ำสำหรับเตียงนี้', w / 2, h / 2);
        return;
      }

      const maxVal = Math.max(...dataPoints, 10) * 1.15;
      const ySteps = 4;
      ctx.strokeStyle = '#e2e8f0';
      ctx.lineWidth = 1;
      ctx.fillStyle = '#718096';
      ctx.font = '11px sans-serif';
      ctx.textAlign = 'right';

      for (let i = 0; i <= ySteps; i++) {
        const val = (maxVal / ySteps) * i;
        const y = padT + plotH - (plotH / ySteps) * i;
        ctx.beginPath(); ctx.moveTo(padL, y); ctx.lineTo(w - padR, y); ctx.stroke();
        ctx.fillText(val.toFixed(0), padL - 6, y + 4);
      }

      const n = dataPoints.length;
      const barWidth = Math.max(2, (plotW / n) * 0.7);
      const gap = plotW / n;
      ctx.fillStyle = barColor;

      for (let i = 0; i < n; i++) {
        const barH = (dataPoints[i] / maxVal) * plotH;
        const x = padL + i * gap + (gap - barWidth) / 2;
        const y = padT + plotH - barH;
        ctx.fillRect(x, y, barWidth, barH);

        if (i === 0 || i === n - 1 || i % 10 === 0) {
          ctx.fillStyle = '#718096';
          ctx.textAlign = 'center';
          ctx.font = '11px sans-serif';
          ctx.fillText(`-${labels[i]}m`, x + barWidth / 2, h - 10);
          ctx.fillStyle = barColor;
        }
      }
    }

    function renderGraphs() {
      fetch(`/api/logs?station=${activeGraphStation}`)
        .then(res => res.json())
        .then(data => {
          if (!data.logs || data.logs.length === 0) {
            drawLineChart('flowRateCanvas', [], [], 'mL/hr', '#0072ff', 'rgba(0,114,255,0.2)');
            drawBarChart('dropsCanvas', [], [], '#38a169');
            document.getElementById('graphPeakRate').innerText = "0.0";
            document.getElementById('graphMinRate').innerText = "0.0";
            document.getElementById('graphAvgRate').innerText = "0.0";
            document.getElementById('graphTotalVol').innerText = "0.00";
            return;
          }

          const labels = [];
          const rateData = [];
          const dropsData = [];
          let sumRate = 0, sumDrops = 0;
          let peak = 0, min = 999999;

          for (let i = 0; i < data.logs.length; i++) {
            const item = data.logs[i];
            const minsAgo = data.logs.length - 1 - i;
            labels.push(minsAgo);
            rateData.push(item.rate);
            dropsData.push(item.drops);
            sumRate += item.rate;
            sumDrops += item.drops;
            if (item.rate > peak) peak = item.rate;
            if (item.rate < min) min = item.rate;
          }

          if (min === 999999) min = 0;

          document.getElementById('graphPeakRate').innerText = peak.toFixed(1);
          document.getElementById('graphMinRate').innerText = min.toFixed(1);
          document.getElementById('graphAvgRate').innerText = (sumRate / data.logs.length).toFixed(1);
          document.getElementById('graphTotalVol').innerText = (sumDrops / (data.dropFactor || 20)).toFixed(2);

          drawLineChart('flowRateCanvas', labels, rateData, 'mL/hr', '#0072ff', 'rgba(0,114,255,0.2)');
          drawBarChart('dropsCanvas', labels, dropsData, '#38a169');
        })
        .catch(err => console.log(err));
    }

    function switchLogTab(stationId, tabEl) {
      activeLogTab = stationId;
      renderTabs();
      fetchLogs();
    }

    function fetchLogs() {
      fetch(`/api/logs?station=${activeLogTab}`)
        .then(res => res.json())
        .then(data => {
          const tbody = document.getElementById('logTableBody');
          tbody.innerHTML = '';

          if (!data.logs || data.logs.length === 0) {
            tbody.innerHTML = `<tr><td colspan="9" style="text-align: center; color: var(--text-muted); padding: 22px; font-size:14.5px;">ยังไม่มีข้อมูลบันทึกสำหรับ เตียง ${activeLogTab}</td></tr>`;
            document.getElementById('logSumDrops').innerText = "0";
            document.getElementById('logSumVolume').innerText = "0.00";
            document.getElementById('logAvgRate').innerText = "0.00";
            return;
          }

          let sumDrops = 0;
          let sumRate = 0;

          for (let i = data.logs.length - 1; i >= 0; i--) {
            const item = data.logs[i];
            sumDrops += item.drops;
            sumRate += item.rate;

            const row = document.createElement('tr');
            row.innerHTML = `
              <td><b>${item.min}</b></td>
              <td><span style="color:#2b6cb0; font-weight:600;">${item.time}</span></td>
              <td><b>${item.drops}</b> drops</td>
              <td>${item.vol} mL</td>
              <td><span style="color:#2f855a; font-weight:700;">${item.rate}</span> mL/hr</td>
              <td>${item.target} mL/hr</td>
              <td>${(ALERT_META[item.alert] || ALERT_META[0]).text}</td>
              <td>${item.rssi} dBm</td>
              <td>${item.battery > 0.5 ? item.battery + ' V' : 'N/A'}</td>
            `;
            tbody.appendChild(row);
          }

          document.getElementById('logSumDrops').innerText = sumDrops;
          document.getElementById('logSumVolume').innerText = (sumDrops / (data.dropFactor || 20)).toFixed(2);
          document.getElementById('logAvgRate').innerText = (sumRate / data.logs.length).toFixed(2);
        })
        .catch(err => console.log(err));
    }

    function downloadShiftCSV() {
      window.location.href = `/api/logs/csv?station=${activeLogTab}`;
    }

    function openApModal() { document.getElementById('apModal').style.display = 'flex'; fetchApStatus(); }
    function closeApModal() { document.getElementById('apModal').style.display = 'none'; }
    function fetchApStatus() {
      fetch('/api/ap/status').then(r=>r.json()).then(d=>{
        document.getElementById('apDetailBox').innerHTML =
          `<b>ชื่อ Wi-Fi (SSID):</b> ${d.ssid}<br>` +
          `<b>รหัสผ่าน:</b> ${d.password}<br>` +
          `<b>ที่อยู่หน้าเว็บ:</b> http://${d.apIP}<br>` +
          `<b>ช่องสัญญาณ:</b> ${d.channel}<br>` +
          `<b>อุปกรณ์ที่ต่ออยู่:</b> ${d.clients} / ${d.maxClients} เครื่อง<br>` +
          `<b>เวลาเครื่อง:</b> ${d.currentTime}`;
      }).catch(e=>{ document.getElementById('apDetailBox').innerText = 'อ่านข้อมูลไม่สำเร็จ'; });
    }

    // ตั้งเวลาให้ Host จากนาฬิกาของเครื่องที่เปิดหน้านี้ (ใช้แทน NTP เพราะไม่มีอินเทอร์เน็ต)
    function syncHostTime() {
      const epoch = Math.floor(Date.now() / 1000);
      fetch(`/api/time/set?epoch=${epoch}`, {method:'POST'}).catch(e=>console.log(e));
    }

    setInterval(() => {
      if (document.hidden) return;   // ไม่ดึงข้อมูลเมื่อสลับแท็บ/ปิดจอ ลดภาระเมื่อมีผู้ใช้หลายเครื่อง
      fetch('/api/data')
        .then(res => res.json())
        .then(data => {
          activeBedCount = data.activeCount;
          document.getElementById('activeBedCountLabel').innerText = activeBedCount;

          const hVolts = data.hostBatVolts;
          const hPct = data.hostBatPct;
          const hostEl = document.getElementById('hostBatDisplay');
          if (hVolts > 0.5) {
            hostEl.innerText = `🔋 Host: ${hVolts.toFixed(2)}V (${hPct}%)`;
            if (hPct <= 20) {
              hostEl.style.background = '#e53e3e';
              hostEl.style.color = '#ffffff';
            } else {
              hostEl.style.background = 'rgba(0,0,0,0.2)';
              hostEl.style.color = '#ffffff';
            }
          } else {
            hostEl.innerText = '⚡ Host: USB Power';
            hostEl.style.background = 'rgba(0,0,0,0.2)';
          }

          let timeNote = '';
          if (!data.timeSynced) timeNote = data.timeApprox ? ' (เวลาโดยประมาณ)' : ' (ยังไม่ตั้งเวลา)';
          document.getElementById('hostClockDisplay').innerText = data.currentTime + timeNote;
          const vb = document.getElementById('versionBadge');
          if (vb && data.version) vb.innerText = 'v' + data.version;   // (5) อ่านเวอร์ชันจริงจาก Host
          document.getElementById('netStatusSubtitle').innerText =
            `AP: 192.168.4.1 · CH ${data.channel} · อุปกรณ์ ${data.apClients}/${data.apMaxClients} เครื่อง`;
          lastHostData = data;
          syncConfigsFromHost(data.stations);
          renderStations(data.stations);
          updateHostCfgNote();
        })
        .catch(err => console.log(err));
    }, 1000);

    setInterval(() => {
      if (document.getElementById('tab-graphs').classList.contains('active')) renderGraphs();
      if (document.getElementById('tab-logs').classList.contains('active')) fetchLogs();
    }, 15000);

    renderTabs();
    loadBedToForm(1);
    syncHostTime();
    setInterval(syncHostTime, 600000);   // ปรับเวลาให้ตรงทุก 10 นาที ระหว่างที่ยังเปิดหน้านี้
  </script>
</body>
</html>
)rawliteral";
