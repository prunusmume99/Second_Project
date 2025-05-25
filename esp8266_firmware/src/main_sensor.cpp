// #include <Arduino.h>
// #include <ArduinoJson.h>
// // git clone https://github.com/bblanchon/ArduinoJson
// #include <ESP8266WiFi.h> // ESP8266 WiFi 기능을 위한 라이브러리
// #include <ESPAsyncTCP.h> // ESP8266 전용 비동기 TCP 통신 라이브러리
// // git clone https://github.com/me-no-dev/ESPAsyncTCP.git
// #include <MFRC522.h> // MFRC522 RFID 모듈용 라이브러리
// #include <time.h>    // 시간 관련 함수 사용을 위한 라이브러리

// #define FSR_PIN A0  // NodeMCU의 아날로그 핀
// #define TOUCH_PIN 4 // D2 (GPIO4)
// //  === RFID PIN 설정 ===
// #define RST_PIN 5 // D1 (GPIO5)
// #define SS_PIN 2  // D4 (GPIO2)
// // #define SCK_PIN 14      // D5 (GPIO14)
// // #define MISO_PIN 12     // D6 (GPIO12)
// // #define MOSI_PIN 13     // D7 (GPIO13)

// MFRC522 rfid(SS_PIN, RST_PIN);

// // Wi-Fi 설정
// const char *ssid = "turtle";                // Wi-Fi SSID
// const char *password = "turtlebot3";        // Wi-Fi 비밀번호
// const char *DESK_ID = "DESK01";             // 고유한 클라이언트 ID (예: DESK01, DESK02)
// const char *ACTUATOR_IP = "192.168.0.87";   // 페어링되는 액츄에이터의 IP
// String UID = "";

// // TCP 클라이언트 설정
// AsyncClient client;
// const char *remoteHost = "192.168.0.60"; // 라즈베리 파이 서버 IP
// const int remotePort = 5010;             // 라즈베리 파이 서버 포트
// unsigned long lastClientSend = 0;

// // === 함수 선언 ===
// void handleClientConnect(void *arg, AsyncClient *c);
// void handleClientData(void *arg, AsyncClient *c, void *data, size_t len);
// void handleClientDisconnect(void *arg, AsyncClient *c);
// String getCurrentTime();
// void sendToTcpServer(String event, String value = String(0));

// // === Sensor Flag 초기화 ===
// bool auth_flag = false, ping_flag = false;
// bool touch_flag = false, fsr_flag = false;
// bool record_flag = false, action_flag = false;

// // === Touch Sensor Action 판별용 변수 ===
// enum TouchState
// {
//     IDLE,
//     TOUCHING,
//     WAITING_FOR_SECOND_TOUCH,
//     COOLDOWN
// };
// TouchState touchState = IDLE;
// unsigned long touchStartTime = 0;
// unsigned long lastTouchTime = 0;
// unsigned long cooldownStartTime = 0;
// bool firstTouchDetected = false;
// const unsigned long doubleTouchGap = 250;
// const unsigned long longTouchThreshold = 4000;
// const unsigned long cooldownDuration = 200;
// const int LCD_MODE_COUNT = 3;
// int lcd_mode = 0;   // 0 : 현재 측정 중인 시간, 1 : 하루동안 측정된 총 시간, 2 : 월간 평균 패턴

// // === FSR Sensor 평균값 계산 변수 ===
// const int FSR_COUNT = 10;
// int fsrValues[FSR_COUNT] = {0};
// int fsrIndex = 0;
// bool filled = false;

// void setup()
// {
//     pinMode(TOUCH_PIN, INPUT);

//     // === SPI & RFID 초기화 ===
//     SPI.begin();     // SPI 통신 선로 준비 (SPI.h)
//     rfid.PCD_Init(); // MFR522 레지스터, 설정 등 내부 초기화 (MFRC522.h)

//     Serial.begin(115200);

//     // Wi-Fi 연결
//     WiFi.begin(ssid, password);
//     Serial.print("\nTry WiFi connect");
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.print("\nConnected to WiFi");
//     Serial.print(" / IP: ");
//     Serial.println(WiFi.localIP());

//     // NTP 설정 (KST 기준)
//     configTime(9 * 3600, 0, "time.nist.gov");
//     Serial.print("Try Time Synchronize");
//     // 시간 동기화 대기
//     while (time(nullptr) < 100000)
//     {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.println("\nTime Sync Complete");

