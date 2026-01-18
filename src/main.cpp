#include <Arduino.h>
#include "switch_ESP32.h"

NSGamepad Gamepad;

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

  analogReadResolution(12);

  Gamepad.begin();
  USB.begin();   // ← サンプル通りここでUSB開始
}

void loop() {
  Gamepad.releaseAll();

  Gamepad.leftXAxis(axis(LX));
  Gamepad.leftYAxis(axis(LY));
  Gamepad.rightXAxis(axis(RX));
  Gamepad.rightYAxis(axis(RY));

  Gamepad.dPad(
    !digitalRead(UP),
    !digitalRead(DOWN),
    !digitalRead(LEFT),
    !digitalRead(RIGHT)
  );

  if(!digitalRead(ZL)) Gamepad.press(NSButton_LeftTrigger);
  if(!digitalRead(ZR)) Gamepad.press(NSButton_RightTrigger);

  Gamepad.loop();   // ← これが公式の送信処理
  delay(5);
}
