#include <Arduino.h>

  void setup() {
  Serial.begin(115200);
  delay(5000);  // 부팅 후 안정 대기
  Serial.println("사용자 모드");  // 첫 줄에 사용자 모드 출력
  Serial.flush();
  delay(2000);   
}

void loop() {
  static unsigned long startTime = millis();
  unsigned long elapsedSeconds = (millis() - startTime) / 1000;
  Serial.print("총 공부 시간: ");
  Serial.print(elapsedSeconds);
  Serial.println(" 초");
  delay(1000);
}
