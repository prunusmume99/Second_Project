#include <Arduino.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // 내장 LED 핀 설정
  Serial.begin(115200);               // 시리얼 통신 시작
  Serial.println("🟢 ESP8266 시작됨!");
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);   // LED ON
  Serial.println("LED ON");
  delay(500);

  digitalWrite(LED_BUILTIN, HIGH);  // LED OFF
  Serial.println("LED OFF");
  delay(500);
}
