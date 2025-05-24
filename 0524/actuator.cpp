#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi 설정
const char* WIFI_SSID = "turtle";
const char* WIFI_PASS = "turtlebot3";

// Actuator TCP 서버 포트
const uint16_t ACTUATOR_PORT = 8080;

// I²C LCD 설정 (주소 0x27, 16×2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// AsyncServer 인스턴스 (ESP가 서버 역할)
AsyncServer server(ACTUATOR_PORT);

// 클라이언트 소켓 핸들
AsyncClient* client = nullptr;
String recvBuffer;

// — onData: 클라이언트가 보낸 데이터 수신 시 호출 — 
void onData(void* arg, AsyncClient* c, void* data, size_t len) {
  // 1) 데이터 조각을 누적
  recvBuffer += String((char*)data).substring(0, len);

  int nl;
  // 2) 개행('\n') 단위로 완성된 메시지 처리
  while ((nl = recvBuffer.indexOf('\n')) != -1) {
    String line = recvBuffer.substring(0, nl);
    recvBuffer = recvBuffer.substring(nl + 1);
    line.trim();
    if (line.length() == 0) continue;

    // 3) 시리얼 출력
    Serial.println("[Actuator] Received: " + line);

    // 4) LCD 화면 업데이트
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(1, 0);
    lcd.print(line.substring(0, 15));
    if (line.length() > 15) {
      size_t endIdx = line.length() < 31 ? line.length() : 31;
      lcd.setCursor(0, 1);
      lcd.print(line.substring(15, endIdx));
    }
  }
}

// — onClient: 새로운 클라이언트가 접속했을 때 호출 —
void onClient(void* arg, AsyncClient* c) {
  Serial.println("[Actuator] Client connected");
  client = c;            // 전역에 저장
  recvBuffer = "";       // 버퍼 초기화

  // 이벤트 콜백 등록
  client->onData(onData, nullptr);
  client->onDisconnect([](void*, AsyncClient* c) {
    Serial.println("[Actuator] Client disconnected");
    client = nullptr;
  }, nullptr);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // LCD 초기화
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Actuator Boot");

  // Wi-Fi 연결
  Serial.printf("[Actuator] Connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    Serial.print("[Actuator] Wi-Fi OK → IP=");
    Serial.println(ip);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IP:");
    lcd.print(ip);
  } else {
    Serial.printf("[Actuator] Wi-Fi Failed (st=%d)\n", WiFi.status());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("st=");
    lcd.print(WiFi.status());
    return;  // 더 이상 진행하지 않음
  }

  // TCP 서버 시작
  server.onClient(onClient, nullptr);
  server.begin();
  Serial.printf("[Actuator] TCP server on port %d\n", ACTUATOR_PORT);
}

void loop() {
  // 서버는 내부 루프에서 동작하므로, loop()는 빈 채로 둡니다.
}
