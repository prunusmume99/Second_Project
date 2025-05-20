#include <Arduino.h>


const int FSR_PIN = A0;  // NodeMCU의 아날로그 핀
const int pinD5 = 14;
const int pinD6 = 12;
const int pinD7 = 13;

void setup() {
    Serial.begin(115200);
    pinMode(pinD5, OUTPUT);
    pinMode(pinD6, OUTPUT);
    pinMode(pinD7, OUTPUT);
}

void loop() {
    int fsrValue = analogRead(FSR_PIN); // 0 ~ 1023

    Serial.print("FSR Value: ");
    Serial.println(fsrValue);

    // 압력 판단 예시
    if (fsrValue < 100)
    {
        Serial.println("No pressure");
        digitalWrite(pinD5, HIGH);
        digitalWrite(pinD6, HIGH);
        digitalWrite(pinD7, HIGH);
    }
    else if (fsrValue < 500)
    {
        Serial.println("Light touch");
        digitalWrite(pinD5, LOW);
        digitalWrite(pinD6, LOW);
        digitalWrite(pinD7, HIGH);
    }
    else
    {
        Serial.println("Heavy pressure");
        digitalWrite(pinD5, HIGH);
        digitalWrite(pinD6, LOW);
        digitalWrite(pinD7, HIGH);
    }

    delay(300); // 0.3초 간격으로 측정
}
