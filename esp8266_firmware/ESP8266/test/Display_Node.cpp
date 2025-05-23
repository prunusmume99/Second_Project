// ✅ 2번 ESP8266: 디스플레이 노드 (LCD/LED 제어 - Raspberry Pi에서 제어 명령 받음)

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char *ssid = "turtle";
const char *password = "turtlebot3";
const int localPort = 9091;  // 이 노드는 서버가 아님, 클라이언트로 동작할 수 있음

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiServer server(localPort);

#define LED_PIN 16  // D0

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("LCD Ready");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Wi-Fi 연결 완료");
  server.begin();
  Serial.println("📡 수신 대기 중 (LCD/LED)");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String line = client.readStringUntil('\n');
    Serial.println("📨 수신: " + line);
    client.stop();

    // 단순 파싱 처리 (JSON 생략된 간단 구조)
    if (line.indexOf("led_on") >= 0) {
      digitalWrite(LED_PIN, HIGH);
    } else if (line.indexOf("led_off") >= 0) {
      digitalWrite(LED_PIN, LOW);
    } else if (line.indexOf("lcd:") >= 0) {
      String content = line.substring(line.indexOf(":") + 1);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(content);
    }
  }
}
