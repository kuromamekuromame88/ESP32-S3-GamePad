#include <Arduino.h>
#include <Preferences.h>

#include "USBHIDGamepad.h"
#include "switch_ESP32.h"

// =====================
// HID インスタンス
// =====================
USBHIDGamepad USBGamepad;
NSGamepad      SwitchGamepad;

Preferences prefs;

// =====================
// モード判定
// =====================
bool usbMode = false;

// =====================
// ピン定義
// =====================
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

// =====================
// スティック
// =====================
uint8_t axis(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
}

// =====================
// マクロ
// =====================
String macro = "";
bool recording = false;
bool playing   = false;

unsigned long lastChange = 0;
unsigned long pressTime  = 0;

uint16_t lastState     = 0;
uint16_t macroButtons = 0;

// =====================
// ボタン取得
// =====================
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

// =====================
// マクロ保存
// =====================
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

// =====================
// setup
// =====================
void setup(){

  int pins[] = {
    MStart1, LB, RB,
    UP, RIGHT, DOWN, LEFT,
    ZL, ZR, L, R,
    PLUS, MINUS,
    BTN_A, BTN_B, BTN_X, BTN_Y
  };

  for(int p : pins) pinMode(p, INPUT_PULLUP);

  analogReadResolution(12);

  // ==== HID モード判定 ====
  delay(10); // 安定待ち
  usbMode = !digitalRead(PLUS); // 押されていたら USB HID

  loadMacro();

  if(usbMode){
    USBGamepad.begin();
    USB.begin();
  }else{
    SwitchGamepad.begin();
    USB.begin();
  }
}

// =====================
// loop
// =====================
void loop(){

  uint16_t manual = readButtons();

  // ==== MStart ====
  if(!digitalRead(MStart1)){
    if(!pressTime) pressTime = millis();
  }
  else if(pressTime){
    unsigned long held = millis() - pressTime;

    if(held > 1000){
      recording = !recording;
      if(recording){
        macro = "";
        lastState  = manual;
        lastChange = millis();
      }else{
        recordState(manual);
        saveMacro();
      }
    }else{
      if(macro.length() > 0){
        playing = true;
      }
    }
    pressTime = 0;
  }

  // ==== 録画 ====
  if(recording && manual != lastState){
    recordState(lastState);
    lastState = manual;
  }

  // ==== 再生 ====
  static int idx = 0;
  static unsigned long waitUntil = 0;

  if(playing && millis() >= waitUntil){
    int c1 = macro.indexOf(",", idx);
    if(c1 == -1){
      playing = false;
      idx = 0;
      macroButtons = 0;
    }else{
      macroButtons = macro.substring(idx, c1).toInt();
      idx = c1 + 1;

      int c2 = macro.indexOf(",", idx);
      int d = macro.substring(idx, c2).toInt();
      idx = c2 + 1;

      waitUntil = millis() + d;
    }
  }

  uint16_t merged = manual | macroButtons;

  // =====================
  // 出力（モード別）
  // =====================
  if(usbMode){
    USBGamepad.setAxes(
      axis(LX), axis(LY),
      axis(RX), axis(RY)
    );

    USBGamepad.setHat(
      (!digitalRead(UP))    ? 0 :
      (!digitalRead(RIGHT)) ? 2 :
      (!digitalRead(DOWN))  ? 4 :
      (!digitalRead(LEFT))  ? 6 : -1
    );

    USBGamepad.setButtons(merged);
    USBGamepad.send();
  }
  else{
    SwitchGamepad.leftXAxis(255 - axis(LX));
    SwitchGamepad.leftYAxis(255 - axis(LY));
    SwitchGamepad.rightXAxis(axis(RX));
    SwitchGamepad.rightYAxis(axis(RY));

    SwitchGamepad.dPad(
      !digitalRead(UP),
      !digitalRead(DOWN),
      !digitalRead(LEFT),
      !digitalRead(RIGHT)
    );

    SwitchGamepad.buttons(merged);
    SwitchGamepad.loop();
  }

  delay(5);
}
