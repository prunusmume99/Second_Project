#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>

const char* ssid = "turtle";
const char* password = "turtlebot3";
const char* server_ip = "192.168.0.67";
const uint16_t server_port = 5001;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600);

WiFiClient client;

#define SS_PIN 2
#define RST_PIN 5
MFRC522 mfrc(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("WiFi 연결 중");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 연결 완료");
  Serial.print("📡 ESP IP 주소: ");
  Serial.println(WiFi.localIP());


  timeClient.begin();
  timeClient.update();
  configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println("🕒 시간 동기화 중...");
  while (time(nullptr) < 100000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ 시간 동기화 완료");

  SPI.begin();
  mfrc.PCD_Init();
  Serial.println("📡 RFID 리더기 준비 완료. 카드를 태그해주세요.");
}

void loop() {
  timeClient.update();
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char timestamp[25];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);

  if (!mfrc.PICC_IsNewCardPresent()) return;
  if (!mfrc.PICC_ReadCardSerial()) return;

  String currentUID = "";
  for (byte i = 0; i < mfrc.uid.size; i++) {
    currentUID += String(mfrc.uid.uidByte[i]);
    if (i < mfrc.uid.size - 1) currentUID += " ";
  }

  Serial.print("📎 읽은 UID: ");
  Serial.println(currentUID);

  if (client.connect(server_ip, server_port)) {
    Serial.println("✅ TCP 연결 성공");

    String json = "{";
    json += "\"desk_id\":\"D12\",";
    json += "\"event\":\"rfid\",";
    json += "\"value\":1,";
    json += "\"uid\":\"" + currentUID + "\",";
    json += "\"timestamp\":\"" + String(timestamp) + "\"";
    json += "}";

    client.println(json);
    Serial.println("📨 UID 전송 완료");

    unsigned long startTime = millis();
    while (!client.available() && millis() - startTime < 2000) {
      delay(10);
    }

    if (client.available()) {
      String response = client.readStringUntil('\n');
      Serial.print("🔁 서버 응답 수신: ");
      Serial.println(response);

      // 안정적인 비교를 위한 전처리
      response.replace(" ", "");          // 공백 제거
      response.toLowerCase();             // 소문자 변환

      if (response.indexOf("\"result\":\"success\"") >= 0) {
        Serial.println("🎉 인증 성공!");
      } else {
        Serial.println("❌ 인증 실패!");
      }

    } else {
      Serial.println("⚠️ 서버 응답 없음");
    }

    client.stop();
  } else {
    Serial.println("❌ TCP 연결 실패");
  }

  mfrc.PICC_HaltA();
  mfrc.PCD_StopCrypto1();
  delay(1000);
}
