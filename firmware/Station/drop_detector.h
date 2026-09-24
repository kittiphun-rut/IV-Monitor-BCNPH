/**
 * @file      drop_detector.h
 * @brief     อัลกอริทึมตรวจจับหยดสารน้ำที่ตั้งค่าตัวเองได้ ไม่ผูกกับ Arduino
 * @version   1.0.0
 * @date      2026-09-12
 * @author    นายกิตติพันธ์ รัตนคร <kittiphun.rut@mcu.ac.th>
 *
 * @par Organization
 * วิทยาลัยพยาบาลบรมราชชนนี แพร่ คณะพยาบาลศาสตร์ สถาบันพระบรมราชชนก
 * หอผู้ป่วยอายุรกรรม โรงพยาบาลสูงเม่น จังหวัดแพร่
 * อาจารย์ที่ปรึกษา ดร.กรรณิการ์ กาศสมบูรณ์
 *
 * @par Description
 * ออกแบบให้ไม่ต้องคาลิเบรตด้วยมือ พยาบาลแค่หนีบเซนเซอร์แล้วเปิดโรลเลอร์
 * เครื่องเรียนรู้เองภายในไม่กี่หยดแล้วขึ้นว่าพร้อมใช้งาน
 * ไฟล์นี้ใช้แค่ `<math.h>` กับ `<stdint.h>` จึงคอมไพล์และทดสอบบน PC ได้ตรง ๆ
 *
 * @par Revision History
 * | Version | Date | Change |
 * |---|---|---|
 * | 1.0.0 | 2026-09-12 | แยกอัลกอริทึมออกจากไฟล์เฟิร์มแวร์ ให้ทดสอบบน PC ได้อิสระ |
 *
 * @warning  ไฟล์นี้ใช้ร่วมกันระหว่าง `firmware/Station/` กับ `firmware/Station-C3-OLED/`
 *           และ **ต้องเหมือนกันทุกไบต์** แก้ที่ไหนต้องคัดลอกไปอีกที่เสมอ
 *           เลขเวอร์ชันในไฟล์นี้จึงเป็นของอัลกอริทึมเอง ไม่ผูกกับ APP_VERSION ของสเก็ตช์ใด
 *
 * @note     ทดสอบด้วย `bash tools/detector-test/run.sh` 10 สถานการณ์ รวมกรณีถุงแกว่ง
 */

