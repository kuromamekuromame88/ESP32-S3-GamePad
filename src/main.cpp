#include <Arduino.h>
#include <Preferences.h>
#include "switch_ESP32.h"

Preferences prefs;
NSGamepad Gamepad;

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

#define ZL 2
#define ZR 1
#define L 35
#define R 36

#define PLUS 38
#define MINUS 37

#define BTN_B 5
#define BTN_A 4
#define BTN_X 7
#define BTN_Y 6

// ================= 共通 =================
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

  if(!digitalRead(BTN_Y)) s|=1<<NSButton_Y;
  if(!digitalRead(BTN_B)) s|=1<<NSButton_B;
  if(!digitalRead(BTN_A)) s|=1<<NSButton_A;
  if(!digitalRead(BTN_X)) s|=1<<NSButton_X;

  if(!digitalRead(ZL)) s|=1<<NSButton_LeftTrigger;
  if(!digitalRead(ZR)) s|=1<<NSButton_RightTrigger;
  if(!digitalRead(L))  s|=1<<NSButton_LeftThrottle;
  if(!digitalRead(R))  s|=1<<NSButton_RightThrottle;

  if(!digitalRead(MINUS)) s|=1<<NSButton_Minus;
  if(!digitalRead(PLUS))  s|=1<<NSButton_Plus;

  if(!digitalRead(LB)) s|=1<<NSButton_LeftStick;
  if(!digitalRead(RB)) s|=1<<NSButton_RightStick;

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

// ================= マクロ記録 =================
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
    USBHID.begin();   // Generic HID
  }

  USB.begin();
}

// ================= loop =================
void loop(){

  // ==== マクロ/連打ボタン判定 ====
  uint16_t manual = readButtons();

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

  // ==== マクロ再生 ====
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

  // ==== 連打 ====
  if(turbo){
    if((millis()/80)%2==0){
      macroButtons = manual;
    }else{
      macroButtons = 0;
    }
  }

  uint16_t merged = manual | macroButtons;

  // ==== 軸 ====
  Gamepad.leftXAxis(255-axis(LX));
  Gamepad.leftYAxis(255-axis(LY));
  Gamepad.rightXAxis(axis(RX));
  Gamepad.rightYAxis(axis(RY));

  // ==== HAT（手動のみ）====
  Gamepad.dPad(
    !digitalRead(UP),
    !digitalRead(DOWN),
    !digitalRead(LEFT),
    !digitalRead(RIGHT)
  );

  // ==== 出力 ====
  if(mode==MODE_SWITCH){
    Gamepad.buttons(merged);
    Gamepad.loop();
  }else{
    // Generic HID送信（後で差し替え）
  }

  delay(5);
}
