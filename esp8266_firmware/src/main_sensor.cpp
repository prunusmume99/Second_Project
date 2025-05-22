#include <Arduino.h>
#include <SPI.h>         // RFID 리더기와 통신을 위한 Arduino 공식 SPI 라이브러리
#include <MFRC522.h>     // MFRC522 RFID 모듈용 라이브러리
#include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
#include <ESPAsyncTCP.h> // ESP8266 전용 비동기 TCP 통신 라이브러리
#include <WiFiUdp.h>     // UDP 통신을 위한 라이브러리 (NTP 용도)
#include <NTPClient.h>   // 인터넷 시간(NTP 서버) 클라이언트
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

// 등록된 RFID UID 목록
// String authorizedRFIDs[] = {
//     "45 13 217 5",
//     "146 57 157 4",
//     "147 148 214 5"
// };

// === WiFi 설정 ===
const char *ssid = "turtle";
const char *password = "turtlebot3";
const char *DESK_ID = "DESK01"; // 고유한 클라이언트 ID (예: DESK01, DESK02)

// === TCP 클라이언트 설정 ===
AsyncClient client;
const char *remoteHost = "192.168.0.60"; // 라즈베리 파이 서버 IP
const int remotePort = 8080;             // 라즈베리 파이 서버 포트
unsigned long lastClientSend = 0;

// === NTP 설정 ===
// NTPClient 라이브러리 사용을 위한 UDP 객체 생성
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600); // KST (UTC+9)

// === 함수 선언 ===
void handleClientConnect(void *arg, AsyncClient *c);
void handleClientData(void *arg, AsyncClient *c, void *data, size_t len);
void handleClientDisconnect(void *arg, AsyncClient *c);
String getCurrentTimestamp();

// === Sensor Flag 초기화 ===
bool auth_flag = false;
bool touch_flag = false, fsr_flag = false;
bool action_flag = false;

void setup()
{
    Serial.begin(115200);
    delay(500);

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
    client.connect(remoteHost, remotePort);

    timeClient.begin();
    timeClient.update(); // 시간 한번 불러오기

    // NTP 서버 설정 (KST = UTC + 9시간)
    configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("시간 동기화 중...");

    // 시간 동기화 대기
    while (time(nullptr) < 100000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("시간 동기화 완료");

    // pinMode(TOUCH_PIN, INPUT);
}

void loop()
{
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
    // 새로운 카드가 있는지 확인 && 카드가 읽기 가능한지 확인
    {
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
            message += "\"did\":" + String(DESK_ID) + ",";
            message += "\"uid\":" + currentUID + ",";
            message += "\"timestamp\":\"" + tagTime + "\"";
            message += "}";
            client.write(message.c_str());
            Serial.println("Sent to server: " + message);
        }

        // 카드 통신 종료
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();

        delay(1000); // 중복 인식 방지
    }
    else
    {
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

// ISO 8601 시간 문자열을 반환하는 함수
String getCurrentTimestamp()
{
    timeClient.update(); // NTP 시간 갱신

    time_t now = time(nullptr);            // 현재 시간 (Epoch time)
    struct tm *timeinfo = localtime(&now); // 현지 시간 구조체로 변환

    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);

    return String(timestamp); // String 타입으로 반환
}

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <time.h>

const char *ssid = "turtle";
const char *password = "turtlebot3";
const char *CLIENT_ID = "Desk-01";

const char *remoteHost = "192.168.0.60";
const int remotePort = 8080;

#define SS_PIN 2
#define RST_PIN 5
MFRC522 mfrc(SS_PIN, RST_PIN);

AsyncClient client;
String lastUID = "";
unsigned long lastSendTime = 0;

void waitForTimeSync()
{
    Serial.println("⏳ NTP 시간 동기화 대기 중...");
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
        Serial.println("\n✅ NTP 시간 동기화 완료!");
    }
    else
    {
        Serial.println("\n❌ 시간 동기화 실패 (인터넷 또는 DNS 확인)");
    }
}

void handleClientConnect(void *arg, AsyncClient *c)
{
    Serial.println("✅ TCP 서버에 연결됨");
}
void handleClientData(void *arg, AsyncClient *c, void *data, size_t len)
{
    Serial.printf("📩 서버 응답: %.*s\n", len, (char *)data);
}
void handleClientDisconnect(void *arg, AsyncClient *c)
{
    Serial.println("❌ TCP 연결 끊김 → 재연결 시도");
    client.connect(remoteHost, remotePort);
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
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

void loop()
{
    if (mfrc.PICC_IsNewCardPresent() && mfrc.PICC_ReadCardSerial())
    {
        String currentUID = "";
        for (byte i = 0; i < mfrc.uid.size; i++)
        {
            currentUID += String(mfrc.uid.uidByte[i]);
            if (i < mfrc.uid.size - 1)
                currentUID += " ";
        }

        if (currentUID == lastUID && millis() - lastSendTime < 3000)
            return;

        lastUID = currentUID;
        lastSendTime = millis();

        time_t now = time(nullptr);
        if (now < 1000000000)
        {
            Serial.println("❌ 아직 시간 동기화 안 됨 → 전송 취소");
            return;
        }

        struct tm *ptm = localtime(&now);
        if (!ptm)
        {
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
        json += "\"did\":\"" + String(CLIENT_ID) + "\",";
        json += "\"uid\":\"" + currentUID + "\",";
        json += "\"timestamp\":\"" + String(timestamp) + "\"";
        json += "}";

        Serial.println("📨 전송 메시지:");
        Serial.println(json);

        if (client.connected())
        {
            client.write(json.c_str());
        }
        else
        {
            Serial.println("❌ 서버 연결 안 됨 → 재연결");
            client.connect(remoteHost, remotePort);
        }

        mfrc.PICC_HaltA();
        mfrc.PCD_StopCrypto1();
        delay(1000);
    }
}