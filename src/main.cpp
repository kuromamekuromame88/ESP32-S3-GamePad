#include <Arduino.h>
#include "switch_ESP32.h"

NSGamepad Gamepad;

// --- スティック ---
#define LX 18
#define LY 17
#define RX 15
#define RY 16

// --- 十字キー ---
#define UP 9
#define RIGHT 3
#define DOWN 10
#define LEFT 7

// --- ショルダー ---
#define ZL 2
#define ZR 1

// --- フェイスボタン（仮ピン） ---
#define BTN_B 4
#define BTN_A 5
#define BTN_X 6
#define BTN_Y 7

uint8_t axis(int pin){
  return map(analogRead(pin), 0, 4095, 0, 255);
}

void setup() {
  pinMode(UP, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(ZL, INPUT_PULLUP);
  pinMode(ZR, INPUT_PULLUP);

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

  Gamepad.loop();
  delay(5);
}
