#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

// === WiFi 설정 ===
const char* ssid = "turtle";
const char* password = "turtlebot3";
const char* server_ip = "192.168.0.67";
const uint16_t server_port = 5001;

WiFiClient client;

// === RFID 핀 설정 (ESP8266 기준) ===
#define SS_PIN 2    // D8 → RC522의 SDA
#define RST_PIN 15  // D4 → RC522의 RST

MFRC522 mfrc(SS_PIN, RST_PIN);

// 등록된 RFID UID 목록
String authorizedRFIDs[] = {
  "180 175 140 4",   // 윤진
  "48 207 16 168"    // 예비 카드
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  // === WiFi 연결 ===
  WiFi.begin(ssid, password);
  Serial.print("WiFi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 연결됨");

  // === SPI & RFID 초기화 ===
  SPI.begin();
  mfrc.PCD_Init();
  Serial.println("📡 RFID 리더기 준비 완료. 카드를 태그해주세요.");
}

void loop() {
  if (!mfrc.PICC_IsNewCardPresent()) return;
  if (!mfrc.PICC_ReadCardSerial()) return;

  // UID를 문자열로 변환
  String currentUID = "";
  for (byte i = 0; i < mfrc.uid.size; i++) {
    currentUID += String(mfrc.uid.uidByte[i]);
    if (i < mfrc.uid.size - 1) currentUID += " ";
  }

  Serial.print("📎 your RFID : ");
  Serial.println(currentUID);

  // 등록된 UID인지 확인
  bool isAuthorized = false;
  for (int i = 0; i < sizeof(authorizedRFIDs) / sizeof(authorizedRFIDs[0]); i++) {
    if (currentUID == authorizedRFIDs[i]) {
      isAuthorized = true;
      break;
    }
  }

  if (isAuthorized) {
    Serial.println("✅ 등록된 사용자입니다. TCP 서버에 데이터 전송 시도...");

    // TCP 연결
    if (client.connect(server_ip, server_port)) {
      Serial.println("✅ TCP 연결 성공");

      // JSON 메시지 예시
      String json = R"({
        "desk_id": "D12",
        "event": "rfid",
        "value": 1,
        "uid": ")" + currentUID + R"(",
        "timestamp": "2025-05-18T01:00:00"
      })";

      client.println(json);  // 개행 포함 전송
      Serial.println("📨 데이터 전송 완료");

      client.stop();
    } else {
      Serial.println("❌ TCP 연결 실패");
    }
  } else {
    Serial.println("⛔ 등록되지 않은 카드입니다.");
  }

  // 카드 통신 종료
  mfrc.PICC_HaltA();
  mfrc.PCD_StopCrypto1();

  delay(1000); // 중복 인식 방지
}
