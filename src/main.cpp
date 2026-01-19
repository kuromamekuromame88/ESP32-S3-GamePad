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

//スティック押し込み
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

// --- フェイスボタン（仮ピン） ---
#define BTN_B 5
#define BTN_A 4
#define BTN_X 7
#define BTN_Y 6

uint8_t axis(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
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
  Gamepad.releaseAll();

  // 左スティック反転（X/Y両方）
  Gamepad.leftXAxis(255 - axis(LX));
  Gamepad.leftYAxis(255 - axis(LY));

  // 右スティック
  Gamepad.rightXAxis(axis(RX));
  Gamepad.rightYAxis(axis(RY));

  //スティックボタン
  if(!digitalRead(LB) Gamepad.press(NSButton_LeftStick);
  if(!digitalRead(RB) Gamepad.press(NSButton_RightStick);
  

  // 十字キー
  Gamepad.dPad(
    !digitalRead(UP),
    !digitalRead(DOWN),
    !digitalRead(LEFT),
    !digitalRead(RIGHT)
  );

  // フェイスボタン
  if(!digitalRead(BTN_A)) Gamepad.press(NSButton_A);
  if(!digitalRead(BTN_B)) Gamepad.press(NSButton_B);
  if(!digitalRead(BTN_X)) Gamepad.press(NSButton_X);
  if(!digitalRead(BTN_Y)) Gamepad.press(NSButton_Y);

  // ZL / ZR
  if(!digitalRead(ZL)) Gamepad.press(NSButton_LeftTrigger);
  if(!digitalRead(ZR)) Gamepad.press(NSButton_RightTrigger);

  //L / R
  if(!digitalRead(L)) Gamepad.press(NSButton_LeftThrottle);
  if(!digitalRead(R)) Gamepad.press(NSButton_RightThrottle);

  // + -
  if(!digitalRead(PLUS) Gamepad.press(NSButton_Plus);
  if(!digitalRead(MINUS) Gamepad.press(NSButton_Minus);
  
  //マクロ
  //マクロの方針 - 通常モード使用中でも記録・再生可能、長押しで記録開始、短押しで再生
  /*
  if(!digitalRead(MStart1))
  */

  Gamepad.loop();
  delay(5);
}
