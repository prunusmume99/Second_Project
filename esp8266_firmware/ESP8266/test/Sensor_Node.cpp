// ✅ 1번 ESP8266: 센서 노드 (RFID, 터치, FSR → Raspberry Pi TCP 서버로 JSON 전송)

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <time.h>

const char *ssid = "turtle";
const char *password = "turtlebot3";
const char *remoteHost = "192.168.0.60";  // Raspberry Pi
const int remotePort = 9090;

#define SS_PIN 2    // D4
#define RST_PIN 15  // D8
#define TOUCH_PIN 4 // D2
#define FSR_PIN A0

MFRC522 mfrc(SS_PIN, RST_PIN);
AsyncClient client;

String lastUID = "";
bool isAuthorized = false;
bool sensorActive = false;
int modeState = 0;
unsigned long authExpireTime = 0;
const unsigned long AUTH_DURATION = 60000;

unsigned long lastTouchTime = 0;
int touchCount = 0;
bool touchPreviouslyHigh = true;

void sendToServer(String json) {
  if (client.connected()) {
    client.write((json + "\n").c_str());
    Serial.println("📨 전송: " + json);
  } else {
    Serial.println("❌ 서버 연결 안됨");
  }
}

void waitForTimeSync() {
  time_t now = time(nullptr);
  while (now < 1000000000) {
    delay(500);
    now = time(nullptr);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  configTime(9 * 3600, 0, "pool.ntp.org");
  waitForTimeSync();
  SPI.begin();
  mfrc.PCD_Init();
  client.connect(remoteHost, remotePort);
  delay(1000);
}

void loop() {
  if (mfrc.PICC_IsNewCardPresent() && mfrc.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc.uid.size; i++) {
      uid += String(mfrc.uid.uidByte[i]);
      if (i < mfrc.uid.size - 1) uid += " ";
    }
    lastUID = uid;
    isAuthorized = true;
    authExpireTime = millis() + AUTH_DURATION;

    time_t now = time(nullptr);
    struct tm *ptm = localtime(&now);
    char timestamp[30];
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02d",
             ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
             ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    String json = "{\"event\":\"rfid_scan\", \"uid\":\"" + uid + "\", \"timestamp\":\"" + String(timestamp) + "\"}";
    sendToServer(json);

    mfrc.PICC_HaltA();
    mfrc.PCD_StopCrypto1();
  }

  if (isAuthorized && millis() > authExpireTime) {
    isAuthorized = false;
    sensorActive = false;
  }

  if (isAuthorized) {
    bool touchVal = digitalRead(TOUCH_PIN);
    if (touchVal == LOW && touchPreviouslyHigh) {
      touchPreviouslyHigh = false;
      unsigned long now = millis();
      if (now - lastTouchTime < 500) touchCount++;
      else touchCount = 1;
      lastTouchTime = now;

      if (touchCount == 2) {
        sensorActive = !sensorActive;
        touchCount = 0;
      } else if (sensorActive && touchCount == 1) {
        modeState = (modeState % 2) + 1;
        String json = "{\"event\":\"mode\", \"mode\":" + String(modeState) + ", \"uid\":\"" + lastUID + "\"}";
        sendToServer(json);
      }
    } else if (touchVal == HIGH) {
      touchPreviouslyHigh = true;
    }

    if (sensorActive) {
      int fsrValue = analogRead(FSR_PIN);
      String json = "{\"event\":\"fsr\", \"uid\":\"" + lastUID + "\", \"value\":" + String(fsrValue) + "}";
      sendToServer(json);
      delay(1000);
    }
  }
  delay(50);
}
