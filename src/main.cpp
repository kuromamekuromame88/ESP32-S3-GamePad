#include <Arduino.h>
#include <Preferences.h>
#include "switch_ESP32.h"
#include "USB.h"
#include "USBHID.h"

Preferences prefs;
NSGamepad Gamepad;
USBHID HID;

// ================= モード =================
enum ControllerMode {
  MODE_SWITCH,
  MODE_GENERIC_HID
};
ControllerMode mode = MODE_SWITCH;

// ================= ピン定義 =================
#define MStart1 42

#define LX 18
#define LY 17
#define RX 15
#define RY 16

#define LB 39
#define RB 40

#define UP 3
#define RIGHT 9
#define DOWN 10
#define LEFT 11

#define ZL 35
#define ZR 36
#define L 2
#define R 1

#define PLUS 38
#define MINUS 37

#define BTN_B 5
#define BTN_A 4
#define BTN_X 7
#define BTN_Y 6

// ================= ユーティリティ =================
uint8_t axis(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
}

// ================= マクロ =================
String macro="";
bool recording=false;
bool playing=false;
bool turbo=false;

unsigned long lastChange=0;
uint16_t lastState=0;
unsigned long pressTime=0;
uint16_t macroButtons=0;

// ================= ボタン読み取り =================
uint16_t readButtons(){
  uint16_t s=0;

  if(!digitalRead(BTN_Y)) s|=1<<0;
  if(!digitalRead(BTN_B)) s|=1<<1;
  if(!digitalRead(BTN_A)) s|=1<<2;
  if(!digitalRead(BTN_X)) s|=1<<3;

  if(!digitalRead(ZL)) s|=1<<4;
  if(!digitalRead(ZR)) s|=1<<5;
  if(!digitalRead(L))  s|=1<<6;
  if(!digitalRead(R))  s|=1<<7;

  if(!digitalRead(MINUS)) s|=1<<8;
  if(!digitalRead(PLUS))  s|=1<<9;

  if(!digitalRead(LB)) s|=1<<10;
  if(!digitalRead(RB)) s|=1<<11;

  return s;
}

// ================= マクロ保存 =================
void saveMacro(){
  prefs.begin("macro", false);
  prefs.putString("data", macro);
  prefs.end();
}
void loadMacro(){
  prefs.begin("macro", true);
  macro=prefs.getString("data","");
  prefs.end();
}

void recordState(uint16_t s){
  macro+=String(s)+","+String(millis()-lastChange)+",";
  lastChange=millis();
}

// ================= モード判定 =================
void detectMode(){
  pinMode(PLUS, INPUT_PULLUP);
  delay(10);
  if(!digitalRead(PLUS)){
    mode=MODE_GENERIC_HID;
  }
}

// ================= Generic HID Report =================
typedef struct __attribute__((packed)){
  uint16_t buttons;
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t rz;
  uint8_t hat;
} HIDReport;

HIDReport hidReport;

// ================= HAT計算 =================
uint8_t calcHat(){
  bool u=!digitalRead(UP);
  bool d=!digitalRead(DOWN);
  bool l=!digitalRead(LEFT);
  bool r=!digitalRead(RIGHT);

  if(u && r) return 1;
  if(r && d) return 3;
  if(d && l) return 5;
  if(l && u) return 7;
  if(u) return 0;
  if(r) return 2;
  if(d) return 4;
  if(l) return 6;
  return 8; // neutral
}

// ================= setup =================
void setup(){
  int pins[]={MStart1,LB,RB,UP,RIGHT,DOWN,LEFT,ZL,ZR,L,R,PLUS,MINUS,BTN_A,BTN_B,BTN_X,BTN_Y};
  for(int p:pins) pinMode(p,INPUT_PULLUP);

  analogReadResolution(12);
  loadMacro();
  detectMode();

  if(mode==MODE_SWITCH){
    Gamepad.begin();
  }else{
    HID.begin();
  }
  USB.begin();
}

// ================= loop =================
void loop(){

  uint16_t manual=readButtons();

  // ---- マクロ制御 ----
  if(!digitalRead(MStart1)){
    if(!pressTime) pressTime=millis();
  }else if(pressTime){
    unsigned long dt=millis()-pressTime;

    if(dt>1000){
      recording=!recording;
      if(recording){
        macro="";
        lastState=manual;
        lastChange=millis();
      }else{
        recordState(manual);
        saveMacro();
      }
    }else{
      if(manual==0 && macro.length()>0){
        playing=true;
      }else{
        turbo=!turbo;
      }
    }
    pressTime=0;
  }

  if(recording && manual!=lastState){
    recordState(lastState);
    lastState=manual;
  }

  static int idx=0;
  static unsigned long wait=0;

  if(playing && millis()>wait){
    int c1=macro.indexOf(",",idx);
    if(c1==-1){
      playing=false;
      idx=0;
      macroButtons=0;
    }else{
      macroButtons=macro.substring(idx,c1).toInt();
      idx=c1+1;
      int c2=macro.indexOf(",",idx);
      int d=macro.substring(idx,c2).toInt();
      idx=c2+1;
      wait=millis()+d;
    }
  }

  if(turbo){
    macroButtons = ((millis()/80)%2)?0:manual;
  }

  uint16_t merged = manual | macroButtons;

  // ---- 軸 ----
  uint8_t lx=255-axis(LX);
  uint8_t ly=255-axis(LY);
  uint8_t rx=axis(RX);
  uint8_t ry=axis(RY);

  if(mode==MODE_SWITCH){
    Gamepad.leftXAxis(lx);
    Gamepad.leftYAxis(ly);
    Gamepad.rightXAxis(rx);
    Gamepad.rightYAxis(ry);

    Gamepad.dPad(
      !digitalRead(UP),
      !digitalRead(DOWN),
      !digitalRead(LEFT),
      !digitalRead(RIGHT)
    );

    Gamepad.buttons(merged);
    Gamepad.loop();
  }else{
    hidReport.buttons = merged;
    hidReport.x = lx;
    hidReport.y = ly;
    hidReport.z = rx;
    hidReport.rz = ry;
    hidReport.hat = calcHat();

    HID.SendReport(1, &hidReport, sizeof(hidReport));
  }

  delay(5);
}
