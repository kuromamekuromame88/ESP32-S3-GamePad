#include <Arduino.h>
#include "switch_ESP32.h"

NSGamepad Gamepad;

// --- マクロボタン ---
#define MStart1 42

// --- スティック ---
#define LX 18
#define LY 17
#define RX 15
#define RY 16

#define LB 39
#define RB 40

// --- 十字キー ---
#define UP 3
#define RIGHT 9
#define DOWN 10
#define LEFT 11

// --- ショルダー ---
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

// ===== マクロ領域 =====
String macro = "";
bool recording = false;
bool playing = false;
unsigned long lastChange = 0;
uint16_t lastState = 0;
unsigned long pressTime = 0;

// ボタン状態を16bitにまとめる
uint16_t readButtons(){
  uint16_t s = 0;
  if(!digitalRead(BTN_A)) s |= 1<<0;
  if(!digitalRead(BTN_B)) s |= 1<<1;
  if(!digitalRead(BTN_X)) s |= 1<<2;
  if(!digitalRead(BTN_Y)) s |= 1<<3;
  if(!digitalRead(L)) s |= 1<<4;
  if(!digitalRead(R)) s |= 1<<5;
  if(!digitalRead(ZL)) s |= 1<<6;
  if(!digitalRead(ZR)) s |= 1<<7;
  if(!digitalRead(PLUS)) s |= 1<<8;
  if(!digitalRead(MINUS)) s |= 1<<9;
  if(!digitalRead(LB)) s |= 1<<10;
  if(!digitalRead(RB)) s |= 1<<11;
  if(!digitalRead(UP)) s |= 1<<12;
  if(!digitalRead(DOWN)) s |= 1<<13;
  if(!digitalRead(LEFT)) s |= 1<<14;
  if(!digitalRead(RIGHT)) s |= 1<<15;
  return s;
}

// 状態を書き出し
void recordState(uint16_t state){
  macro += String(state);
  macro += ",";
  macro += String(millis() - lastChange);
  macro += ",";
  lastChange = millis();
}

void applyState(uint16_t s){
  if(s&(1<<0)) Gamepad.press(NSButton_A);
  if(s&(1<<1)) Gamepad.press(NSButton_B);
  if(s&(1<<2)) Gamepad.press(NSButton_X);
  if(s&(1<<3)) Gamepad.press(NSButton_Y);
  if(s&(1<<4)) Gamepad.press(NSButton_LeftThrottle);
  if(s&(1<<5)) Gamepad.press(NSButton_RightThrottle);
  if(s&(1<<6)) Gamepad.press(NSButton_LeftTrigger);
  if(s&(1<<7)) Gamepad.press(NSButton_RightTrigger);
  if(s&(1<<8)) Gamepad.press(NSButton_Plus);
  if(s&(1<<9)) Gamepad.press(NSButton_Minus);
  if(s&(1<<10)) Gamepad.press(NSButton_LeftStick);
  if(s&(1<<11)) Gamepad.press(NSButton_RightStick);
}

void setup() {
  pinMode(MStart1, INPUT_PULLUP);
  pinMode(LB, INPUT_PULLUP);
  pinMode(RB, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(ZL, INPUT_PULLUP);
  pinMode(ZR, INPUT_PULLUP);
  pinMode(L, INPUT_PULLUP);
  pinMode(R, INPUT_PULLUP);
  pinMode(PLUS, INPUT_PULLUP);
  pinMode(MINUS, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(BTN_X, INPUT_PULLUP);
  pinMode(BTN_Y, INPUT_PULLUP);

  analogReadResolution(12);
  Gamepad.begin();
  USB.begin();
}

void loop() {

  // ==== マクロボタン判定 ====
  if(!digitalRead(MStart1)){
    if(pressTime==0) pressTime = millis();
  } else if(pressTime){
    if(millis()-pressTime > 1000){
      recording = !recording;
      if(recording){
        macro="";
        lastState = readButtons();
        lastChange = millis();
      } else {
        recordState(readButtons());
      }
    } else if(!recording && macro.length()>0){
      playing = true;
    }
    pressTime=0;
  }

  // ==== 記録処理 ====
  uint16_t current = readButtons();
  if(recording && current != lastState){
    recordState(lastState);
    lastState = current;
  }

  // ==== 再生処理 ====
  static int index = 0;
  static unsigned long waitUntil = 0;
  if(playing){
    if(millis() > waitUntil){
      int comma = macro.indexOf(",", index);
      if(comma==-1){ playing=false; index=0; return; }
      uint16_t state = macro.substring(index, comma).toInt();
      index = comma+1;
      comma = macro.indexOf(",", index);
      int duration = macro.substring(index, comma).toInt();
      index = comma+1;

      Gamepad.releaseAll();
      applyState(state);
      waitUntil = millis() + duration;
    }
  }

  // ==== 通常入力（記録中はそのまま送信される） ====
  Gamepad.releaseAll();
  Gamepad.leftXAxis(255-axis(LX));
  Gamepad.leftYAxis(255-axis(LY));
  Gamepad.rightXAxis(axis(RX));
  Gamepad.rightYAxis(axis(RY));

  Gamepad.dPad(
    !digitalRead(UP),
    !digitalRead(DOWN),
    !digitalRead(LEFT),
    !digitalRead(RIGHT)
  );

  if(!playing){
    applyState(current);
  }

  Gamepad.loop();
  delay(5);
}
