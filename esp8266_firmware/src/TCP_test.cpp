#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

// Wi-Fi 설정
const char* ssid = "yourSSID";       // Wi-Fi SSID
const char* password = "yourPASSWORD"; // Wi-Fi 비밀번호
const char* CLIENT_ID = "ESP1";       // 고유한 클라이언트 ID (예: ESP1, ESP2)

// TCP 클라이언트 설정
AsyncClient client;
const char* remoteHost = "192.168.1.100"; // 라즈베리 파이 서버 IP
const int remotePort = 8080;              // 라즈베리 파이 서버 포트
unsigned long lastClientSend = 0;

// 클라이언트 연결 콜백
void handleClientConnect(void* arg, AsyncClient* c) {
  Serial.println("Connected to server");
}

// 클라이언트 데이터 수신 콜백
void handleClientData(void* arg, AsyncClient* c, void* data, size_t len) {
  Serial.printf("Data from server: %.*s\n", len, (char*)data);
}

// 클라이언트 연결 해제 콜백
void handleClientDisconnect(void* arg, AsyncClient* c) {
  Serial.println("Disconnected from server");
  // 재연결 시도
  client.connect(remoteHost, remotePort);
}

void setup() {
  Serial.begin(115200);

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // TCP 클라이언트 설정
  client.onConnect(&handleClientConnect, nullptr);
  client.onData(&handleClientData, nullptr);
  client.onDisconnect(&handleClientDisconnect, nullptr);
  client.connect(remoteHost, remotePort);
}

void loop() {
  // 주기적으로 서버에 데이터 전송 (5초마다)
  if (client.connected() && (millis() - lastClientSend >= 5000)) {
    String message = String(CLIENT_ID) + ": Sensor data " + String(random(10, 30)) + "\n";
    client.write(message.c_str());
    Serial.println("Sent to server: " + message);
    lastClientSend = millis();
  }
}