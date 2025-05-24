#include <ESP8266WiFi.h>    // ESP8266 WiFi 연결 라이브러리
#include <ESPAsyncTCP.h>     // 비동기 TCP 클라이언트 라이브러리
#include <SPI.h>             // SPI 통신을 위한 라이브러리
#include <MFRC522.h>         // RFID 리더기 (MFRC522) 라이브러리

// Wi-Fi 네트워크 설정
const char* ssid = "turtle";        // Wi-Fi SSID
const char* password = "turtlebot3"; // Wi-Fi 비밀번호

// 서버(IP, 포트) 설정
const IPAddress serverIP(192,168,0,60); // 라즈베리파이 서버 IP
const uint16_t serverPort = 9090;        // 서버 포트

// RFID 모듈 핀 설정
#define SS_PIN 2  // SDA 핀
#define RST_PIN 15 // Reset 핀
MFRC522 rfid(SS_PIN, RST_PIN);

// 터치 및 FSR 센서 핀 설정
#define TOUCH_PIN D2 // 터치 입력 핀
#define FSR_PIN A0   // FSR 아날로그 입력 핀

// 비동기 TCP 클라이언트 객체
AsyncClient client;

// 이전 착석 상태 저장 변수
bool prevSeated = false;
// 터치 이벤트 타이밍 변수
unsigned long lastTouchTime = 0;

// 서버 연결 성공 시 호출되는 콜백
void handleConnect(void* arg, AsyncClient* c) {
  Serial.println("Connected to server");
}

// 서버 연결 해제 시 호출되는 콜백
void handleDisconnect(void* arg, AsyncClient* c) {
  Serial.println("Disconnected, retrying...");
  client.connect(serverIP, serverPort);  // 재연결 시도
}

// 서버 연결 실패 시 호출되는 콜백
void handleError(void* arg, AsyncClient* c, int8_t error) {
  Serial.printf("Connect failed: %d\n", error);
}

// 서버로부터 데이터 수신 시 호출되는 콜백
void handleData(void* arg, AsyncClient* c, void* data, size_t len) {
  Serial.printf("Server: %.*s", len, (char*)data);
}

void setup() {
  Serial.begin(115200);
  SPI.begin();            // SPI 초기화
  rfid.PCD_Init();        // RFID 리더기 초기화
  pinMode(TOUCH_PIN, INPUT_PULLUP);  // 터치 핀 입력 설정

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi connected");

  // AsyncClient 콜백 등록
  client.onConnect(handleConnect, nullptr);
  client.onDisconnect(handleDisconnect, nullptr);
  client.onError(handleError, nullptr);
  client.onData(handleData, nullptr);

  // 서버에 비동기 연결 시도
  client.connect(serverIP, serverPort);
}

void loop() {
  // 1) RFID 태그 감지 및 전송
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // UID 문자열 생성
    String uid;
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    // JSON 형식으로 이벤트 전송
    sendEvent("{\"event\":\"rfid_scan\",\"uid\":\"" + uid + "\"}\n");
  }

  // 2) 터치 센서 싱글/더블 터치 감지
  if (digitalRead(TOUCH_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastTouchTime < 500) {
      // 더블 터치 이벤트
      sendEvent("{\"event\":\"touch_double\",\"client_id\":\"ESP1\"}\n");
      delay(600);  // 디바운스
    } else {
      // 싱글 터치 이벤트
      sendEvent("{\"event\":\"touch_single\",\"client_id\":\"ESP1\"}\n");
    }
    lastTouchTime = now;
  }

  // 3) FSR 착석/이탈 감지
  bool seated = analogRead(FSR_PIN) > 500;  // 임계값 500 기준
  if (seated != prevSeated) {
    // 착석(true) 또는 이탈(false) 이벤트 전송
    sendEvent("{\"event\":\"fsr\",\"client_id\":\"ESP1\",\"seated\":" + String(seated) + "}\n");
    prevSeated = seated;
  }

  delay(50);  // 메인 루프 딜레이
}

// 이벤트 전송 함수
void sendEvent(const String &msg) {
  // 연결이 끊겨 있으면 재연결
  if (!client.connected()) {
    client.connect(serverIP, serverPort);
  }
  // 연결 상태에서만 전송
  if (client.connected()) {
    client.print(msg);
  }
}