// ============================================================================
// drop_detector.h — ตัวตรวจจับหยดสารน้ำที่ตั้งค่าตัวเองได้ (Smart IV Alert)
//
// ออกแบบมาเพื่อให้ "ไม่ต้องคาลิเบรตด้วยมือ" พยาบาลแค่หนีบเซนเซอร์แล้วเปิดโรลเลอร์
// เครื่องจะเรียนรู้เองภายในไม่กี่หยดแล้วขึ้นว่าพร้อมใช้งาน
//
// ไฟล์นี้ไม่ผูกกับ Arduino เลย (ใช้แค่ <math.h> กับ <stdint.h>) จึงคอมไพล์และ
// ทดสอบบนเครื่อง PC ได้ตรง ๆ — ดู tools/detector-test/
//
// ----------------------------------------------------------------------------
// หลักการที่ใช้ (เรียงตามลำดับการไหลของสัญญาณ)
//
// 1) มีเดียน 5 จุด — ตัดสไปก์เดี่ยวของ ADC และจังหวะที่ ESP-NOW ส่งข้อมูล
//    (Hampel/median filter ตัด impulse noise ได้โดยไม่ทำให้ขอบสัญญาณมน)
//
// 2) ตัวกรองจับคู่รูปคลื่น (matched filter) — ในกรณีสัญญาณรบกวนขาว ตัวกรองที่ให้
//    อัตราส่วนสัญญาณต่อสัญญาณรบกวนสูงสุดคือตัวที่มีรูปร่างเหมือนสัญญาณที่จะตรวจจับ
//    พัลส์หยดมีรูปร่างใกล้เคียงกันทุกหยด จึงประมาณด้วยค่าเฉลี่ยเคลื่อนที่ความยาว
//    เท่ากับความกว้างพัลส์ที่เรียนรู้ไว้ ได้กำไร SNR ราว sqrt(N) เท่า
//    (N = 17 ตัวอย่าง -> ดีขึ้นราว 4 เท่า โดยไม่ต้องแก้ฮาร์ดแวร์เลย)
//
// 3) ผลต่างอันดับสอง (discrete Laplacian / difference of boxcars)
//       edge = 2*b(t-W) - b(t) - b(t-2W)     โดย b คือผลจากข้อ 2 และ W คือความกว้างพัลส์
//    นี่คือหัวใจของรุ่นนี้ และเป็นตัวแก้ปัญหา "ถุงน้ำเกลือแกว่งตอนคนไข้เดิน"
//    ผลต่างอันดับสองตัดทั้งค่าคงที่ (DC) และความชันคงที่ (drift) ออกโดยอัตโนมัติ
//    เหลือไว้เฉพาะ "ความโค้ง" ที่สเกลเดียวกับความกว้างพัลส์
//    - สัญญาณช้าอย่างการแกว่ง 2 Hz ที่แอมพลิจูด 150 ADC เหลือเพียงราว A*(w*dt)^2
//      = 150 * (2*pi*2*0.0085)^2 ~ 1.7 ADC เท่านั้น
//    - ส่วนพัลส์หยดกว้าง 12 ms ผ่านมาได้เต็ม ๆ และยังถูกขยายเป็นราว 2 เท่า
//    จึงแยกหยดออกจากการแกว่งได้ด้วยอัตราส่วนราว 100 เท่า โดยไม่ต้องเดาเกณฑ์เอง
//    และเพราะ DC ถูกตัดโดยโครงสร้าง จึงไม่มีปัญหา "เส้นฐานค้าง" อีกเลย
//
// 4) ประมาณสัญญาณรบกวนแบบทนทาน (robust) — ใช้ตัวติดตามมัธยฐานแบบสตรีมของ |สัญญาณ|
//    ได้ค่า MAD แล้วแปลงเป็นค่าเบี่ยงเบนมาตรฐานด้วย sigma = 1.4826 x MAD
//    ข้อดีสำคัญ: มัธยฐานไม่ถูกดึงโดยค่าผิดปกติ จึงวัดสัญญาณรบกวนได้ "ขณะที่น้ำกำลังหยด"
//    ไม่ต้องให้ผู้ใช้หยุดน้ำเพื่อวัดพื้นสัญญาณรบกวนอีกต่อไป
//
// 5) เกณฑ์แบบ CFAR (Constant False Alarm Rate) — ตั้งเกณฑ์ที่ k เท่าของ sigma
//    ที่ประมาณได้ อัตราการแจ้งเตือนผิดพลาดจึงคงที่ไม่ว่าสัญญาณรบกวนจะมากหรือน้อย
//    k = 6 กับสัญญาณรบกวนแบบเกาส์ให้โอกาสผิดพลาดราว 1e-9 ต่อตัวอย่าง
//
// 6) ชมิตต์ทริกเกอร์ + ประตูความกว้าง + เวลาพักรับ — นับตอน "พัลส์จบ" ที่จุดเดียว
//    และต้องกลับมานิ่งก่อนจึงรับพัลส์ถัดไป ทำให้ 1 พัลส์ = 1 หยด เสมอ
//
// 7) ยืนยันด้วยความสม่ำเสมอของช่วงหยด — การหยดของสารน้ำเป็นกระบวนการกึ่งคาบ
//    ถ้าช่วงห่างระหว่างหยดเกาะกลุ่มกัน แปลว่ากำลังจับหยดจริง ไม่ใช่สัญญาณรบกวน
//    ใช้ตัดสินว่าจะขึ้นสถานะ "พร้อมใช้งาน" ให้พยาบาลเห็นเมื่อไร
// ============================================================================
#pragma once
#include <stdint.h>
#include <math.h>
#include <string.h>

