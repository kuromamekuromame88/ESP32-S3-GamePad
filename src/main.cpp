#include <Arduino.h>
#include "switch_ESP32.h"

SwitchController Gamepad;

// --- ピン定義 ---
#define PIN_LX 18
#define PIN_LY 17
#define PIN_RX 15
#define PIN_RY 16

#define PIN_UP 9
#define PIN_RIGHT 3
#define PIN_DOWN 10
#define PIN_LEFT 7

#define PIN_ZL 2
#define PIN_ZR 1

// ADC → Switch用範囲へ変換（0〜4095 → -32768〜32767）
int16_t readStick(int pin) {
  int v = analogRead(pin);
  return map(v, 0, 4095, -32768, 32767);
}

void setup() {
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_ZL, INPUT_PULLUP);
  pinMode(PIN_ZR, INPUT_PULLUP);

  analogReadResolution(12);

  Gamepad.begin();
}

void loop() {
  // スティック
  Gamepad.lStick(readStick(PIN_LX), readStick(PIN_LY));
  Gamepad.rStick(readStick(PIN_RX), readStick(PIN_RY));

  // 十字キー
  Gamepad.dpad(
    !digitalRead(PIN_UP),
    !digitalRead(PIN_DOWN),
    !digitalRead(PIN_LEFT),
    !digitalRead(PIN_RIGHT)
  );

  // ショルダー
  Gamepad.buttonZL(!digitalRead(PIN_ZL));
  Gamepad.buttonZR(!digitalRead(PIN_ZR));

  Gamepad.sendReport();
  delay(5);
}
