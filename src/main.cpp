#include <Arduino.h>
#include "switch_ESP32.h"

NSGamepad pad;

// --- ピン ---
#define LX 18
#define LY 17
#define RX 15
#define RY 16

#define UP 9
#define RIGHT 3
#define DOWN 10
#define LEFT 7

#define ZL 2
#define ZR 1

// ADCをSwitch形式へ変換（中央128）
uint8_t axis(int pin){
  int v = analogRead(pin);
  return map(v, 0, 4095, 0, 255);
}

void setup() {
  pinMode(UP, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(LEFT, INPUT_PULLUP);
  pinMode(ZL, INPUT_PULLUP);
  pinMode(ZR, INPUT_PULLUP);

  analogReadResolution(12);

  pad.begin();
}

void loop() {
  pad.releaseAll();

  // スティック
  pad.leftXAxis(axis(LX));
  pad.leftYAxis(axis(LY));
  pad.rightXAxis(axis(RX));
  pad.rightYAxis(axis(RY));

  // 十字キー
  pad.dPad(
    !digitalRead(UP),
    !digitalRead(DOWN),
    !digitalRead(LEFT),
    !digitalRead(RIGHT)
  );

  // ZL / ZR
  if(!digitalRead(ZL)) pad.press(NSButton_LeftTrigger);
  if(!digitalRead(ZR)) pad.press(NSButton_RightTrigger);

  pad.write();
  delay(5);
}
