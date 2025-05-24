#include <Arduino.h>

const int TOUCH_PIN = D2;
bool touch_flag = true;

enum TouchState {
  IDLE,
  TOUCHING,
  WAITING_FOR_SECOND_TOUCH
};

TouchState state = IDLE;
unsigned long touchStartTime = 0;
unsigned long lastTouchTime = 0;
bool firstTouchDetected = false;

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);
}

void loop() {
  static int touchAction = 0;

  bool isTouched = digitalRead(TOUCH_PIN) == HIGH;
  unsigned long now = millis();

  switch (state) {
    case IDLE:
      if (touch_flag && isTouched) {
        touchStartTime = now;
        state = TOUCHING;
      }
      break;

    case TOUCHING:
      if (!isTouched) {  // 터치 해제됨
        unsigned long duration = now - touchStartTime;

        if (duration >= 5000) {
          touchAction = 3; // Long touch
          state = IDLE;
        } else {
          // 첫 번째 터치로 간주하고 더블터치 대기
          lastTouchTime = now;
          firstTouchDetected = true;
          state = WAITING_FOR_SECOND_TOUCH;
        }
      }
      break;

    case WAITING_FOR_SECOND_TOUCH:
      if (isTouched) {
        if (now - lastTouchTime <= 250) {
          touchAction = 2; // Double touch
        }
        state = IDLE;
        firstTouchDetected = false;
      } else if (now - lastTouchTime > 250) {
        touchAction = 1; // Single short touch
        state = IDLE;
        firstTouchDetected = false;
      }
      break;
  }

  // 결과 출력 및 초기화
  if (touchAction > 0) {
    Serial.println("Touch Count : " + String(touchAction));
    touchAction = 0;
  }

  delay(1); // Watchdog 회피용 (비필수지만 안전하게)
}
