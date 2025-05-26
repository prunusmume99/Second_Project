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

#define LED_R_PIN 14    // D5
#define LED_G_PIN 12    // D6
#define LED_B_PIN 13    // D7
#define SCL_PIN 5       // D1
#define SDA_PIN 4       // D2
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
time_t parseTimestamp(const String& timestamp);

bool action_flag = false;
bool timer_flag = false;
String status_timestamp = "";
unsigned long lastTimerPrint = 0;

void setup()
{
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(LED_B_PIN, OUTPUT);

    digitalWrite(LED_R_PIN, LOW);
    digitalWrite(LED_G_PIN, LOW);
    digitalWrite(LED_B_PIN, LOW);

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
    if (timer_flag && (status_timestamp != ""))
    {
        if (millis() - lastTimerPrint >= 5000)
        {
            // 현재 status 시작 시간을 time_t로 변환
            time_t target = parseTimestamp(status_timestamp);
            // 현재 시간 구조체로 가져오기
            time_t now = time(nullptr);
            
            long diff = difftime(now, target);  // 초 단위 차이
            long minutes = diff / 60;
            String status = action_flag ? "Study" : "Break";
            String timer_print = "[" + status + "] " + String(minutes) + " Mins";

            setLcdPrintLine(0, "Cur Action Timer");
            setLcdPrintLine(1, timer_print);
            
            lastTimerPrint = millis();
        }
    }
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
            action_flag = false;
            timer_flag = false;
            status_timestamp = "";
            digitalWrite(LED_R_PIN, LOW);
            digitalWrite(LED_G_PIN, LOW);
            digitalWrite(LED_B_PIN, LOW);
        }
        else if (code == 10)
        {
            setLcdPrintLine(0, "Cur Action Timer");
            setLcdPrintLine(1, "[State] 000 Mins");
            timer_flag = true;
        }
        else if (code == 11)
        {
            timer_flag = false;
            setLcdPrintLine(0, "Daily Total Time");
            setLcdPrintLine(1, value);
        }
        else if (code == 12)
        {
            timer_flag = false;
            setLcdPrintLine(0, "Monthly Avg Time");
            setLcdPrintLine(1, value);
        }
    }
    else if (event == "action")
    {
        setLcdPrintLine(0, "Cur Action Timer");
        setLcdPrintLine(1, "[State] 000 Mins");
        status_timestamp = timestamp;
        if (code)
        {
            action_flag = true;
            digitalWrite(LED_R_PIN, HIGH);
            digitalWrite(LED_G_PIN, HIGH);
            digitalWrite(LED_B_PIN, HIGH);
        }
        else
        {
            action_flag = false;
            digitalWrite(LED_R_PIN, HIGH);
            digitalWrite(LED_G_PIN, HIGH);
            digitalWrite(LED_B_PIN, LOW);
        }
        timer_flag = true;
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

// 문자열을 time_t로 변환하는 함수
time_t parseTimestamp(const String& timestamp)
{
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    // 문자열을 구조체에 파싱 (주의: sscanf는 C 함수임)
    sscanf(timestamp.c_str(), "%4d-%2d-%2d %2d:%2d:%2d",
           &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

    tm.tm_year -= 1900; // year since 1900
    tm.tm_mon  -= 1;    // month [0-11]

    return mktime(&tm);
}