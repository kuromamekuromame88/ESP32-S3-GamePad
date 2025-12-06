#include <Arduino.h>
#include <BleMouse.h>

// ピン定義
#define VRX1 5  // 移動用スティックX
#define VRY1 4  // 移動用スティックY
#define VRX2 10  // クリック/スクロール用スティックX
#define VRY2 9  // クリック/スクロール用スティックY

BleMouse bleMouse("ESP32 Mouse", "MyMouse", 100);

// ドラッグ・クリック管理用変数
unsigned long leftStartTime = 0;
unsigned long rightStartTime = 0;

bool leftHolding = false;
bool rightHolding = false;

bool leftDragging = false;
bool rightDragging = false;

const int DRAG_THRESHOLD = 200;

int x1_center = 2048;
int y1_center = 2048;
int x2_center = 2048;
int y2_center = 2048;

void setup() {
  Serial.begin(115200);

  pinMode(VRX1, INPUT);
  pinMode(VRY1, INPUT);
  pinMode(VRX2, INPUT);
  pinMode(VRY2, INPUT);

  delay(500);
  // 起動時に中心値をキャリブレーション
  x1_center = analogRead(VRX1);
  y1_center = analogRead(VRY1);
  x2_center = analogRead(VRX2);
  y2_center = analogRead(VRY2);

  Serial.println("Calibration:");
  Serial.printf("X1:%d Y1:%d X2:%d Y2:%d\n", x1_center, y1_center, x2_center, y2_center);

  bleMouse.begin();
}

void loop() {
  if (bleMouse.isConnected()) {
    // === スティック①：マウス移動 ===
    int x1_raw = analogRead(VRX1);
    int y1_raw = analogRead(VRY1);

    int x1_offset = x1_raw - x1_center;
    int y1_offset = y1_raw - y1_center;

    int move_deadzone = 100;
    int dx = 0, dy = 0;

    if (abs(x1_offset) > move_deadzone) {
      dx = map(x1_offset, -2048, 2048, -30, 30);
    }

    if (abs(y1_offset) > move_deadzone) {
      dy = map(y1_offset, -2048, 2048, 30, -30);
    }

    // === スティック②：スクロール・クリック/ドラッグ ===
    int x2 = analogRead(VRX2);
    int y2 = analogRead(VRY2);

    int adjustedY2 = y2 - y2_center;
    int scroll = 0;
    int scroll_deadzone = 100;

    if (adjustedY2 > scroll_deadzone) {
      scroll = map(adjustedY2, scroll_deadzone, 2048, 1, 5);
    } else if (adjustedY2 < -scroll_deadzone) {
      scroll = map(adjustedY2, -scroll_deadzone, -2048, -1, -5);
    }
    // ※ 反転は無し

    // --- クリック用デッドゾーン処理 ---
    const int click_threshold = 400;

    if (x2 < (x2_center - click_threshold)) {
      if (!leftHolding) {
        leftStartTime = millis();
        leftHolding = true;
      } else if (!leftDragging && millis() - leftStartTime > DRAG_THRESHOLD) {
        bleMouse.press(MOUSE_LEFT);
        leftDragging = true;
      }
    } else if (x2 > (x2_center + click_threshold)) {
      if (!rightHolding) {
        rightStartTime = millis();
        rightHolding = true;
      } else if (!rightDragging && millis() - rightStartTime > DRAG_THRESHOLD) {
        bleMouse.press(MOUSE_RIGHT);
        rightDragging = true;
      }
    } else {
      if (leftHolding) {
        if (!leftDragging) {
          bleMouse.click(MOUSE_LEFT);
        } else {
          bleMouse.release(MOUSE_LEFT);
        }
        leftHolding = false;
        leftDragging = false;
      }
      if (rightHolding) {
        if (!rightDragging) {
          bleMouse.click(MOUSE_RIGHT);
        } else {
          bleMouse.release(MOUSE_RIGHT);
        }
        rightHolding = false;
        rightDragging = false;
      }
    }

    // === 実行 ===
    if (dx != 0 || dy != 0 || scroll != 0) {
      bleMouse.move(dx, dy, scroll);
    }

    delay(20);
  }
}