namespace iv {

// ---------------------------------------------------------------- ค่าตั้งต้น
struct DetectorConfig {
  uint32_t sampleIntervalUs = 500;   // คาบการอ่าน ADC เป้าหมาย (2 kHz)
  uint16_t gapResyncMs      = 25;    // อ่านขาดช่วงนานกว่านี้ = ทิ้งพัลส์ที่เห็นไม่ครบ
  float    cfarK            = 6.0f;  // เกณฑ์ = k เท่าของ sigma ที่ประมาณได้
  float    releaseRatio     = 0.40f; // เกณฑ์ปลด = สัดส่วนนี้ของเกณฑ์เข้า (ฮิสเทอรีซิส)
  float    minAmpRatio      = 0.35f; // พัลส์ที่เตี้ยกว่าสัดส่วนนี้ของที่เรียนรู้ = ไม่ใช่หยด
  uint16_t minWidthMs       = 2;     // แคบกว่านี้ = สัญญาณรบกวน
  uint16_t maxWidthMs       = 150;   // กว้างกว่านี้ = ไม่ใช่หยด (สิ่งบัง/ระดับน้ำเปลี่ยน)
  uint16_t rearmMs          = 20;    // ต้องนิ่งใต้เกณฑ์ปลดเท่านี้ก่อนรับหยดถัดไป
  uint16_t polarityTrialMs  = 10000; // ลองทิศละกี่มิลลิวินาทีก่อนสลับ (ถ้ายังจับหยดไม่ได้)
  uint16_t startWidthMs     = 12;    // ความกว้างพัลส์ที่ใช้ตั้งต้นก่อนจะเรียนรู้ของจริง
                                     // (หยดสารน้ำอยู่ในช่วง 5-30 ms ค่ากลางจึงจับได้ทุกแบบ)
  uint16_t stuckTimeoutMs   = 300;   // ค้างในเฟสรอนิ่งนานเกินนี้ = ยึดเส้นฐานใหม่
  uint16_t baseTauMs        = 500;   // เส้นฐาน DC (ใช้แสดงผลและตรวจการแกว่งเท่านั้น)
  uint16_t matchedMaxTaps   = 40;    // ความยาวสูงสุดของตัวกรองจับคู่รูปคลื่น
  uint8_t  lockNeeded       = 4;     // ต้องได้ช่วงหยดที่สม่ำเสมอกี่ครั้งจึงถือว่าพร้อม
  float    lockTolerance    = 0.30f; // ช่วงหยดต่างจากค่ากลางได้ไม่เกินสัดส่วนนี้
};

enum class Phase : uint8_t { Watch = 0, Pulse = 1, Settle = 2 };

// สถานะที่พยาบาลเห็นบนหน้าจอ — ตั้งใจให้มีแค่ 4 แบบ อ่านแล้วรู้ว่าต้องทำอะไรต่อ
enum class Lock : uint8_t {
  Warmup = 0,   // เพิ่งเปิดเครื่อง กำลังวัดพื้นสัญญาณรบกวน (ไม่กี่วินาที)
  Learning,     // เห็นหยดแล้ว กำลังยืนยันจังหวะ
  Ready,        // จับหยดได้สม่ำเสมอ พร้อมใช้งาน
  NoDrops       // ไม่เห็นหยดเลยนานเกินไป -> ให้ตรวจตำแหน่งเซนเซอร์
};

struct DetectorStatus {
  Phase    phase          = Phase::Watch;
  Lock     lock           = Lock::Warmup;
  int      edge           = 0;   // สัญญาณที่ตัวตรวจจับใช้ตัดสินจริง (หลังกรองครบ)
  int      threshold      = 0;   // เกณฑ์เข้าปัจจุบัน
  int      noiseSigma     = 0;   // สัญญาณรบกวนที่ประมาณได้ (หน่วยเดียวกับ edge)
  int      learnedAmp     = 0;   // ความสูงพัลส์ที่เรียนรู้ไว้
  int      learnedWidthMs = 0;   // ความกว้างพัลส์ที่เรียนรู้ไว้
  int      snrX10         = 0;   // อัตราส่วนสัญญาณต่อสัญญาณรบกวน คูณ 10
  bool     motion         = false;   // กำลังแกว่ง (ถุงส่าย/คนไข้เดิน)
  uint8_t  confidencePct  = 0;   // ความสม่ำเสมอของช่วงหยด 0-100
  uint32_t drops = 0, rejects = 0, gaps = 0, recoveries = 0;
  uint8_t  polarityFlips = 0;    // จำนวนครั้งที่สลับทิศสัญญาณระหว่างหาทิศที่ถูก
};

// ---------------------------------------------------------------------------
// ตัวติดตามมัธยฐานแบบสตรีม (frugal streaming median)
// ขยับเข้าหาค่ากลางทีละก้าวเล็ก ๆ ใช้หน่วยความจำคงที่ และไม่ถูกดึงโดยค่าผิดปกติ
// ---------------------------------------------------------------------------
class MedianTracker {
 public:
  void reset(float initial, float floorStep) { v_ = initial; floor_ = floorStep; }
  void update(float x) {
    float step = v_ * 0.01f;
    if (step < floor_) step = floor_;
    if (x > v_)      v_ += step;
    else if (x < v_) v_ -= step;
    if (v_ < 0.0f) v_ = 0.0f;
  }
  float value() const { return v_; }
 private:
  float v_ = 0.0f, floor_ = 0.05f;
};

// ---------------------------------------------------------------------------
class DropDetector {
 public:
  void begin(const DetectorConfig &cfg) {
    cfg_ = cfg;
    if (cfg_.matchedMaxTaps > kMaxTaps) cfg_.matchedMaxTaps = kMaxTaps;
    reset(0);
    st_ = DetectorStatus();
  }

