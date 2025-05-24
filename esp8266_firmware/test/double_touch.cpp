#include <Arduino.h>
#define TOUCH_PIN 4 // D2 (GPIO4)

bool lastTouchState = false;
unsigned long lastTouchTime = 0;
unsigned long firstTouchTime = 0;
bool waitingForSecondTouch = false;

const unsigned long doubleTouchInterval = 400; // 최대 400ms 간격을 더블 터치로 인식

void setup() {
  pinMode(TOUCH_PIN, INPUT);
  Serial.begin(115200);
}

void loop() {
  bool isTouched = digitalRead(TOUCH_PIN) == HIGH;

  if (isTouched && !lastTouchState) { // rising edge: 터치 시작
    unsigned long now = millis();

    if (waitingForSecondTouch && (now - firstTouchTime <= doubleTouchInterval)) {
      // 더블 터치 성공
      Serial.println("Double Touch Detected!" + String(now - firstTouchTime));
      waitingForSecondTouch = false;
    } else {
      // 첫 번째 터치로 간주
      firstTouchTime = now;
      waitingForSecondTouch = true;
    }
  }

  // 시간이 지나면 더블터치 실패 처리
  if (waitingForSecondTouch && millis() - firstTouchTime > doubleTouchInterval) {
    waitingForSecondTouch = false;
  }

  lastTouchState = isTouched;
}