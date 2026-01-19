//（全体差し替えOK）

#include <Arduino.h>
#include "switch_ESP32.h"

NSGamepad Gamepad;

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

uint8_t axis(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
}

// ===== マクロ用 =====
String macro="";
bool recording=false;
bool playing=false;
unsigned long lastChange=0;
uint16_t lastState=0;
unsigned long pressTime=0;
uint16_t macroButtons=0;

uint16_t readButtons(){
  uint16_t s = 0;

  if(!digitalRead(BTN_Y)) s |= 1 << NSButton_Y;
  if(!digitalRead(BTN_B)) s |= 1 << NSButton_B;
  if(!digitalRead(BTN_A)) s |= 1 << NSButton_A;
  if(!digitalRead(BTN_X)) s |= 1 << NSButton_X;

  if(!digitalRead(ZL)) s |= 1 << NSButton_LeftTrigger;
  if(!digitalRead(ZR)) s |= 1 << NSButton_RightTrigger;

  if(!digitalRead(L))  s |= 1 << NSButton_LeftThrottle;
  if(!digitalRead(R))  s |= 1 << NSButton_RightThrottle;

  if(!digitalRead(MINUS)) s |= 1 << NSButton_Minus;
  if(!digitalRead(PLUS))  s |= 1 << NSButton_Plus;

  if(!digitalRead(LB)) s |= 1 << NSButton_LeftStick;
  if(!digitalRead(RB)) s |= 1 << NSButton_RightStick;

  return s;
}

void recordState(uint16_t s){
  macro+=String(s)+","+String(millis()-lastChange)+",";
  lastChange=millis();
}

void setup(){
  int pins[]={MStart1,LB,RB,UP,RIGHT,DOWN,LEFT,ZL,ZR,L,R,PLUS,MINUS,BTN_A,BTN_B,BTN_X,BTN_Y};
  for(int p:pins) pinMode(p,INPUT_PULLUP);

  analogReadResolution(12);
  Gamepad.begin();
  USB.begin();
}

void loop(){

  // ==== マクロボタン ====
  if(!digitalRead(MStart1)){
    if(!pressTime) pressTime=millis();
  }else if(pressTime){
    if(millis()-pressTime>1000){
      recording=!recording;
      if(recording){
        macro="";
        lastState=readButtons();
        lastChange=millis();
      }else{
        recordState(readButtons());
      }
    }else if(macro.length()>0){
      playing=true;
    }
    pressTime=0;
  }

  uint16_t manual=readButtons();
  if(recording && manual!=lastState){
    recordState(lastState);
    lastState=manual;
  }

  // ==== 再生処理（macroButtons更新のみ）====
  static int idx=0;
  static unsigned long wait=0;

  if(playing && millis()>wait){
    int c1=macro.indexOf(",",idx);
    if(c1==-1){playing=false;idx=0;macroButtons=0;}
    else{
      macroButtons=macro.substring(idx,c1).toInt();
      idx=c1+1;
      int c2=macro.indexOf(",",idx);
      int d=macro.substring(idx,c2).toInt();
      idx=c2+1;
      wait=millis()+d;
    }
  }

  uint16_t merged = manual | macroButtons;

  // ==== 軸は常に手動優先 ====
  Gamepad.leftXAxis(255-axis(LX));
  Gamepad.leftYAxis(255-axis(LY));
  Gamepad.rightXAxis(axis(RX));
  Gamepad.rightYAxis(axis(RY));

  Gamepad.buttons(merged);
  Gamepad.loop();
  delay(5);
}