  // ตั้งต้นสายสัญญาณใหม่ทั้งหมด (เรียกตอนเปิดเครื่อง หรือเมื่อเปลี่ยนเซนเซอร์)
  void reset(int seedRaw) {
    for (uint8_t i = 0; i < kMedianN; i++) med_[i] = seedRaw;
    medIdx_ = 0;
    baseline_ = (float)seedRaw;
    // ตั้งความยาวตัวกรองจากความกว้างพัลส์ที่คาดไว้ ผลต่างอันดับสองจะตอบสนองสูงสุด
    // เมื่อระยะห่างของตัวกรองใกล้เคียงความกว้างพัลส์จริง
    setTaps(widthToTaps((float)cfg_.startWidthMs));
    boxSum_ = 0.0f;
    memset(box_, 0, sizeof(box_));
    memset(hist_, 0, sizeof(hist_));
    boxIdx_ = 0; histIdx_ = 0; primed_ = 0;
    noise_.reset(2.0f, 0.05f);
    phase_ = Phase::Watch;
    phaseMs_ = settleMs_ = 0;
    peak_ = 0; thrAtTrigger_ = 0;
    lastSampleUs_ = 0; haveSample_ = false;
    intervalCount_ = 0; intervalIdx_ = 0;
    lastDropMs_ = 0;
    motionRef_ = 0.0f; motionMs_ = 0; motionUntilMs_ = 0;
    trialStartMs_ = 0;
    learnAmp_ = 0.0f; learnWidth_ = 0.0f;
    startMs_ = 0;
  }

