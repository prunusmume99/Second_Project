#include <Arduino.h>
#include <SPI.h>         // RFID 리더기와 통신을 위한 Arduino 공식 SPI 라이브러리
#include <MFRC522.h>     // MFRC522 RFID 모듈용 라이브러리
#include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
#include <ESPAsyncTCP.h> // ESP8266 전용 비동기 TCP 통신 라이브러리
#include <time.h>        // 시간 관련 함수 사용을 위한 라이브러리

#define FSR_PIN A0 // NodeMCU의 아날로그 핀
#define TOUCH_PIN 4     // D2 (GPIO4)
//  === RFID PIN 설정 ===
#define RST_PIN 5       // D1 (GPIO5)
#define SS_PIN 2        // D4 (GPIO2)
// #define SCK_PIN 14      // D5 (GPIO14)
// #define MISO_PIN 12     // D6 (GPIO12)
// #define MOSI_PIN 13     // D7 (GPIO13)
// #define D3_PIN 0        // D3 (GPIO0)
// #define D8_PIN 15       // D8 (GPIO15)

MFRC522 rfid(SS_PIN, RST_PIN);

// === WiFi 설정 ===
const char *ssid = "turtle";
const char *password = "turtlebot3";
const char *DESK_ID = "DESK01"; // 고유한 클라이언트 ID (예: DESK01, DESK02)

// === TCP 클라이언트 설정 ===
AsyncClient client;
const char *remoteHost = "192.168.0.67"; // 라즈베리 파이 서버 IP
const int remotePort = 8080;             // 라즈베리 파이 서버 포트
unsigned long lastClientSend = 0;
String pendingMessage = ""; // 연결 후 보낼 메시지

// === ping 설정 ===
unsigned long lastPingTime = 0;
const unsigned long PING_INTERVAL = 1000; // 1초 간격

// === 함수 선언 ===
void handleClientConnect(void *arg, AsyncClient *c);
void handleClientData(void *arg, AsyncClient *c, void *data, size_t len);
void handleClientDisconnect(void *arg, AsyncClient *c);
void sendMessageAfterConnect(String msg);
void waitForTimeSync();
String getCurrentTimestamp();

// === Sensor Flag 초기화 ===
bool auth_flag = false;
bool touch_flag = false, fsr_flag = false;
bool action_flag = false;

void setup()
{
    Serial.begin(115200);
    delay(500);
    
    // === SPI & RFID 초기화 ===
    SPI.begin();     // SPI 통신 선로 준비 (SPI.h)
    rfid.PCD_Init(); // MFR522 레지스터, 설정 등 내부 초기화 (MFRC522.h)
    
    pinMode(TOUCH_PIN, INPUT);

    // === WiFi 연결 ===
    WiFi.begin(ssid, password);
    Serial.print("\nWiFi 연결 중");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nWiFi 연결됨");
    Serial.print("  IP: ");
    Serial.println(WiFi.localIP());
    
    // TCP 클라이언트 설정
    client.onConnect(&handleClientConnect, nullptr);
    client.onData(&handleClientData, nullptr);
    client.onDisconnect(&handleClientDisconnect, nullptr);
    if (!client.connected() && !client.connecting()) {
        client.connect(remoteHost, remotePort);
    }

    // NTP 설정 (KST 기준)
    configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    waitForTimeSync();
}

void loop()
{
    if (!auth_flag)
    {
        // 새로운 카드가 있는지 확인
        if (!rfid.PICC_IsNewCardPresent()) return;
        // 카드가 읽기 가능한지 확인
        if (!rfid.PICC_ReadCardSerial()) return;

        // UID를 문자열로 변환
        String currentUID = "";
        for (byte i = 0; i < rfid.uid.size; i++)
        {
            currentUID += String(rfid.uid.uidByte[i]);
            if (i < rfid.uid.size - 1)
                currentUID += " ";
        }
        String tagTime = getCurrentTimestamp();

        Serial.print("your RFID : ");
        Serial.println(currentUID);
        Serial.print("your DESK : ");
        Serial.println(DESK_ID);
        Serial.print("touch time : ");
        Serial.println(tagTime);

        if (client.connected())
        {
            String message = "{";
            message += "\"event\":\"rfid\",";
            message += "\"did\":\"" + String(DESK_ID) + "\",";
            message += "\"uid\":\"" + currentUID + "\",";
            message += "\"timestamp\":\"" + tagTime + "\"";
            message += "}";
            sendMessageAfterConnect(message);
            // client.write(message.c_str());
            // Serial.println("Sent to server: " + message);
        }
        else
        {
            Serial.println("Tcp unconnected!!");
            client.connect(remoteHost, remotePort);
        }

        // 카드 통신 종료
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();

        delay(1000); // 중복 인식 방지
    }
    else
    {
    }

    // === ping 처리 ===
    if (millis() - lastPingTime > PING_INTERVAL)
    {
        lastPingTime = millis();

        if (client.connected())
        {
            String pingMsg = "{";
            pingMsg += "\"event\":\"ping\",";
            pingMsg += "\"did\":\"" + String(DESK_ID) + "\",";
            pingMsg += "\"timestamp\":\"" + getCurrentTimestamp() + "\"";
            pingMsg += "}";
            client.write((pingMsg + "\n").c_str());
            Serial.println("Ping sent: " + pingMsg);
        }
        else
        {
            Serial.println("Ping 실패: TCP 연결 안됨");
            client.connect(remoteHost, remotePort); // 재연결 시도
        }
    }
}

// 클라이언트 연결 콜백
void handleClientConnect(void *arg, AsyncClient *c)
{
    Serial.println("Connected to server");
    if (pendingMessage.length() > 0) {
        client.write((pendingMessage + "\n").c_str());
        Serial.println("Sent after connect: " + pendingMessage);
        pendingMessage = ""; // 전송 완료 후 초기화
    }
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
    client.connect(remoteHost, remotePort);
}

void sendMessageAfterConnect(String msg) {
    if (client.connected()) {
        client.write((msg + "\n").c_str());
        Serial.println("Sent immediately: " + msg);
    } else {
        pendingMessage = msg; // 나중에 전송할 메시지 저장
        client.connect(remoteHost, remotePort); // 연결 시도
    }
}

void waitForTimeSync()
{
    Serial.println("NTP 시간 동기화 대기 중...");
    time_t now = time(nullptr);
    int attempts = 0;
    while (now < 1000000000 && attempts++ < 20)
    {
        delay(500);
        now = time(nullptr);
        Serial.print(".");
    }
    if (now >= 1000000000)
    {
        Serial.println("\nNTP 시간 동기화 완료!");
    }
    else
    {
        Serial.println("\n시간 동기화 실패 (인터넷 또는 DNS 확인)");
    }
}

// ISO 8601 시간 문자열을 반환하는 함수
String getCurrentTimestamp()
{
    time_t now = time(nullptr); // 현재 시간 (Epoch time)
    if (now < 1000000000)
    {
        Serial.println("아직 시간 동기화 안 됨 → 전송 취소");
        return String("1970-01-01일 00:00:00");
    }

    struct tm *timeinfo = localtime(&now); // 현지 시간 구조체로 변환
    if (!timeinfo)
    {
        Serial.println("localtime() 실패 → 전송 취소");
        return String("1970-01-01일 00:00:00");
    }

    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    return String(timestamp); // String 타입으로 반환
}