#include <Arduino.h>

#define TOUCH_PIN 5  // D1 = GPIO5


void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);      // 내장 LED 사용
}

void loop() {
  int touchValue = digitalRead(TOUCH_PIN);

  if (touchValue == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);  // 내장 LED OFF
    Serial.println("터치 감지됨 → LED OFF");
  } else {
    digitalWrite(LED_BUILTIN, LOW); // 내장 LED ON
    Serial.println("터치 없음 → LED ON");
  }

  delay(100);
}
