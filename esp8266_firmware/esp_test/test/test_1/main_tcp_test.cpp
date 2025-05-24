#include <ESP8266WiFi.h>

const char* ssid = "turtle";         // 와이파이 이름
const char* password = "turtlebot3"; // 와이파이 비밀번호
const char* server_ip = "192.168.0.67";     // 브릿지 서버의 IP 주소
const uint16_t server_port = 5001;           // 브릿지 서버 포트

WiFiClient client;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("WiFi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 연결됨");

  if (client.connect(server_ip, server_port)) {
    Serial.println("✅ TCP 연결 성공!");

    String json = R"({
      "desk_id": "D12",
      "event": "touch",
      "value": 1,
      "timestamp": "2025-05-18T01:00:00"
    })";

    client.println(json);  // 줄바꿈 포함 전송
    Serial.println("📨 데이터 전송 완료");

    client.stop();
  } else {
    Serial.println("❌ TCP 연결 실패");
  }
}

void loop() {
  // 테스트는 1회성 → loop는 비워둬도 됨
}
