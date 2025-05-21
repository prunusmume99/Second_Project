#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
#include <WiFiUdp.h>     // UDP 통신을 위한 라이브러리 (NTP 용도)
#include <NTPClient.h>   // 인터넷 시간(NTP 서버) 클라이언트
#include <time.h>        // 시간 관련 함수 사용을 위한 라이브러리

// === WiFi 설정 ===
const char *ssid = "turtle";
const char *password = "turtlebot3";
const uint16_t server_port = 6002;      // TCP 서버 포트 번호

// === NTP 설정 ===
// NTPClient 라이브러리 사용을 위한 UDP 객체 생성
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600); // KST (UTC+9)

// ✅ TCP 서버 설정
WiFiServer server(server_port); // Python bridge에서 이 포트로 연결함

// === LCD_I2C_MODULE 핀 설정 (ESP8266 기준) ===
#define SCL_PIN 5 // D1
#define SDA_PIN 4 // D2

// 주소는 보통 0x27 (일부는 0x3F) → I2C 스캐너로 확인 가능
LiquidCrystal_I2C lcd(0x27, 16, 2); // 주소, 열, 행

void setup()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();      // LCD 초기화
    lcd.backlight(); // 백라이트 켜기
    lcd.setCursor(0, 0);
    lcd.print("Hello, ESP8266!");

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

    server.begin(); // ✅ TCP 서버 시작
    Serial.println("✅ TCP 서버 시작됨");
}

void loop()
{
    WiFiClient client = server.available(); // 클라이언트 연결 기다림
    
    if (client)
    {
        Serial.println("📥 클라이언트 연결됨");

        unsigned long startTime = millis();
        while (!client.available()) {
            if (millis() - startTime > 1000) { // 1초 대기 후 타임아웃
                Serial.println("⚠️ 데이터 수신 대기 시간 초과");
                client.stop();
                return;
            }
            delay(10); // 잠깐 대기
        }

        // 데이터가 도착했으므로 읽기 시작
        char buffer[128];
        int len = client.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        buffer[len] = '\0'; // null-terminate
        String message = String(buffer);
        Serial.println("수신 메시지: " + message);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("From Server:");

        lcd.setCursor(0, 1);
        if (message.length() > 16)
        {
            message = message.substring(0, 16); // LCD에 맞게 자르기
        }
        lcd.print(message);

        client.stop();
        delay(500); // LCD에 읽을 시간 확보
    }
    else
    {
        Serial.println("❌ TCP 연결 실패");
        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("TCP Fail...");
        delay(10);
    }

}
