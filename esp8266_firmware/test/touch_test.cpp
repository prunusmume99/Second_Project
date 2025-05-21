#include <Arduino.h>

#define TOUCH1_PIN 5     // D1
#define TOUCH2_PIN 4     // D2
#define LED_R_PIN 14    // D5
#define LED_G_PIN 12    // D6

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
    if (touch1State == HIGH)
    {
        digitalWrite(LED_G_PIN, HIGH);
        Serial.println("No.1 Touching");
    }
    else
    {
        digitalWrite(LED_G_PIN, LOW);
        Serial.println("No.1 NoTouch");
    }
    
    int touch2State = digitalRead(TOUCH2_PIN);
    if (touch2State == HIGH)
    {
        digitalWrite(LED_R_PIN, HIGH);
        Serial.println("No.2 Touching");
    }
    else
    {
        digitalWrite(LED_R_PIN, LOW);
        Serial.println("No.2 NoTouch");
    }

    delay(100);
}