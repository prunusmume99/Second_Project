// 25-02-22 오후 3:12 , 라즈베리파이 SQL 서버에 RFID 태그를 읽고, 읽은 태그의 UID와 현재 시간을 JSON 형식으로 전송하는 코드입니다.

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <time.h>

const char *ssid = "turtle";
const char *password = "turtlebot3";
const char *CLIENT_ID = "Desk-01";

const char *remoteHost = "192.168.0.60";
const int remotePort = 9090;

#define SS_PIN 2
#define RST_PIN 15
MFRC522 mfrc(SS_PIN, RST_PIN);

AsyncClient client;
String lastUID = "";
unsigned long lastSendTime = 0;

void waitForTimeSync() {
  Serial.println("⏳ NTP 시간 동기화 대기 중...");
  time_t now = time(nullptr);
  int attempts = 0;
  while (now < 1000000000 && attempts++ < 20) {
    delay(500);
    now = time(nullptr);
    Serial.print(".");
  }
  if (now >= 1000000000) {
    Serial.println("\n✅ NTP 시간 동기화 완료!");
  } else {
    Serial.println("\n❌ 시간 동기화 실패 (인터넷 또는 DNS 확인)");
  }
}

void handleClientConnect(void *arg, AsyncClient *c) {
  Serial.println("✅ TCP 서버에 연결됨");
}
void handleClientData(void *arg, AsyncClient *c, void *data, size_t len) {
  Serial.printf("📩 서버 응답: %.*s\n", len, (char *)data);
}
void handleClientDisconnect(void *arg, AsyncClient *c) {
  Serial.println("❌ TCP 연결 끊김 → 재연결 시도");
  client.connect(remoteHost, remotePort);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi 연결 완료");

  // ✅ NTP 설정 (KST 기준)
  configTime(9 * 3600, 0, "pool.ntp.org");
  waitForTimeSync();

  // ✅ RFID 초기화
  SPI.begin();
  mfrc.PCD_Init();

  // ✅ TCP 연결 설정
  client.onConnect(&handleClientConnect, nullptr);
  client.onData(&handleClientData, nullptr);
  client.onDisconnect(&handleClientDisconnect, nullptr);
  client.connect(remoteHost, remotePort);
}

void loop() {
  if (mfrc.PICC_IsNewCardPresent() && mfrc.PICC_ReadCardSerial()) {
    String currentUID = "";
    for (byte i = 0; i < mfrc.uid.size; i++) {
      currentUID += String(mfrc.uid.uidByte[i]);
      if (i < mfrc.uid.size - 1) currentUID += " ";
    }

    if (currentUID == lastUID && millis() - lastSendTime < 3000) return;

    lastUID = currentUID;
    lastSendTime = millis();

    time_t now = time(nullptr);
    if (now < 1000000000) {
      Serial.println("❌ 아직 시간 동기화 안 됨 → 전송 취소");
      return;
    }

    struct tm *ptm = localtime(&now);
    if (!ptm) {
      Serial.println("❌ localtime() 실패 → 전송 취소");
      return;
    }

    char timestamp[30];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02d",
             ptm->tm_year + 1900,
             ptm->tm_mon + 1,
             ptm->tm_mday,
             ptm->tm_hour,
             ptm->tm_min,
             ptm->tm_sec);

    String json = "{";
    json += "\"event\":\"rfid_scan\",";
    json += "\"uid\":\"" + currentUID + "\",";
    json += "\"timestamp\":\"" + String(timestamp) + "\"";
    json += "}";

    String message = String(CLIENT_ID) + ": " + json + "\n";
    Serial.println("📨 전송 메시지:");
    Serial.println(message);

    if (client.connected()) {
      client.write(message.c_str());
    } else {
      Serial.println("❌ 서버 연결 안 됨 → 재연결");
      client.connect(remoteHost, remotePort);
    }

    mfrc.PICC_HaltA();
    mfrc.PCD_StopCrypto1();
    delay(1000);
  }
}
