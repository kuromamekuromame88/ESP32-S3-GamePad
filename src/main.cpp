#include <Arduino.h>
#include <Preferences.h>
#include <USB.h>
#include <USBHIDGamepad.h>
#include "switch_ESP32.h"

// ================== インスタンス ==================
NSGamepad      SwitchGamepad;
USBHIDGamepad  USBGamepad;
Preferences prefs;

// ================== モード ==================
enum Mode {
  MODE_SWITCH,
  MODE_USBHID
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

// ================== スティック変換 ==================
uint8_t axisSwitch(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
}

int8_t axisUSB(int pin){
  return map(analogRead(pin), 0, 4095, -127, 127);
}

// ================== マクロ用 ==================
String macro="";
bool recording=false;
bool playing=false;

unsigned long lastChange=0;
uint16_t lastState=0;
unsigned long pressTime=0;
uint16_t macroButtons=0;

// ================== Switch用ボタン取得 ==================
uint16_t readButtonsSwitch(){
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

// ================== USBHID用ボタン取得 ==================
uint32_t readButtonsUSB(){
  uint32_t b = 0;

  if(!digitalRead(BTN_A)) b |= 1 << BUTTON_A;
  if(!digitalRead(BTN_B)) b |= 1 << BUTTON_B;
  if(!digitalRead(BTN_X)) b |= 1 << BUTTON_X;
  if(!digitalRead(BTN_Y)) b |= 1 << BUTTON_Y;

  if(!digitalRead(LB))    b |= 1 << BUTTON_TL;
  if(!digitalRead(RB))    b |= 1 << BUTTON_TR;

  if(!digitalRead(ZL))    b |= 1 << BUTTON_TL2;
  if(!digitalRead(ZR))    b |= 1 << BUTTON_TR2;

  if(!digitalRead(MINUS)) b |= 1 << BUTTON_SELECT;
  if(!digitalRead(PLUS))  b |= 1 << BUTTON_START;

  return b;
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
  int pins[] = {
    MStart1, LB, RB, UP, RIGHT, DOWN, LEFT,
    ZL, ZR, L, R, PLUS, MINUS,
    BTN_A, BTN_B, BTN_X, BTN_Y
  };
  for(int p : pins) pinMode(p, INPUT_PULLUP);

  analogReadResolution(12);
  loadMacro();

  delay(10);

  // 起動時 PLUS 判定
  if(!digitalRead(PLUS)){
    mode = MODE_USBHID;
  }else{
    mode = MODE_SWITCH;
  }

  if(mode == MODE_SWITCH){
    SwitchGamepad.begin();
    USB.begin();
  }else{
    USBGamepad.begin();
    USB.begin();
  }
}

// ================== メインループ ==================
void loop(){

  // ===== 共通 =====
  if(!digitalRead(MStart1)){
    if(!pressTime) pressTime = millis();
  }else if(pressTime){
    unsigned long held = millis() - pressTime;

    if(held > 1000){
      recording = !recording;
      if(recording){
        macro="";
        lastState = readButtonsSwitch();
        lastChange = millis();
      }else{
        recordState(readButtonsSwitch());
        saveMacro();
      }
    }else{
      if(macro.length()) playing = true;
    }
    pressTime = 0;
  }

  // ===== Switch モード =====
  if(mode == MODE_SWITCH){

    uint16_t manual = readButtonsSwitch();

    if(recording && manual != lastState){
      recordState(lastState);
      lastState = manual;
    }

    uint16_t merged = manual | macroButtons;

    SwitchGamepad.leftXAxis(255 - axisSwitch(LX));
    SwitchGamepad.leftYAxis(255 - axisSwitch(LY));
    SwitchGamepad.rightXAxis(axisSwitch(RX));
    SwitchGamepad.rightYAxis(axisSwitch(RY));

    SwitchGamepad.dPad(
      !digitalRead(UP),
      !digitalRead(DOWN),
      !digitalRead(LEFT),
      !digitalRead(RIGHT)
    );

    SwitchGamepad.buttons(merged);
    SwitchGamepad.loop();
  }

  // ===== USBHID モード =====
  else{

    uint32_t buttons = readButtonsUSB();

    uint8_t hat = HAT_CENTER;
    if(!digitalRead(UP))       hat = HAT_UP;
    else if(!digitalRead(RIGHT)) hat = HAT_RIGHT;
    else if(!digitalRead(DOWN))  hat = HAT_DOWN;
    else if(!digitalRead(LEFT))  hat = HAT_LEFT;

    USBGamepad.send(
      axisUSB(LX),
      axisUSB(LY),
      axisUSB(RX),
      axisUSB(RY),
      0, 0,
      hat,
      buttons
    );
  }

  delay(5);
}
