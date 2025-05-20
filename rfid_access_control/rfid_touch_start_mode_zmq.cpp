#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>

// ZMQ 메시지를 대신할 TCP 전송 (브릿지 서버로)
// 브릿지 서버가 받은 메시지를 ZMQ PUB으로 브로드캐스트하게 설계

WiFiClient client;

const char* ssid = "turtle";
const char* password = "turtlebot3";
const char* server_ip = "192.168.0.65";  // 브릿지 서버 IP
const uint16_t server_port = 5001;

#define SS_PIN 2       // RFID (D4)
#define RST_PIN 15     // RFID (D8)
#define TOUCH_PIN 4    // 터치 센서 (D2)

MFRC522 mfrc(SS_PIN, RST_PIN);

String authorizedRFIDs[] = {
  "180 175 140 4"
};
bool isAuthorized = false;
int lastTouchState = LOW;
unsigned long lastTouchTime = 0;
int touchCount = 0;
String currentMode = "";

// 시간 동기화용
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600);  // UTC+9

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("📡 WiFi 연결 중...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 연결됨");

  timeClient.begin();
  timeClient.update();

  SPI.begin();
  mfrc.PCD_Init();

  pinMode(TOUCH_PIN, INPUT_PULLUP);

  Serial.println("📡 RFID 리더기 초기화 완료.");
  Serial.println("🪪 등록된 카드: 180 175 140 4");
}

void loop() {
  timeClient.update();
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char timestamp[25];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);

  // ✅ RFID 태그 처리
  if (mfrc.PICC_IsNewCardPresent() && mfrc.PICC_ReadCardSerial()) {
    String currentUID = "";
    for (byte i = 0; i < mfrc.uid.size; i++) {
      currentUID += String(mfrc.uid.uidByte[i]);
      if (i < mfrc.uid.size - 1) currentUID += " ";
    }

    Serial.println("===================================");
    Serial.print("📎 현재 태그된 카드 UID: ");
    Serial.println(currentUID);

    isAuthorized = false;
    for (int i = 0; i < sizeof(authorizedRFIDs) / sizeof(authorizedRFIDs[0]); i++) {
      if (currentUID == authorizedRFIDs[i]) {
        isAuthorized = true;
        break;
      }
    }

    if (isAuthorized) {
      Serial.println("✅ 등록된 사용자입니다.");
      Serial.println("🎮 터치 센서 사용 가능");
    } else {
      Serial.println("⛔ 미등록 카드입니다.");
    }

    mfrc.PICC_HaltA();
    mfrc.PCD_StopCrypto1();
    delay(1000);
  }

  // ✅ 터치 감지 및 모드 진입 (이후 ZMQ 메시지 전송)
  if (isAuthorized) {
    int currentTouch = digitalRead(TOUCH_PIN);

    if (currentTouch != lastTouchState) {
      lastTouchState = currentTouch;

      if (currentTouch == HIGH) {
        Serial.println("👆 터치 감지됨");
        unsigned long now = millis();

        if (now - lastTouchTime < 1000) {
          touchCount++;
        } else {
          touchCount = 1;
        }

        lastTouchTime = now;

        if (touchCount == 2) {
          currentMode = "MODE_1";
          Serial.println("📘 사용자 설정 모드 1 진입 (공부 모드)");

          // ✅ TCP 서버로 JSON 데이터 전송
          if (client.connect(server_ip, server_port)) {
            String json = "{";
            json += "\"desk_id\":\"D12\",";
            json += "\"event\":\"touch_mode\",";
            json += "\"value\":1,";
            json += "\"mode\":\"" + currentMode + "\",";
            json += "\"timestamp\":\"" + String(timestamp) + "\"";
            json += "}";

            client.println(json);
            Serial.println("📨 ZMQ 브릿지 서버로 전송 완료");
            client.stop();
          } else {
            Serial.println("❌ TCP 서버 연결 실패");
          }
        }
      }
    }
  }
}
