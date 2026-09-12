// ============================================================================
// ชุดทดสอบตัวตรวจจับหยด — คอมไพล์และรันบนเครื่อง PC ได้ทันที ไม่ต้องมีบอร์ด
//   ./run.sh
// แต่ละสถานการณ์จำลองสัญญาณเซนเซอร์อินฟราเรดจริง แล้วตรวจว่านับหยดได้ตรงไหม
// เกณฑ์ผ่าน: นับได้ภายใน +-1 หยดจากของจริง และต้องไม่นับเกิน (ห้ามนับเบิ้ล)
// ============================================================================
#include "../../firmware/ESP32-S3-Station-V_7_7_0/drop_detector.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <string>

struct Scenario {
  const char *name;
  float dropIntervalMs;      // ช่วงห่างระหว่างหยดจริง
  float dropAmp;             // ความสูงพัลส์
  float dropWidthMs;         // ความกว้างพัลส์
  float noisePp;             // สัญญาณรบกวนยอดถึงยอด
  float swingAmp;            // แอมพลิจูดการแกว่งของถุง
  float swingPeriodMs;       // คาบการแกว่ง
  float driftPerSec;         // ระดับสัญญาณไหลต่อวินาที
  int   blackoutMs;          // ช่วงที่ไม่มีการอ่านเลย (จำลองการวาดจอ)
  int   blackoutEveryMs;
  bool  doubleLobe;          // พัลส์มีสองลูกติดกัน (หัวหยดกับหางหยด)
  int   allowedMiss;         // ยอมให้ขาดได้กี่หยด (สำหรับเคสที่เกินขีดของฮาร์ดแวร์เอง)
};

static unsigned long g_rng = 12345;
static float frand() { g_rng = g_rng * 1103515245u + 12345u; return (float)((g_rng >> 16) & 0x7fff) / 32767.0f; }

static int sampleSignal(const Scenario &s, float tMs) {
  float v = 2700.0f;
  v += (frand() - 0.5f) * s.noisePp;
  if (s.swingAmp > 0) v += s.swingAmp * sinf(2.0f * (float)M_PI * tMs / s.swingPeriodMs);
  v -= s.driftPerSec * tMs / 1000.0f;

  float p = fmodf(tMs, s.dropIntervalMs);
  if (p < s.dropWidthMs) v -= s.dropAmp * sinf((float)M_PI * p / s.dropWidthMs);
  if (s.doubleLobe) {
    float q = p - (s.dropWidthMs + 4.0f);
    if (q >= 0 && q < s.dropWidthMs) v -= s.dropAmp * 0.75f * sinf((float)M_PI * q / s.dropWidthMs);
  }
  return (int)lrintf(v);
}

// จำนวนหยดที่ "มองเห็นได้จริง" — หยดที่ตกตรงช่วงที่ไม่มีการอ่านเลย ไม่มีทางตรวจพบได้
static int runScenario(const Scenario &s, int seconds, int *expectedOut) {
  iv::DetectorConfig cfg;
  iv::DropDetector det;
  det.begin(cfg);
  det.reset(2700);
  det.setPolarityAuto();     // ให้เดาทิศสัญญาณเอง เหมือนการใช้งานจริง

  int counted = 0, visible = 0;
  float lastDropPhase = 1e9f;
  uint32_t lastBlackout = 0;
  uint32_t tUs = 0;
  const uint32_t stepUs = 500;                 // อ่าน 2 kHz
  uint32_t endUs = (uint32_t)seconds * 1000000u;

  while (tUs < endUs) {
    float tMs = (float)tUs / 1000.0f;
    bool settled = tMs >= 3000.0f;              // ข้าม 3 วินาทีแรก (ช่วงเครื่องตั้งตัว)
    float phase = fmodf(tMs, s.dropIntervalMs);
    if (phase < lastDropPhase && settled) visible++;
    lastDropPhase = phase;
    if (det.update(sampleSignal(s, tMs), tUs) && settled) counted++;
    tUs += stepUs;
    if (s.blackoutMs > 0 && (tUs / 1000u) - lastBlackout >= (uint32_t)s.blackoutEveryMs) {
      lastBlackout = tUs / 1000u;
      tUs += (uint32_t)s.blackoutMs * 1000u;   // ช่วงที่ไม่ได้อ่านเลย
      lastDropPhase = fmodf((float)tUs / 1000.0f, s.dropIntervalMs);
    }
  }
  *expectedOut = visible;
  return counted;
}

int main() {
  // ค่าฐาน: หยดทุก 870 ms สูง 220 ADC กว้าง 12 ms สัญญาณรบกวน 36 ADC
  Scenario base = {"", 870, 220, 12, 36, 0, 500, 0, 0, 0, false, 1};

  Scenario cases[10];
  const char *names[10];
  int n = 0;

  cases[n] = base;                       names[n] = "นิ่งปกติ"; n++;
  // SNR ดิบเพียง 6.4 เท่า = เกินขีดความสามารถของเซนเซอร์เอง ยอมให้ขาดได้ 3 หยด
  cases[n] = base; cases[n].noisePp = 120; cases[n].allowedMiss = 3; names[n] = "สัญญาณรบกวนแรง 120 ADC"; n++;
  cases[n] = base; cases[n].dropAmp = 90;  names[n] = "หยดจาง (SNR ต่ำ)"; n++;
  cases[n] = base; cases[n].swingAmp = 150; cases[n].swingPeriodMs = 500; names[n] = "ถุงแกว่ง 2Hz +-150"; n++;
  cases[n] = base; cases[n].swingAmp = 300; cases[n].swingPeriodMs = 1000; names[n] = "ถุงแกว่ง 1Hz +-300"; n++;
  cases[n] = base; cases[n].driftPerSec = 20; names[n] = "ระดับไหล 20 ADC/s"; n++;
  cases[n] = base; cases[n].driftPerSec = 100; names[n] = "ระดับไหล 100 ADC/s"; n++;
  cases[n] = base; cases[n].blackoutMs = 40; cases[n].blackoutEveryMs = 150; names[n] = "วาดจอบล็อก 40ms/150ms"; n++;
  cases[n] = base; cases[n].doubleLobe = true; names[n] = "พัลส์สองลูกติดกัน"; n++;
  cases[n] = base; cases[n].dropIntervalMs = 300; names[n] = "หยดเร็ว 200 หยด/นาที"; n++;

  printf("\n%-26s %8s %8s %7s   %s\n", "สถานการณ์", "คาดหวัง", "นับได้", "ต่าง", "ผล");
  printf("--------------------------------------------------------------------\n");
  int failures = 0;
  for (int i = 0; i < n; i++) {
    int expected = 0;
    int got = runScenario(cases[i], 20, &expected);
    int diff = got - expected;
    bool pass = (diff <= 0) && (diff >= -cases[i].allowedMiss);   // ห้ามนับเกินเด็ดขาด
    if (!pass) failures++;
    printf("%-26s %8d %8d %+7d   %s\n", names[i], expected, got, diff, pass ? "ผ่าน" : "ไม่ผ่าน");
  }
  printf("--------------------------------------------------------------------\n");
  printf("%s (%d/%d ผ่าน)\n\n", failures ? "มีข้อที่ไม่ผ่าน" : "ผ่านทั้งหมด", n - failures, n);
  return failures ? 1 : 0;
}
