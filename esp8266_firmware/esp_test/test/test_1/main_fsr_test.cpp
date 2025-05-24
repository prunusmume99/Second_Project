#include <Arduino.h>
#include <SPI.h>          // RFID 리더기와 통신을 위한 Arduino 공식 SPI 라이브러리
#include <ESP8266WiFi.h>  // ESP8266 WiFi 기능을 위한 라이브러리
#include <WiFiUdp.h>      // UDP 통신을 위한 라이브러리 (NTP 용도)
#include <NTPClient.h>    // 인터넷 시간(NTP 서버) 클라이언트
#include <time.h>         // 시간 관련 함수 사용을 위한 라이브러리

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

// === FSR, RGB led 핀 설정 (ESP8266 기준) ===
#define FSR_PIN A0      // NodeMCU의 아날로그 핀
#define LED_R_PIN 14    // D5
#define LED_G_PIN 12    // D6
#define LED_B_PIN 13    // D7

void setup() {
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);

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

    Serial.begin(115200);
    delay(1000);

    // ✅ NTP 서버 설정 (KST = UTC + 9시간)
    configTime(9 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("🕒 시간 동기화 중...");

    // ✅ 시간 동기화 대기
    while (time(nullptr) < 100000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ 시간 동기화 완료");
}

void loop() {
    int fsrValue = analogRead(FSR_PIN); // 0 ~ 1023

    Serial.print("FSR Value: ");
    Serial.println(fsrValue);

    // 압력 판단 예시
    if (fsrValue < 100)
    {
        Serial.println("No pressure");
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_G_PIN, HIGH);
        digitalWrite(LED_B_PIN, HIGH);
    }
    else if (fsrValue < 500)
    {
        Serial.println("Light touch");
        digitalWrite(LED_R_PIN, LOW);
        digitalWrite(LED_G_PIN, LOW);
        digitalWrite(LED_B_PIN, HIGH);
    }
    else
    {
        Serial.println("Heavy pressure");
        digitalWrite(LED_R_PIN, HIGH);
        digitalWrite(LED_G_PIN, LOW);
        digitalWrite(LED_B_PIN, HIGH);
    }

    // 시간 갱신
    timeClient.update();

    // ✅ 현재 시간 구조체로 가져오기
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    // ✅ ISO8601 문자열로 포맷팅
    char timestamp[25];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", timeinfo);

    // Serial.println("✅ 등록된 사용자입니다. TCP 서버에 데이터 전송 시도...");

    // TCP 연결
    if (client.connect(server_ip, server_port))
    {
        Serial.println("✅ TCP 연결 성공");

        String json = "{";
        json += "\"event\":\"fsr\",";
        json += "\"value\":" + String(fsrValue) + ",";
        json += "\"timestamp\":\"" + String(timestamp) + "\"";
        json += "}";

        client.println(json); // 개행 포함 전송
        Serial.println("📨 데이터 전송 완료");

        client.stop();
    }
    else
    {
        Serial.println("❌ TCP 연결 실패");
    }

    delay(1000); // 0.3초 간격으로 측정
}