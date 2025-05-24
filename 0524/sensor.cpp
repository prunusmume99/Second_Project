#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ArduinoJson.h>

/////////////////// 설정 영역 ///////////////////
// 본인 AP 정보로 수정하세요
const char* WIFI_SSID     = "turtle";
const char* WIFI_PASS     = "turtlebot3";

// Python 서버 IP/Port (tcp_raspberry.py 가 리스닝 중인 주소)
const char* SERVER_IP     = "192.168.0.65";
const uint16_t SERVER_PORT = 8080;

// 센서 핀
#define FSR_PIN   A0    // 아날로그 FSR
#define TOUCH_PIN D2    // 디지털 터치 센서

// 전역 객체
AsyncClient client;
String recvBuffer;  // 서버로부터 받을 데이터(없을 수도 있음)

// — onData: 서버로부터 데이터(예: 에코)가 올 때
void onData(void* arg, AsyncClient* c, void* data, size_t len) {
  // (필요시 서버 응답을 처리할 수 있습니다)
  recvBuffer += String((char*)data).substring(0, len);
  int nl;
  while ((nl = recvBuffer.indexOf('\n')) != -1) {
    String line = recvBuffer.substring(0, nl);
    recvBuffer = recvBuffer.substring(nl + 1);
    line.trim();
    if (line.length()) {
      Serial.println("[Sensor ESP] Server says: " + line);
    }
  }
}

// — onConnect: TCP 연결 성공 시
void onConnect(void* arg, AsyncClient* c) {
  Serial.println("[Sensor ESP] Connected to server");
}

// — onDisconnect: 연결 끊길 때
void onDisconnect(void* arg, AsyncClient* c) {
  Serial.println("[Sensor ESP] Disconnected, reconnecting in 3s...");
  delay(3000);
  client.connect(SERVER_IP, SERVER_PORT);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // 1) Wi-Fi 연결
  Serial.printf("[Sensor ESP] Connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("[Sensor ESP] Wi-Fi OK → IP=");
  Serial.println(WiFi.localIP());

  // 2) TCP 콜백 등록 및 서버 연결
  client.onData(onData, nullptr);
  client.onConnect(onConnect, nullptr);
  client.onDisconnect(onDisconnect, nullptr);
  client.connect(SERVER_IP, SERVER_PORT);
}

void loop() {
  // 3) 서버에 연결되어 있을 때만 센서 전송
  if (client.connected()) {
    // 센서 값 읽기
    int fsrValue = analogRead(FSR_PIN);
    bool touch   = (digitalRead(TOUCH_PIN) == LOW);

    // JSON으로 포장
    StaticJsonDocument<128> doc;
    doc["event"] = "sensor_data";
    doc["fsr"]   = fsrValue;
    doc["touch"] = touch;
    String out;
    serializeJson(doc, out);

    // TCP로 전송 (\n 으로 메시지 구분)
    client.write(out.c_str(), out.length());
    client.write("\n");

    Serial.printf("[Sensor ESP] Sent → %s\n", out.c_str());
  }

  // 1초 대기
  delay(1000);
}
