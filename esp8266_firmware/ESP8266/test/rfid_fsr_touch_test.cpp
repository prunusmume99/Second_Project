#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <time.h>

const char *ssid = "turtle";
const char *password = "turtlebot3";
const char *CLIENT_ID = "Desk-02";
const char *remoteHost = "192.168.0.60";
const int remotePort = 9090;

#define SS_PIN 2
#define RST_PIN 15
#define TOUCH_PIN 4    // D2
#define FSR_PIN A0
#define LED_PIN 16     // D0

MFRC522 mfrc(SS_PIN, RST_PIN);
AsyncClient client;

String lastUID = "";
unsigned long lastSendTime = 0;
bool isAuthorized = false;
bool sensorActive = false;
int modeState = 0; // 0: none, 1: mode1, 2: mode2
unsigned long authExpireTime = 0;
const unsigned long AUTH_DURATION = 60000;

unsigned long lastTouchTime = 0;
int touchCount = 0;
bool touchPreviouslyHigh = true;

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

void sendToServer(String jsonPayload) {
  String message = String(CLIENT_ID) + ": " + jsonPayload + "\n";
  if (client.connected()) {
    client.write(message.c_str());
    Serial.println("📨 전송: " + message);
  } else {
    Serial.println("❌ 서버 연결 안 됨");
    client.connect(remoteHost, remotePort);
  }
}

void handleTouchInput() {
  bool touchValue = digitalRead(TOUCH_PIN);

  if (touchValue == LOW && touchPreviouslyHigh) {
    touchPreviouslyHigh = false;
    unsigned long now = millis();

    if (now - lastTouchTime < 500) {
      touchCount++;
    } else {
      touchCount = 1;
    }
    lastTouchTime = now;

    if (touchCount == 2) {
      sensorActive = !sensorActive;
      Serial.println(sensorActive ? "🔛 센서 감지 ON" : "⛔ 센서 감지 OFF");
      touchCount = 0;
    } else if (sensorActive && touchCount == 1) {
      modeState = (modeState % 2) + 1;
      Serial.printf("🎮 모드 %d 진입\n", modeState);
      String json = "{\"event\":\"mode\", \"mode\":" + String(modeState) + ", \"uid\":\"" + lastUID + "\"}";
      sendToServer(json);
    }
  } else if (touchValue == HIGH) {
    touchPreviouslyHigh = true;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi 연결 완료");

  configTime(9 * 3600, 0, "pool.ntp.org");
  waitForTimeSync();

  SPI.begin();
  mfrc.PCD_Init();

  client.onConnect(&handleClientConnect, nullptr);
  client.onData(&handleClientData, nullptr);
  client.onDisconnect(&handleClientDisconnect, nullptr);
  client.connect(remoteHost, remotePort);
}

void loop() {
  // RFID 인증
  if (mfrc.PICC_IsNewCardPresent() && mfrc.PICC_ReadCardSerial()) {
    String currentUID = "";
    for (byte i = 0; i < mfrc.uid.size; i++) {
      currentUID += String(mfrc.uid.uidByte[i]);
      if (i < mfrc.uid.size - 1) currentUID += " ";
    }
    if (currentUID == lastUID && millis() - lastSendTime < 3000) return;
    lastUID = currentUID;
    lastSendTime = millis();

    isAuthorized = true;
    authExpireTime = millis() + AUTH_DURATION;
    Serial.println("🟢 인증 성공: 센서 입력 허용");
    mfrc.PICC_HaltA();
    mfrc.PCD_StopCrypto1();
  }

  if (isAuthorized && millis() > authExpireTime) {
    isAuthorized = false;
    sensorActive = false;
    Serial.println("🔒 인증 만료 → 센서 입력 차단");
  }

  if (isAuthorized) {
    handleTouchInput();

    if (sensorActive) {
      int fsrValue = analogRead(FSR_PIN);
      if (fsrValue > 300) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("💺 착석 감지 (LED ON)");
      } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println("💤 미착석 (LED OFF)");
      }
    }
  }

  delay(50);
}
