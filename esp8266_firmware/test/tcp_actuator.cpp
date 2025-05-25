#include <Arduino.h>
#include <ArduinoJson.h>
// git clone https://github.com/bblanchon/ArduinoJson
#include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
#include <ESPAsyncTCP.h> // ESP8266 전용 비동기 TCP 통신 라이브러리
// git clone https://github.com/me-no-dev/ESPAsyncTCP.git
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// git clone https://github.com/johnrickman/LiquidCrystal_I2C.git
#include <time.h>        // 시간 관련 함수 사용을 위한 라이브러리

#define SCL_PIN 5  // D1
#define SDA_PIN 4  // D2
// I2C 주소는 보통 0x27 (일부는 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);     // 주소, 열, 행

// 네트워크 설정
const char *ssid = "turtle";            // Wi-Fi SSID
const char *password = "turtlebot3";    // Wi-Fi 비밀번호
const uint16_t ACTUATOR_PORT = 7010;    // Actuator TCP 서버 포트
AsyncServer server(ACTUATOR_PORT);      // AsyncServer 인스턴스 (ESP가 서버 역할)

// 클라이언트 소켓 핸들
AsyncClient *client = nullptr;
String recvBuffer;

// === 함수 선언 ===
void onData(void *arg, AsyncClient *c, void *data, size_t len);
void onClient(void *arg, AsyncClient *c);
void setLcdPrintLine(int line, String value);

bool action_flag = false;
String action_timestamp = "";

void setup()
{
    Serial.begin(115200);
    delay(100);

    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();         // LCD initialize
    lcd.backlight();    // backlight ON
    lcd.clear();
    setLcdPrintLine(0, "Actuator Booting");
    
    // Wi-Fi 연결
    Serial.printf("[Actuator] Connecting to %s\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    setLcdPrintLine(1, "Try WiFi Connect");
    Serial.print("[Actuator] Try WiFi Connect");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\n[Actuator] Connected to WiFi");
    Serial.print(" / IP: ");
    Serial.println(WiFi.localIP());
    setLcdPrintLine(0, "IP:");
    lcd.print(WiFi.localIP());
    setLcdPrintLine(1, "Connect Sucess!!");

    // NTP 설정 (KST 기준)
    configTime(9 * 3600, 0, "time.nist.gov");
    Serial.print("[Actuator] Try Time Synchronize");
    // 시간 동기화 대기
    while (time(nullptr) < 100000)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[Actuator] Time Sync Complete");

    // TCP 서버 시작
    server.onClient(onClient, nullptr);
    server.begin();
    Serial.printf("[Actuator] TCP server on port %d\n", ACTUATOR_PORT);

    setLcdPrintLine(1, "Please Tag RFID.");
}

void loop()
{
    // 서버는 내부 루프에서 동작하므로, loop()는 빈 채로 둡니다.
}

// 클라이언트가 보낸 데이터 수신 시 호출
void onData(void *arg, AsyncClient *c, void *data, size_t len)
{
    Serial.printf("[Actuator] Received: %.*s\n", len, (char *)data);

    String jsonStr = String((char *)data).substring(0, len);
    jsonStr.trim();  // 개행 문자 제거

    StaticJsonDocument<512> resp;  // 필요한 크기 조정 가능
    DeserializationError error = deserializeJson(resp, jsonStr);

    if (error)
    {
        Serial.print("JSON 파싱 실패: ");
        Serial.println(error.c_str());
        return;
    }

    // 필드 추출
    String event = resp["event"];
    int code = resp["code"];
    String value = resp["value"];
    String timestamp = resp["timestamp"];

    // 사용 예시
    Serial.print("event: " + event);
    Serial.print("\tcode: " + code);
    Serial.print("\tvalue: " + value);
    Serial.println("\ttimestamp: " + timestamp);

    if (event == "rfid")
    {
        if (code)
        {
            setLcdPrintLine(0, "Welcome My User!");
            setLcdPrintLine(1, "Plz Double Touch");
        }
        else
        {
            setLcdPrintLine(0, "Access Denied :(");
            setLcdPrintLine(1, "Please Tag RFID.");
        }
    }
    else if (event == "touch")
    {
        if (code == 2)
        {
            setLcdPrintLine(0, "Recording Start!");
            setLcdPrintLine(1, "Plz Take a Seat~");
        }
        else if (code == 3)
        {
            setLcdPrintLine(0, "Goodbye My User!");
            setLcdPrintLine(1, "Please Tag RFID.");
        }
        else if (code == 10)
        {
            setLcdPrintLine(0, "Cur Action Timer");
            setLcdPrintLine(1, value);
        }
        else if (code == 11)
        {
            setLcdPrintLine(0, "Daily Total Time");
            setLcdPrintLine(1, value);
        }
        else if (code == 12)
        {
            setLcdPrintLine(0, "Monthly Avg Time");
            setLcdPrintLine(1, value);
        }
    }
    else
    {
        setLcdPrintLine(0, "==ReceivedData==");
        setLcdPrintLine(1, value); 
    }   
}

// 새로운 클라이언트가 접속했을 때 호출
void onClient(void *arg, AsyncClient *c)
{
    Serial.println("[Actuator] Client connected");
    client = c;      // 전역에 저장
    recvBuffer = ""; // 버퍼 초기화

    // 이벤트 콜백 등록
    client->onData(onData, nullptr);
    client->onDisconnect([](void *, AsyncClient *c) {
            Serial.println("[Actuator] Client disconnected");
            client = nullptr; 
        }, nullptr);
}

// LCD를 줄 단위로 리셋 후, 출력
void setLcdPrintLine(int line, String value)
{
    lcd.setCursor(0, line);
    lcd.print("                ");
    lcd.setCursor(0, line);
    lcd.print(value);
}