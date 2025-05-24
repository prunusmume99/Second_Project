#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <SPI.h>
#include <MFRC522.h>

// Wi-Fi 설정
const char* ssid = "yourSSID";
const char* password = "yourPASSWORD";

// 서버(IP, 포트)
const IPAddress serverIP(192,168,1,100);
const uint16_t serverPort = 8080;

// RFID 핀
#define SS_PIN 2
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

// 터치 및 FSR 핀
#define TOUCH_PIN D2
#define FSR_PIN A0

AsyncClient client;
bool prevSeated = false;
unsigned long lastTouchTime = 0;

void handleConnect(void* arg, AsyncClient* c) {
  Serial.println("Connected to server");
}
void handleDisconnect(void* arg, AsyncClient* c) {
  Serial.println("Disconnected, retrying...");
  client.connect(serverIP, serverPort);
}
void handleError(void* arg, AsyncClient* c, int8_t error) {
  Serial.printf("Connect failed: %d\n", error);
}
void handleData(void* arg, AsyncClient* c, void* data, size_t len) {
  Serial.printf("Server: %.*s", len, (char*)data);
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi connected");

  // AsyncClient 이벤트 등록
  client.onConnect(handleConnect, nullptr);
  client.onDisconnect(handleDisconnect, nullptr);
  client.onError(handleError, nullptr);
  client.onData(handleData, nullptr);
  client.connect(serverIP, serverPort);
}

void loop() {
  // 1) RFID 태그 감지 및 전송
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String uid;
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    sendEvent("{\"event\":\"rfid_scan\",\"uid\":\"" + uid + "\"}\n");
  }

  // 2) 터치 센서 싱글/더블 감지
  if (digitalRead(TOUCH_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastTouchTime < 500) {
      sendEvent("{\"event\":\"touch_double\",\"client_id\":\"ESP1\"}\n");
      delay(600);
    } else {
      sendEvent("{\"event\":\"touch_single\",\"client_id\":\"ESP1\"}\n");
    }
    lastTouchTime = now;
  }

  // 3) FSR 값 변화 감지
  bool seated = analogRead(FSR_PIN) > 500;
  if (seated != prevSeated) {
    sendEvent("{\"event\":\"fsr\",\"client_id\":\"ESP1\",\"seated\":" + String(seated) + "}\n");
    prevSeated = seated;
  }

  delay(50);
}

void sendEvent(const String &msg) {
  if (!client.connected()) {
    client.connect(serverIP, serverPort);
  }
  if (client.connected()) {
    client.print(msg);
  }
}