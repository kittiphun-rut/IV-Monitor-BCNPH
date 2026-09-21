// เทียบสูตรเดิม (EMA ของส่วนกลับ) กับสูตรใหม่ (หน้าต่างเวลาสะสม)
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>
#define RATE_WINDOW_DROPS 8
static unsigned long rateWinMs[RATE_WINDOW_DROPS]; static uint8_t rateWinIdx, rateWinCount;
void resetRateWindow(){ rateWinIdx=0; rateWinCount=0; }
void pushRateSample(unsigned long t){ rateWinMs[rateWinIdx]=t; rateWinIdx=(uint8_t)((rateWinIdx+1)%RATE_WINDOW_DROPS); if(rateWinCount<RATE_WINDOW_DROPS) rateWinCount++; }
unsigned long rateWindowSpanMs(){ if(rateWinCount<2) return 0;
  uint8_t o=(uint8_t)((rateWinIdx+RATE_WINDOW_DROPS-rateWinCount)%RATE_WINDOW_DROPS);
  uint8_t n=(uint8_t)((rateWinIdx+RATE_WINDOW_DROPS-1)%RATE_WINDOW_DROPS);
  return rateWinMs[n]-rateWinMs[o]; }
unsigned long rateWindowMeanIntervalMs(){ unsigned long s=rateWindowSpanMs(); return s? s/(unsigned long)(rateWinCount-1):0; }
float rateFromWindow(uint8_t df){ unsigned long s=rateWindowSpanMs(); if(!s||!df) return 0.0f;
  return ((float)(rateWinCount-1)/(float)df)*3600000.0f/(float)s; }

int main(){
  const uint8_t DF=20;
  struct Case{const char*name; std::vector<unsigned long> iv; float truth;};
  // ของจริง 150 mL/h = 50 gtt/min = เฉลี่ย 1200 ms/หยด
  std::vector<Case> cases = {
    {"สม่ำเสมอ 1200ms        ", {1200}, 150.0f},
    {"สลับ 1000/1400ms       ", {1000,1400}, 150.0f},
    {"สลับ 800/1600ms        ", {800,1600}, 150.0f},
    {"ช้า 9000ms (20 mL/h)   ", {9000}, 20.0f},
  };
  for(auto&c:cases){
    // --- สูตรเดิม ---
    float ema=0; unsigned long t=0; 
    for(int k=0;k<400;k++){ unsigned long d=c.iv[k%c.iv.size()]; t+=d;
      float g=60000.0f/(float)d, r=(g*60.0f)/(float)DF;
      ema = (ema==0.0f)? r : (r*0.75f+ema*0.25f); }
    float emaLo=1e9,emaHi=-1e9,e2=0; unsigned long t2=0;
    for(int k=0;k<400;k++){ unsigned long d=c.iv[k%c.iv.size()]; t2+=d;
      float g=60000.0f/(float)d, r=(g*60.0f)/(float)DF;
      e2=(e2==0.0f)? r : (r*0.75f+e2*0.25f);
      if(k>50){ if(e2<emaLo)emaLo=e2; if(e2>emaHi)emaHi=e2; } }
    // --- สูตรใหม่ ---
    resetRateWindow(); float win=0,wLo=1e9,wHi=-1e9; unsigned long tt=0;
    for(int k=0;k<400;k++){ unsigned long d=c.iv[k%c.iv.size()]; tt+=d;
      pushRateSample(tt); float w=rateFromWindow(DF); if(w>0) win=w;
      if(k>50){ if(win<wLo)wLo=win; if(win>wHi)wHi=win; } }
    float lo=c.truth*0.9f, hi=c.truth*1.1f;
    printf("%s ของจริง %6.1f | เดิม %6.1f-%6.1f %s | ใหม่ %6.1f-%6.1f %s\n",
      c.name, c.truth, emaLo, emaHi, (emaLo>=lo&&emaHi<=hi)?"ผ่าน":"หลุด+-10%",
      wLo, wHi, (wLo>=lo&&wHi<=hi)?"ผ่าน":"หลุด+-10%");
  }
  // ตรวจว่าตรงกับวิธีที่ Host บันทึก Log (นับหยดใน 1 นาที)
  printf("\n-- เทียบกับสูตร Log ของ Host: (หยดใน 1 นาที / DF) x 60 --\n");
  resetRateWindow(); unsigned long t=0; float win=0;
  for(int k=0;k<200;k++){ t+=1200; pushRateSample(t); float w=rateFromWindow(DF); if(w>0) win=w; }
  float hostLog = (50.0f/(float)DF)*60.0f;   // 50 หยดใน 1 นาที
  printf("Station (หน้าต่าง) = %.2f mL/h | Host (log 1 นาที) = %.2f mL/h | ต่างกัน %.3f%%\n",
         win, hostLog, fabsf(win-hostLog)/hostLog*100.0f);
  return 0;
}
