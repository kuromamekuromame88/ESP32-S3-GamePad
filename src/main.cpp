#include <Arduino.h>
#include <Preferences.h>
#include "switch_ESP32.h"
#include <BleGamepad.h>

// ================== インスタンス ==================
NSGamepad Gamepad;
BleGamepad bleGamepad;
Preferences prefs;

// ================== モード ==================
enum Mode {
  MODE_SWITCH,
  MODE_BLE
};
Mode mode = MODE_SWITCH;

// ================== ピン定義 ==================
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

// ================== ユーティリティ ==================
uint8_t axis(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
}

// ================== マクロ ==================
String macro="";
bool recording=false;
bool playing=false;
bool rapidA=false;

unsigned long lastRapid=0;
unsigned long lastChange=0;
uint16_t lastState=0;
unsigned long pressTime=0;
uint16_t macroButtons=0;

// ================== ボタン取得（NS互換） ==================
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

// ================== マクロ保存 ==================
void recordState(uint16_t s){
  macro += String(s) + "," + String(millis() - lastChange) + ",";
  lastChange = millis();
}

void saveMacro(){
  prefs.begin("macro", false);
  prefs.putString("data", macro);
  prefs.end();
}

void loadMacro(){
  prefs.begin("macro", true);
  macro = prefs.getString("data", "");
  prefs.end();
}

// ================== セットアップ ==================
void setup(){
  int pins[]={MStart1,LB,RB,UP,RIGHT,DOWN,LEFT,ZL,ZR,L,R,PLUS,MINUS,BTN_A,BTN_B,BTN_X,BTN_Y};
  for(int p:pins) pinMode(p,INPUT_PULLUP);

  analogReadResolution(12);
  loadMacro();

  delay(10);
  if(!digitalRead(PLUS)){
    mode = MODE_BLE;
  }

  if(mode == MODE_SWITCH){
    Gamepad.begin();
    USB.begin();
  }else{
    bleGamepad.begin();
  }
}

// ================== メインループ ==================
void loop(){

  uint16_t manual = readButtons();

  // ---- MStart 処理 ----
  if(!digitalRead(MStart1)){
    if(!pressTime) pressTime = millis();
  }else if(pressTime){
    unsigned long held = millis() - pressTime;

    if(held > 1000){
      recording = !recording;
      if(recording){
        macro="";
        lastState = manual;
        lastChange = millis();
      }else{
        recordState(manual);
        saveMacro();
      }
    }else{
      if(manual & (1 << NSButton_A)){
        rapidA = true;
      }else if(macro.length()){
        playing = true;
      }
    }
    pressTime = 0;
  }

  // ---- A連打 ----
  static bool rapidState=false;
  if(rapidA){
    if(millis() - lastRapid > 40){
      rapidState = !rapidState;
      lastRapid = millis();
    }
  }else{
    rapidState = false;
  }

  if(rapidState) manual |=  (1 << NSButton_A);
  else           manual &= ~(1 << NSButton_A);

  // ---- 録画 ----
  if(recording && manual != lastState){
    recordState(lastState);
    lastState = manual;
  }

  // ---- マクロ再生 ----
  static int idx=0;
  static unsigned long wait=0;

  if(playing && millis() > wait){
    int c1 = macro.indexOf(",", idx);
    if(c1 == -1){
      playing=false; idx=0; macroButtons=0;
    }else{
      macroButtons = macro.substring(idx, c1).toInt();
      idx = c1 + 1;
      int c2 = macro.indexOf(",", idx);
      int d = macro.substring(idx, c2).toInt();
      idx = c2 + 1;
      wait = millis() + d;
    }
  }

  uint16_t merged = manual | macroButtons;

  uint8_t lx = 255 - axis(LX);
  uint8_t ly = 255 - axis(LY);
  uint8_t rx = axis(RX);
  uint8_t ry = axis(RY);

  // ================== 出力 ==================
  if(mode == MODE_SWITCH){

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

  }else if(bleGamepad.isConnected()){

    bleGamepad.setLeftThumb(lx, ly);
    bleGamepad.setRightThumb(rx, ry);

    bleGamepad.setButtons(merged);

    if(!digitalRead(UP))       bleGamepad.setHat(HAT_UP);
    else if(!digitalRead(RIGHT)) bleGamepad.setHat(HAT_RIGHT);
    else if(!digitalRead(DOWN))  bleGamepad.setHat(HAT_DOWN);
    else if(!digitalRead(LEFT))  bleGamepad.setHat(HAT_LEFT);
    else                         bleGamepad.setHat(HAT_CENTERED);
  }

  delay(5);
}
