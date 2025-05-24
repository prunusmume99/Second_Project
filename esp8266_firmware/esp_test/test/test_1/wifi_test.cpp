#include <ESP8266WiFi.h>

const char* ssid = "turtle";
const char* password = "turtlebot3";

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nWiFi 연결 시도 중...");
  
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt++ < 20) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi 연결 성공!");
    Serial.print("IP 주소: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi 연결 실패!");
  }
}

void loop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 2000) {
    lastCheck = millis();
    wl_status_t status = WiFi.status();
    Serial.print("[WiFi 상태 점검] ");
    if (status == WL_CONNECTED) {
      Serial.println("연결 유지 중 ✅");
    } else {
      Serial.println("연결 끊김 ❌ → 재연결 시도 중...");
      WiFi.begin(ssid, password);
    }
  }
}
