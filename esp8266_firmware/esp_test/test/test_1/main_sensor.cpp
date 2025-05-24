#include <Arduino.h>
#include <SPI.h>         // RFID 리더기와 통신을 위한 Arduino 공식 SPI 라이브러리
#include <MFRC522.h>     // MFRC522 RFID 모듈용 라이브러리
#include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
#include <WiFiUdp.h>     // UDP 통신을 위한 라이브러리 (NTP 용도)
#include <NTPClient.h>   // 인터넷 시간(NTP 서버) 클라이언트
#include <time.h>        // 시간 관련 함수 사용을 위한 라이브러리


#define FSR_PIN A0      // NodeMCU의 아날로그 핀
#define TOUCH_PIN 2     // D4 (GPIO2)
// === RFID PIN 설정 ===
#define RST_PIN 5       // D1 (GPIO5)
#define SS_PIN 4        // D2 (GPIO4)
// #define SCK_PIN 14      // D5 (GPIO14)
// #define MISO_PIN 12     // D6 (GPIO12)
// #define MOSI_PIN 13     // D7 (GPIO13)
// #define D3_PIN 0        // D3 (GPIO0)
// #define D8_PIN 15       // D8 (GPIO15)

MFRC522 rfid(SS_PIN, RST_PIN);

// 등록된 RFID UID 목록
String authorizedRFIDs[] = {
    "45 13 217 5",
    "146 57 157 4",
    "147 148 214 5"
};

// === RFID 리더기 식별 번호 ===
const char* deskid = "DESK01";

// === WiFi 설정 ===
const char* ssid = "turtle";
const char* password = "turtlebot3";
const char* server_ip = "192.168.0.82"; // TCP 서버 IP 주소
const uint16_t server_port = 5001;      // TCP 서버 포트 번호

// === NTP 설정 ===
// NTPClient 라이브러리 사용을 위한 UDP 객체 생성
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600);  // KST (UTC+9)

// === TCP 클라이언트 설정 ===
WiFiClient client;

// === Sensor Flag 초기화 ===
bool auth_flag = false;
bool touch_flag = false, fsr_flag = false;
bool action_flag = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // === WiFi 연결 ===
    WiFi.begin(ssid, password);
    Serial.print("WiFi 연결 중");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi 연결됨");

    timeClient.begin();
    timeClient.update(); // 시간 한번 불러오기

    // NTP 서버 설정 (KST = UTC + 9시간)
    configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("🕒 시간 동기화 중...");

    // 시간 동기화 대기
    while (time(nullptr) < 100000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ 시간 동기화 완료");

    pinMode(TOUCH_PIN, INPUT);
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

        Serial.print("📎 your RFID : ");
        Serial.println(currentUID);
        Serial.print("📎 your DESK : ");
        Serial.println(deskid);
        Serial.print("🕒 touch time : ");
        Serial.println(getCurrentTimestamp());

        bool isAuthorized = false;

        // 카드 통신 종료
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();

        delay(1000); // 중복 인식 방지
    }
    else
    {

    }

    delay(100);
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