  // เรียกให้ถี่ที่สุดเท่าที่ทำได้ — คืน true เมื่อ "เพิ่งนับหยดได้ 1 หยด"
  // raw   = ค่าที่อ่านจาก ADC
  // nowUs = เวลาปัจจุบันหน่วยไมโครวินาที
  bool update(int raw, uint32_t nowUs) {
    if (!haveSample_) {
      lastSampleUs_ = nowUs; haveSample_ = true;
      startMs_ = trialStartMs_ = nowUs / 1000u;
    }
    uint32_t dtUs = nowUs - lastSampleUs_;
    if (dtUs < cfg_.sampleIntervalUs) return false;
    lastSampleUs_ = nowUs;
    uint32_t nowMs = nowUs / 1000u;

    // -- อ่านขาดช่วงนาน (เช่นกำลังวาดจอทั้งหน้า): ทิ้งเฉพาะพัลส์ที่มองเห็นไม่ครบ
    //    แต่ยังเก็บเส้นฐานและค่าที่เรียนรู้ไว้ เพราะเป็นข้อมูลที่สะสมมานาน
    if (dtUs > (uint32_t)cfg_.gapResyncMs * 1000u) {
      for (uint8_t i = 0; i < kMedianN; i++) med_[i] = raw;
      setTaps(taps_);                       // ล้างท่อกรอง ข้อมูลที่ขาดช่วงใช้ไม่ได้
      if (phase_ == Phase::Pulse) st_.rejects++;
      phase_ = Phase::Settle;
      phaseMs_ = settleMs_ = nowMs;
      st_.gaps++;
      return false;
    }

    // -- 1) มีเดียน 5 จุด
    int filtered = pushMedian(raw);

    // -- 2+3) ตัวกรองจับคู่รูปคลื่น แล้วหาผลต่างอันดับสอง
    //          ขั้นนี้ตัด DC และความชันคงที่ออกเอง จึงกันถุงแกว่งได้โดยโครงสร้าง
    float tickMs = (float)dtUs / 1000.0f;
    bool  active = (phase_ == Phase::Pulse);
    float edge = pushFilter((float)filtered);

    // ยังไม่ยืนยันทิศสัญญาณ: ใช้วิธีที่เชื่อถือได้ที่สุดคือ "ลองแล้ววัดผล"
    // เริ่มจากทิศที่พบบ่อยที่สุด (หยดบังลำแสง -> ค่าลดลง) ถ้าครบเวลาแล้วยังจับจังหวะ
    // หยดไม่ได้ ก็สลับทิศแล้วลองใหม่ ตัวตัดสินคือความสม่ำเสมอของช่วงหยดจริง
    // ซึ่งเชื่อถือได้กว่าการวิเคราะห์รูปสัญญาณช่วงสั้น ๆ มาก
    if (polarityTrial_ && (nowMs - trialStartMs_) > cfg_.polarityTrialMs) {
      polarity_ = -polarity_;
      trialStartMs_ = nowMs;
      setTaps(taps_);
      learnAmp_ = 0.0f;
      intervalCount_ = intervalIdx_ = 0;
      lastDropMs_ = 0;
      st_.polarityFlips++;
    }

    // -- 4) ประมาณสัญญาณรบกวนแบบทนทาน (อัปเดตเฉพาะตอนไม่มีพัลส์)
    //       มัธยฐานของ |edge| = MAD  ->  sigma = 1.4826 x MAD
    if (!active) noise_.update(fabsf(edge));
    float sigma = noise_.value() * 1.4826f;
    if (sigma < 0.5f) sigma = 0.5f;

    // -- 5) เกณฑ์ CFAR (และไม่ต่ำกว่าสัดส่วนหนึ่งของหยดที่เรียนรู้ไว้)
    float thr = cfg_.cfarK * sigma;
    if (learnAmp_ > 0.0f) {
      float floorAmp = learnAmp_ * cfg_.minAmpRatio;
      if (thr < floorAmp) thr = floorAmp;
    }
    float rel = thr * cfg_.releaseRatio;

    st_.edge       = (int)lrintf(edge);
    st_.threshold  = (int)lrintf(thr);
    st_.noiseSigma = (int)lrintf(sigma);

    // -- 6) เครื่องจักรสถานะ: นับตอนพัลส์จบที่จุดเดียว
    bool counted = false;
    switch (phase_) {
      case Phase::Watch:
        if (edge >= thr) {
          phase_ = Phase::Pulse;
          phaseMs_ = nowMs;
          peak_ = edge;
          thrAtTrigger_ = thr;
        }
        break;

      case Phase::Pulse: {
        if (edge > peak_) peak_ = edge;
        uint32_t width = nowMs - phaseMs_;
        if (edge <= rel) {
          if (acceptPulse(peak_, (int)width)) {
            registerDrop(nowMs, peak_, (int)width);
            counted = true;
          } else {
            st_.rejects++;
          }
          phase_ = Phase::Settle; phaseMs_ = settleMs_ = nowMs;
        } else if (width > cfg_.maxWidthMs) {
          st_.rejects++;
          phase_ = Phase::Settle; phaseMs_ = settleMs_ = nowMs;
        }
        break;
      }

      case Phase::Settle:
        if (edge > rel) settleMs_ = nowMs;
        else if (nowMs - settleMs_ >= cfg_.rearmMs) phase_ = Phase::Watch;

        // กันค้าง: ระดับสัญญาณเลื่อนไปค้างจริง ๆ ให้ยึดค่าปัจจุบันเป็นเส้นฐานใหม่
        if (nowMs - phaseMs_ >= cfg_.stuckTimeoutMs) {
          phase_ = Phase::Watch;            // ผลต่างอันดับสองกลับเข้าศูนย์เองอยู่แล้ว
          st_.recoveries++;
        }
        break;
    }

    st_.phase = phase_;
    updateLock(nowMs);
    return counted;
  }

