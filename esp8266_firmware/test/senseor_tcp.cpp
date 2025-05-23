#include <Arduino.h>
#include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
#include <ESPAsyncTCP.h> // ESP8266 전용 비동기 TCP 통신 라이브러리
// git clone https://github.com/me-no-dev/ESPAsyncTCP.git
#include <MFRC522.h> // MFRC522 RFID 모듈용 라이브러리
#include <time.h>    // 시간 관련 함수 사용을 위한 라이브러리

#define FSR_PIN A0  // NodeMCU의 아날로그 핀
#define TOUCH_PIN 4 // D2 (GPIO4)
//  === RFID PIN 설정 ===
#define RST_PIN 5 // D1 (GPIO5)
#define SS_PIN 2  // D4 (GPIO2)
// #define SCK_PIN 14      // D5 (GPIO14)
// #define MISO_PIN 12     // D6 (GPIO12)
// #define MOSI_PIN 13     // D7 (GPIO13)

MFRC522 rfid(SS_PIN, RST_PIN);

// Wi-Fi 설정
const char *ssid = "turtle";         // Wi-Fi SSID
const char *password = "turtlebot3"; // Wi-Fi 비밀번호
const char *DESK_ID = "DESK01";      // 고유한 클라이언트 ID (예: ESP1, ESP2)

// TCP 클라이언트 설정
AsyncClient client;
const char *remoteHost = "192.168.0.60"; // 라즈베리 파이 서버 IP
const int remotePort = 5010;             // 라즈베리 파이 서버 포트
unsigned long lastClientSend = 0;

// === 함수 선언 ===
void handleClientConnect(void *arg, AsyncClient *c);
void handleClientData(void *arg, AsyncClient *c, void *data, size_t len);
void handleClientDisconnect(void *arg, AsyncClient *c);

// === Sensor Flag 초기화 ===
bool auth_flag = false;

void setup()
{
    pinMode(TOUCH_PIN, INPUT);

    // === SPI & RFID 초기화 ===
    SPI.begin();     // SPI 통신 선로 준비 (SPI.h)
    rfid.PCD_Init(); // MFR522 레지스터, 설정 등 내부 초기화 (MFRC522.h)

    Serial.begin(115200);

    // Wi-Fi 연결
    WiFi.begin(ssid, password);
    Serial.print("\nTry WiFi connect");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nConnected to WiFi");
    Serial.print(" / IP: ");
    Serial.println(WiFi.localIP());

    // NTP 설정 (KST 기준)
    configTime(9 * 3600, 0, "time.nist.gov");
    Serial.print("Try Time Synchronize");
    // 시간 동기화 대기
    while (time(nullptr) < 100000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nTime Sync Complete");

    // TCP 클라이언트 설정
    client.onConnect(&handleClientConnect, nullptr);
    client.onData(&handleClientData, nullptr);
    client.onDisconnect(&handleClientDisconnect, nullptr);
    client.connect(remoteHost, remotePort);
}

void loop()
{
    if (client.connected() && (millis() - lastClientSend >= 1000))
    {
        int fsrValue = analogRead(FSR_PIN); // 0 ~ 1023
        int touchState = digitalRead(TOUCH_PIN);
        String currentUID = "000 000 00 0";
        if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
        {
            // UID를 문자열로 변환
            currentUID = "";
            for (byte i = 0; i < rfid.uid.size; i++)
            {
                currentUID += String(rfid.uid.uidByte[i]);
                if (i < rfid.uid.size - 1)
                    currentUID += " ";
            }

            // 카드 통신 종료
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();

            delay(1000); // 중복 인식 방지
        }

        // 현재 시간 구조체로 가져오기
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);
        // ISO8601 문자열로 포맷팅
        char timestamp[25];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

        String message = "{";
        message += "\"event\":\"fsr\",";
        message += "\"did\":\"" + String(DESK_ID) + "\",";
        message += "\"uid\":\"" + currentUID + "\",";
        message += "\"touch_value\":" + String(touchState) + ",";
        message += "\"fsr_value\":" + String(fsrValue) + ",";
        message += "\"timestamp\":\"" + String(timestamp) + "\"";
        message += "}";
        client.write((message + "\n").c_str());
        Serial.println("Sent to server: " + message);
        lastClientSend = millis();
    }
}

// 클라이언트 연결 콜백
void handleClientConnect(void *arg, AsyncClient *c)
{
    Serial.println("Connected to server");
}

// 클라이언트 데이터 수신 콜백
void handleClientData(void *arg, AsyncClient *c, void *data, size_t len)
{
    Serial.printf("Data from server: %.*s\n", len, (char *)data);
}

// 클라이언트 연결 해제 콜백
void handleClientDisconnect(void *arg, AsyncClient *c)
{
    Serial.println("Disconnected from server");
    // 재연결 시도
    client.connect(remoteHost, remotePort);
}