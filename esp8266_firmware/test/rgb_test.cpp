#include <Arduino.h>

const int pinD1 = 5;
const int pinD2 = 4;
// const int pinD3 = 0;		// 사용 지양.
const int pinD4 = 2;
const int pinD5 = 14;
const int pinD6 = 12;
const int pinD7 = 13;

void setup() {
	pinMode(pinD4, OUTPUT);
	pinMode(pinD5, OUTPUT);
	pinMode(pinD6, OUTPUT);
	pinMode(pinD7, OUTPUT);
}

void loop() {
	digitalWrite(pinD4, HIGH);
	digitalWrite(pinD5, HIGH);
	digitalWrite(pinD6, LOW);
	digitalWrite(pinD7, LOW);
	delay(1000);
	digitalWrite(pinD4, LOW);
	digitalWrite(pinD5, LOW);
	digitalWrite(pinD6, HIGH);
	digitalWrite(pinD7, LOW);
	delay(1000);
	digitalWrite(pinD4, HIGH);
	digitalWrite(pinD5, LOW);
	digitalWrite(pinD6, LOW);
	digitalWrite(pinD7, HIGH);
	delay(1000);
	digitalWrite(pinD4, LOW);
	digitalWrite(pinD5, HIGH);
	digitalWrite(pinD6, HIGH);
	digitalWrite(pinD7, HIGH);
	delay(1000);
}