  const DetectorStatus &status() const { return st_; }

  // ทิศสัญญาณ: -1 = หยดทำให้ค่า ADC ลดลง (ปกติ), +1 = เพิ่มขึ้น
  // ปล่อยเป็นอัตโนมัติได้ เครื่องจะดูเองว่ายอดแหลมพุ่งไปทางไหนแรงกว่ากัน
  void setPolarity(int8_t p) { polarity_ = (p >= 0) ? 1.0f : -1.0f; polarityTrial_ = false; }
  // เริ่มจากทิศมาตรฐาน แล้วให้เครื่องพิสูจน์เองว่าถูกหรือไม่
  void setPolarityAuto() { polarity_ = -1.0f; polarityTrial_ = true; trialStartMs_ = 0; }
  bool polarityLocked() const { return !polarityTrial_; }
  int8_t polarity() const { return polarity_ > 0 ? 1 : -1; }

  // ใส่ค่าที่เคยเรียนรู้ไว้กลับเข้ามา (โหลดจาก NVS ตอนเปิดเครื่อง) เพื่อให้พร้อมใช้เร็วขึ้น
  void seedLearning(int amp, int widthMs) {
    if (amp > 0)     learnAmp_   = (float)amp;
    if (widthMs > 0) { learnWidth_ = (float)widthMs; updateTaps(); }
  }

 private:
  static const uint8_t kMedianN = 5;
  static const uint8_t  kMaxTaps = 40;
  static const uint16_t kMaxHist = 2 * kMaxTaps + 1;
  static const uint8_t kIntervals = 8;

  // ---- มีเดียน 5 จุด ----
  int pushMedian(int v) {
    med_[medIdx_] = v;
    medIdx_ = (uint8_t)((medIdx_ + 1) % kMedianN);
    int t[kMedianN];
    memcpy(t, med_, sizeof(t));
    for (uint8_t i = 1; i < kMedianN; i++) {          // เรียงแบบแทรก (N=5)
      int k = t[i]; int j = (int)i - 1;
      while (j >= 0 && t[j] > k) { t[j + 1] = t[j]; j--; }
      t[j + 1] = k;
    }
    return t[kMedianN / 2];
  }

  // ---- ตัวกรองจับคู่รูปคลื่น (ค่าเฉลี่ยเคลื่อนที่ W จุด) + ผลต่างอันดับสอง ----
  // คืนค่า edge = 2*b(t-W) - b(t) - b(t-2W) คูณด้วยทิศสัญญาณ
  // ผลต่างอันดับสองตัดทั้งค่าคงที่และความชันคงที่ออกเอง จึงไม่ต้องมีเส้นฐานในเส้นทางนี้
  float pushFilter(float x) {
    boxSum_ -= box_[boxIdx_];
    box_[boxIdx_] = x;
    boxSum_ += x;
    boxIdx_ = (uint8_t)((boxIdx_ + 1) % taps_);
    float b = boxSum_ / (float)taps_;

    hist_[histIdx_] = b;
    uint16_t iNow  = histIdx_;
    uint16_t iMid  = (uint16_t)((histIdx_ + histLen_ - taps_) % histLen_);
    uint16_t iFar  = (uint16_t)((histIdx_ + histLen_ - 2 * taps_) % histLen_);
    histIdx_ = (uint16_t)((histIdx_ + 1) % histLen_);
    // ต้องรอจนทั้งหน้าต่างเฉลี่ย (taps_) และวงแหวนประวัติ (histLen_) เต็มด้วยข้อมูลจริง
    // ไม่งั้นช่วงตั้งต้นจะให้ค่าพุ่งมหาศาลจนถูกเข้าใจผิดว่าเป็นหยด
    if (primed_ < (uint16_t)(histLen_ + taps_)) { primed_++; return 0.0f; }

    return polarity_ * (2.0f * hist_[iMid] - hist_[iNow] - hist_[iFar]);
  }

