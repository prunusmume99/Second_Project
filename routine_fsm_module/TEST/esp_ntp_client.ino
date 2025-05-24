#include <ESP8266WiFi.h>
#include <WiFiUdp.h>          // UDP 통신 라이브러리
#include <NTPClient.h>       // NTPClient 라이브러리
#include <SPI.h>             // SPI 통신
#include <MFRC522.h>         // RFID MFRC522 라이브러리

// === WiFi 설정 ===
const char* ssid       = "turtle";        // Wi-Fi SSID
const char* password   = "turtlebot3";    // Wi-Fi 비밀번호

// === NTP 설정 ===
WiFiUDP ntpUDP;                             // UDP 객체
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600);

// === TCP 서버 정보 ===
const char* server_ip   = "192.168.0.67"; // 라즈베리파이 서버 IP
const uint16_t server_port = 5001;         // 서버 포트
WiFiClient client;                          // TCP 클라이언트

// === RFID 핀 설정 ===
#define SS_PIN   2    // SDA (D8)
#define RST_PIN  15   // RST (D4)
MFRC522 mfrc(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  // 1) WiFi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("
WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // 2) NTP 클라이언트 시작
  timeClient.begin();
  timeClient.update();

  // 3) RFID 리더 초기화
  SPI.begin();
  mfrc.PCD_Init();
}

void loop() {
  // 매 루프마다 현재 시간 갱신
  timeClient.update();
  String timestamp = timeClient.getFormattedTime();  // "HH:MM:SS"

  // 1) RFID 스캔 이벤트
  if (mfrc.PICC_IsNewCardPresent() && mfrc.PICC_ReadCardSerial()) {
    // UID 문자열 생성
    String uid;
    for (byte i = 0; i < mfrc.uid.size; i++) {
      uid += String(mfrc.uid.uidByte[i], HEX);
    }
    // JSON 메시지 구성
    String msg = "{\"event\":\"rfid_scan\","
                 "\"uid\":\"" + uid + "\","  
                 "\"time\":\"" + timestamp + "\"}";
    sendJSON(msg);
  }

  // 이후 터치/FSR 이벤트도 동일 방식으로 timestamp 포함 가능
  delay(1000);  // 예시 딜레이
}

// JSON 메시지 전송 함수
void sendJSON(const String &json) {
  if (!client.connected()) {
    if (!client.connect(server_ip, server_port)) {
      Serial.println("Connection failed");
      return;
    }
  }
  Serial.print("Sending: "); Serial.println(json);
  client.println(json);
  // 응답 대기 (선택)
  while (client.available()) {
    String line = client.readStringUntil('
');
    Serial.print("Server: "); Serial.println(line);
  }
  // 연결 종료
  client.stop();
}