//     // TCP 클라이언트 설정
//     client.onConnect(&handleClientConnect, nullptr);
//     client.onData(&handleClientData, nullptr);
//     client.onDisconnect(&handleClientDisconnect, nullptr);
//     client.connect(remoteHost, remotePort);
// }

// void loop()
// {
//     static int touchAction = 0;
//     static int fsrSum = 0;
//     static int fsrAverage = 0;

//     if (touch_flag)
//     {
//         bool isTouched = digitalRead(TOUCH_PIN) == HIGH;
//         unsigned long now = millis();

//         switch (touchState)
//         {
//             case IDLE:
//                 if (touch_flag && isTouched)
//                 {
//                     touchStartTime = now;
//                     touchState = TOUCHING;
//                 }
//                 break;

//             case TOUCHING:
//                 if (!isTouched)
//                 {
//                     unsigned long duration = now - touchStartTime;
//                     if (duration >= longTouchThreshold)
//                     {
//                         touchAction = 3;  // Long touch
//                         touchState = COOLDOWN; // 쿨다운 진입
//                         cooldownStartTime = now;
//                     }
//                     else
//                     {
//                         lastTouchTime = now;
//                         firstTouchDetected = true;
//                         touchState = WAITING_FOR_SECOND_TOUCH;
//                     }
//                 }
//                 break;

//             case WAITING_FOR_SECOND_TOUCH:
//                 if (isTouched && (now - lastTouchTime <= doubleTouchGap))
//                 {
//                     touchAction = 2; // Double touch
//                     touchState = COOLDOWN;
//                     cooldownStartTime = now;
//                     firstTouchDetected = false;
//                 }
//                 else if (!isTouched && (now - lastTouchTime > doubleTouchGap))
//                 {
//                     touchAction = 1; // Single short touch
//                     touchState = COOLDOWN;
//                     cooldownStartTime = now;
//                     firstTouchDetected = false;
//                 }
//                 break;
//             case COOLDOWN:
//                 if (now - cooldownStartTime > cooldownDuration)
//                 {
//                     touchState = IDLE;
//                 }
//                 break;
//         }
//     }

//     if (client.connected() && (millis() - lastClientSend >= 1000))
//     {
//         ping_flag = true;

//         if (!auth_flag)
//         {
//             if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
//             {
//                 // 태그한 UID를 문자열로 변환
//                 String tagUID = "";
//                 for (byte i = 0; i < rfid.uid.size; i++)
//                 {
//                     tagUID += String(rfid.uid.uidByte[i]);
//                     if (i < rfid.uid.size - 1)
//                         tagUID += " ";
//                 }

//                 // 카드 통신 종료
//                 rfid.PICC_HaltA();
//                 rfid.PCD_StopCrypto1();

//                 sendToTcpServer("rfid", tagUID);
//                 ping_flag = false;
                
//                 delay(500); // 중복 인식 방지
//             }
//         }

//         if (touch_flag && touchAction > 0)
//         {
//             if (touchAction == 1)       // LCD 출력 모드 변경
//             {
//                 if (record_flag)
//                 {
//                     lcd_mode = (lcd_mode + 1) % LCD_MODE_COUNT;
//                     int touchValue = 10 + lcd_mode;
//                     sendToTcpServer("touch", String(touchValue));
//                     ping_flag = false;
//                 }
//             }
//             else if (touchAction == 2)  // 공부 루틴 기록 시작
//             {
//                 if (!fsr_flag)
//                 {
//                     record_flag = true;
//                     fsr_flag = true;
//                     memset(fsrValues, 0, sizeof(fsrValues));
//                     fsrIndex = 0;
//                     filled = false;
//                     fsrSum = 0;
//                     fsrAverage = 0;

//                     sendToTcpServer("touch", String(touchAction));
//                     ping_flag = false;
//                 }
//             }
//             else if (touchAction == 3)  // 전체 종료
//             {
//                 auth_flag = false;
//                 touch_flag = false;
//                 fsr_flag = false;
//                 record_flag = false;
//                 action_flag = false;
//                 lcd_mode = 0;
                
//                 sendToTcpServer("touch", String(touchAction));
//                 ping_flag = false;
                
//                 UID = "";
//                 Serial.println("Goodbye My User");
//             }