  void setTaps(int n) {
    if (n < 3) n = 3;
    if (n > (int)cfg_.matchedMaxTaps) n = (int)cfg_.matchedMaxTaps;
    taps_ = (uint8_t)n;
    histLen_ = (uint16_t)(2 * taps_ + 1);
    memset(box_, 0, sizeof(box_));
    memset(hist_, 0, sizeof(hist_));
    boxIdx_ = 0; histIdx_ = 0; boxSum_ = 0.0f; primed_ = 0;
  }

  int widthToTaps(float widthMs) const {
    float samplesPerMs = 1000.0f / (float)cfg_.sampleIntervalUs;
    return (int)lrintf(widthMs * samplesPerMs * 0.8f);
  }

  // ความยาวตัวกรองควรใกล้เคียงความกว้างพัลส์ จึงจะได้ SNR สูงสุด
  void updateTaps() {
    if (learnWidth_ <= 0.0f) return;
    int n = widthToTaps(learnWidth_);
    if (n != (int)taps_) setTaps(n);
  }

  bool acceptPulse(float peak, int widthMs) {
    if (widthMs < (int)cfg_.minWidthMs || widthMs > (int)cfg_.maxWidthMs) return false;
    if (learnAmp_ > 0.0f && peak < learnAmp_ * cfg_.minAmpRatio) return false;
    if (learnWidth_ > 0.0f && (float)widthMs > learnWidth_ * 3.0f + 20.0f) return false;
    return true;
  }

  // ความกว้างที่วัดได้คือความกว้าง "ณ ระดับเกณฑ์" ไม่ใช่ความกว้างจริงของพัลส์
  // ถ้าป้อนค่านี้กลับไปตั้งความยาวตัวกรองโดยตรง ตัวกรองจะหดลงเรื่อย ๆ จนตอบสนองหาย
  // สำหรับพัลส์รูปครึ่งไซน์ ความกว้างที่ระดับ f ของยอด = T * (1 - 2*asin(f)/pi)
  // จึงย้อนกลับไปหา T จริงได้ ทำให้ค่าที่เรียนรู้ไม่ขึ้นกับเกณฑ์ที่ใช้
  float trueWidthMs(float measuredMs, float peak) const {
    float f = (peak > 0.1f) ? (thrAtTrigger_ / peak) : 0.5f;
    if (f < 0.02f) f = 0.02f;
    if (f > 0.95f) f = 0.95f;
    float k = 1.0f - 2.0f * asinf(f) / 3.14159265f;
    if (k < 0.15f) k = 0.15f;
    float t = measuredMs / k;
    if (t < 3.0f)  t = 3.0f;
    if (t > 60.0f) t = 60.0f;
    return t;
  }

  void registerDrop(uint32_t nowMs, float peak, int widthMs) {
    // เรียนรู้รูปร่างหยดต่อเนื่อง (ค่าเฉลี่ยถ่วงน้ำหนัก 1/8 ต่อหยด)
    float wTrue = trueWidthMs((float)widthMs, peak);
    if (learnAmp_ <= 0.0f) { learnAmp_ = peak; learnWidth_ = wTrue; }
    else {
      learnAmp_   += (peak  - learnAmp_)   * 0.125f;
      learnWidth_ += (wTrue - learnWidth_) * 0.125f;
    }
    updateTaps();

    if (lastDropMs_ != 0) {
      uint32_t iv = nowMs - lastDropMs_;
      if (iv > 50 && iv < 60000) {
        intervals_[intervalIdx_] = iv;
        intervalIdx_ = (uint8_t)((intervalIdx_ + 1) % kIntervals);
        if (intervalCount_ < kIntervals) intervalCount_++;
      }
    }
    lastDropMs_ = nowMs;
    st_.drops++;
    st_.learnedAmp     = (int)lrintf(learnAmp_);
    st_.learnedWidthMs = (int)lrintf(learnWidth_);
    float sigma = noise_.value() * 1.4826f;
    st_.snrX10 = (sigma > 0.01f) ? (int)lrintf(learnAmp_ * 10.0f / sigma) : 999;
    if (st_.snrX10 > 999) st_.snrX10 = 999;
  }

