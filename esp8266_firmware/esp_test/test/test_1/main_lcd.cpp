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
const char *server_ip = "192.168.0.82"; // TCP 서버 IP 주소
const uint16_t server_port = 5001;      // TCP 서버 포트 번호

// === NTP 설정 ===
// NTPClient 라이브러리 사용을 위한 UDP 객체 생성
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 9 * 3600); // KST (UTC+9)

// === TCP 클라이언트 설정 ===
WiFiClient client;

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

void loop()
{
    Serial.println("🔌 TCP 서버 연결 시도...");
    if (client.connect(server_ip, server_port))
    {
        Serial.println("✅ TCP 연결 성공");

        // 서버에서 메시지 수신
        String message = client.readStringUntil('\n'); // 한 줄만 수신

        Serial.println("📨 수신: " + message);

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
    }
    else
    {
        Serial.println("❌ TCP 연결 실패");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TCP Fail...");
    }

    delay(3000); // 3초마다 재시도

    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.print(millis() / 1000);
    delay(1000);
}