//             touchAction = 0;
//         }

//         if (fsr_flag && record_flag)
//         {
//             int fsrValue = analogRead(FSR_PIN); // 0 ~ 1023

//             fsrSum -= fsrValues[fsrIndex];
//             fsrValues[fsrIndex] = fsrValue;
//             fsrSum += fsrValues[fsrIndex];

//             fsrIndex = (fsrIndex + 1) % FSR_COUNT;
//             if (fsrIndex == 0) filled = true;
        
//             // 평균 계산
//             int count = filled ? FSR_COUNT : fsrIndex;        
//             fsrAverage = fsrSum / count;
//             Serial.print("FSR average (");
//             Serial.print(fsrIndex);
//             Serial.print("): ");
//             Serial.println(fsrAverage);

//             if (action_flag)
//             {
//                 if (fsrAverage < 30)    // 휴식 시간 측정으로 전환
//                 {
//                     action_flag = false;
//                     sendToTcpServer("action", "0");
//                     lcd_mode = 0;
//                     ping_flag = false;
//                 }
//             }
//             else
//             {
//                 if (fsrAverage > 300)   // 공부 시간 측정으로 전환
//                 {
//                     action_flag = true;
//                     sendToTcpServer("action", "1");
//                     lcd_mode = 0;
//                     ping_flag = false;
//                 }
//             }
//         }

//         if (ping_flag)
//         {
//             sendToTcpServer("ping");
//         }

//         lastClientSend = millis();
//     }
// }

// // 클라이언트 연결 콜백
// void handleClientConnect(void *arg, AsyncClient *c)
// {
//     Serial.println("Connected to server");
// }

// // 클라이언트 데이터 수신 콜백
// void handleClientData(void *arg, AsyncClient *c, void *data, size_t len)
// {
//     Serial.printf("Data from server: %.*s\n", len, (char *)data);

//     String jsonStr = String((char *)data).substring(0, len);
//     jsonStr.trim();  // 개행 문자 제거

//     StaticJsonDocument<512> resp;  // 필요한 크기 조정 가능
//     DeserializationError error = deserializeJson(resp, jsonStr);

//     if (error)
//     {
//         Serial.print("JSON 파싱 실패: ");
//         Serial.println(error.c_str());
//         return;
//     }

//     // 필드 추출
//     String event = resp["event"];
//     String actuIP = resp["actuIP"];
//     String did = resp["did"];
//     String uid = resp["uid"];
//     int value = resp["value"];
//     String timestamp = resp["timestamp"];

//     // 사용 예시
//     Serial.print("event: " + event);
//     Serial.print("\tactuIP: " + actuIP);
//     Serial.print("\tdid: " + did);
//     Serial.print("\tuid: " + uid);
//     Serial.print("\tvalue: " + String(value));
//     Serial.println("\ttimestamp: " + timestamp);

//     if (event == "rfid")
//     {
//         if (value)
//         {
//             auth_flag = true;
//             touch_flag = true;
//             UID = uid;
//             Serial.println("Welcome My User");
//         }
//         else
//         {
//             Serial.println("Please Don't Tag");
//         }
//     }
// }

// // 클라이언트 연결 해제 콜백
// void handleClientDisconnect(void *arg, AsyncClient *c)
// {
//     Serial.println("Disconnected from server");
//     // 재연결 시도
//     client.connect(remoteHost, remotePort);
// }

// String getCurrentTime()
// {
//     // 현재 시간 구조체로 가져오기
//     time_t now = time(nullptr);
//     struct tm *timeinfo = localtime(&now);
//     // ISO8601 문자열로 포맷팅
//     char timestamp[25];
//     strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

//     return String(timestamp);
// }

// void sendToTcpServer(String event, String value)
// {
//     String message = "{";
//     message += "\"event\":\"" + event + "\",";
//     message += "\"actuIP\":\"" + String(ACTUATOR_IP) + "\",";
//     message += "\"did\":\"" + String(DESK_ID) + "\",";
//     message += "\"uid\":\"" + (event == "rfid" ? value : UID) + "\",";
//     message += "\"value\":" + (event == "rfid" ? "0" : value) + ",";
//     message += "\"timestamp\":\"" + getCurrentTime() + "\"";
//     message += "}";
//     client.write((message + "\n").c_str());
//     Serial.println("Sent to server: " + message);
// }