  void updateMotion(uint32_t nowMs) {
    if (nowMs - motionMs_ >= 200) {
      float moved = fabsf(baseline_ - motionRef_);
      motionRef_ = baseline_;
      motionMs_ = nowMs;
      float thr = noise_.value() * 1.4826f * 2.0f;
      if (thr < 4.0f) thr = 4.0f;
      if (moved > thr) motionUntilMs_ = nowMs + 2000;
    }
    st_.motion = (int32_t)(motionUntilMs_ - nowMs) > 0;
  }

  // ยืนยันว่ากำลังจับ "หยด" จริง จากความสม่ำเสมอของช่วงหยด
  void updateLock(uint32_t nowMs) {
    if (intervalCount_ >= 2) {
      uint32_t t[kIntervals];
      memcpy(t, intervals_, sizeof(uint32_t) * intervalCount_);
      for (uint8_t i = 1; i < intervalCount_; i++) {
        uint32_t k = t[i]; int j = (int)i - 1;
        while (j >= 0 && t[j] > k) { t[j + 1] = t[j]; j--; }
        t[j + 1] = k;
      }
      uint32_t med = t[intervalCount_ / 2];
      uint8_t good = 0;
      for (uint8_t i = 0; i < intervalCount_; i++) {
        float d = fabsf((float)intervals_[i] - (float)med) / (float)med;
        if (d <= cfg_.lockTolerance) good++;
      }
      st_.confidencePct = (uint8_t)(good * 100u / intervalCount_);
      medianIntervalMs_ = med;

      if (good >= cfg_.lockNeeded) {
        st_.lock = Lock::Ready;
        polarityTrial_ = false;        // จับจังหวะหยดได้สม่ำเสมอ = ทิศสัญญาณถูกแล้ว
        return;
      }
    }

    // ไม่เห็นหยดเลยนานเกินไป -> บอกให้ไปตรวจตำแหน่งเซนเซอร์
    uint32_t sinceDrop = (lastDropMs_ != 0) ? (nowMs - lastDropMs_) : (nowMs - startMs_);
    uint32_t idleLimit = (medianIntervalMs_ > 0 && st_.lock == Lock::Ready)
                           ? (medianIntervalMs_ * 4 + 8000) : 30000;
    if (sinceDrop > idleLimit)      st_.lock = Lock::NoDrops;
    else if (st_.drops > 0)         st_.lock = Lock::Learning;
    else if (nowMs - startMs_ > 3000) st_.lock = Lock::Learning;
    else                            st_.lock = Lock::Warmup;
  }

  DetectorConfig cfg_;
  DetectorStatus st_;

  int      med_[kMedianN] = {0};
  uint8_t  medIdx_ = 0;
  float    baseline_ = 0.0f;
  float    polarity_ = -1.0f;
  bool     polarityTrial_ = false;
  uint32_t trialStartMs_ = 0;

  float    box_[kMaxTaps] = {0.0f};        // หน้าต่างของตัวกรองจับคู่รูปคลื่น
  float    hist_[kMaxHist] = {0.0f};       // ประวัติผลลัพธ์ เพื่อทำผลต่างอันดับสอง
  uint8_t  boxIdx_ = 0, taps_ = 8;
  uint16_t histIdx_ = 0, histLen_ = 17, primed_ = 0;
  float    boxSum_ = 0.0f;

  MedianTracker noise_;

  Phase    phase_ = Phase::Watch;
  uint32_t phaseMs_ = 0, settleMs_ = 0;
  float    peak_ = 0.0f, thrAtTrigger_ = 0.0f;

  uint32_t lastSampleUs_ = 0;
  bool     haveSample_ = false;
  uint32_t startMs_ = 0;

  float    learnAmp_ = 0.0f, learnWidth_ = 0.0f;

  uint32_t intervals_[kIntervals] = {0};
  uint8_t  intervalIdx_ = 0, intervalCount_ = 0;
  uint32_t lastDropMs_ = 0, medianIntervalMs_ = 0;

  float    motionRef_ = 0.0f;
  uint32_t motionMs_ = 0, motionUntilMs_ = 0;
};

}  // namespace iv
