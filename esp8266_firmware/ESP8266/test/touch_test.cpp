#include <Arduino.h>

const int touchPin = 4;  // D2 (GPIO4)

void setup() {
  Serial.begin(115200);
  pinMode(touchPin, INPUT);
}

void loop() {
  int touchState = digitalRead(touchPin);
  
  if (touchState == HIGH) {
    Serial.println("👆 터치 감지됨!");
  } else {
    Serial.println("... 대기 중");
  }

  delay(500);
}
