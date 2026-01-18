#include <Arduino.h>
#include "USB.h"
#include "USBHIDGamepad.h"

USBHIDGamepad gamepad;

// --- GPIO（あとで変更可） ---
const int dpadUp    = 9;
const int dpadDown  = 10;
const int dpadLeft  = 11;
const int dpadRight = 3;

const int btnA = 5;
const int btnB = 4;
const int btnX = 6;
const int btnY = 7;

const int btnZL = 2;
const int btnZR = 1;

const int LX = 18;
const int LY = 17;
const int RX = 15;
const int RY = 16;

// --- HAT計算 ---
uint8_t getHat() {
  bool u = !digitalRead(dpadUp);
  bool d = !digitalRead(dpadDown);
  bool l = !digitalRead(dpadLeft);
  bool r = !digitalRead(dpadRight);

  if (u && r) return 1;
  if (r && d) return 3;
  if (d && l) return 5;
  if (l && u) return 7;
  if (u) return 0;
  if (r) return 2;
  if (d) return 4;
  if (l) return 6;
  return 8;
}

// --- ADC ---
int readAxis(int pin) {
  int raw = analogRead(pin);
  return map(raw, 0, 4095, -127, 127);
}

void setup() {
  pinMode(dpadUp, INPUT_PULLUP);
  pinMode(dpadDown, INPUT_PULLUP);
  pinMode(dpadLeft, INPUT_PULLUP);
  pinMode(dpadRight, INPUT_PULLUP);

  pinMode(btnA, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
  pinMode(btnX, INPUT_PULLUP);
  pinMode(btnY, INPUT_PULLUP);
  pinMode(btnZL, INPUT_PULLUP);
  pinMode(btnZR, INPUT_PULLUP);

  analogReadResolution(12);

  USB.begin();
  gamepad.begin();
}

void loop() {
  gamepad.releaseAll();

  if (!digitalRead(btnA)) gamepad.pressButton(1);
  if (!digitalRead(btnB)) gamepad.pressButton(2);
  if (!digitalRead(btnX)) gamepad.pressButton(3);
  if (!digitalRead(btnY)) gamepad.pressButton(4);
  if (!digitalRead(btnZL)) gamepad.pressButton(5);
  if (!digitalRead(btnZR)) gamepad.pressButton(6);

  gamepad.leftStick(readAxis(LX), readAxis(LY));
  gamepad.rightStick(readAxis(RX), readAxis(RY));

  gamepad.hat(getHat());
  gamepad.sendReport();

  delay(5);
}
