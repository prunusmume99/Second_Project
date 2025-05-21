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
    "180 175 140 4", // 윤진
    "48 207 16 168"  // 예비 카드
};

// === Sensor Flag 초기화 ===
bool auth_flag = 0, touch_flag = 0, fsr_flag = 0;
bool action_flag = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(TOUCH1_PIN, INPUT);
    pinMode(TOUCH2_PIN, INPUT);
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
}

void loop()
{ 
    int touch1State = digitalRead(TOUCH1_PIN);
    int touch2State = digitalRead(TOUCH2_PIN);

    if (touch1State == HIGH)
    {
        digitalWrite(LED_G_PIN, HIGH);
        Serial.println("No.1 Touching");
        if (!fsrFlag)
            fsrFlag = 1;
    }
    else
        digitalWrite(LED_G_PIN, LOW);
    
    if (touch2State == HIGH)
    {
        digitalWrite(LED_R_PIN, HIGH);
        Serial.println("No.2 Touching");
        if (fsrFlag)
            fsrFlag = 0;
    }
    else
        digitalWrite(LED_R_PIN, LOW);

    if (fsrFlag)
    {
        int fsrValue = analogRead(FSR_PIN); // 0 ~ 1023
        Serial.print("FSR Value: ");
        Serial.println(fsrValue);
    }

    delay(100);
}