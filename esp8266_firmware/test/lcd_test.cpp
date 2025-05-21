#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SCL_PIN 5  // D1
#define SDA_PIN 4  // D2

// 주소는 보통 0x27 (일부는 0x3F) → I2C 스캐너로 확인 가능
LiquidCrystal_I2C lcd(0x27, 16, 2); // 주소, 열, 행

void setup() {
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();         // LCD 초기화
    lcd.backlight();    // 백라이트 켜기
    lcd.setCursor(0, 0);
    lcd.print("Hello, ESP8266!");
}

void loop() {
    lcd.setCursor(0, 1);
    lcd.print("Time: ");
    lcd.print(millis() / 1000);
    delay(1000);
}

