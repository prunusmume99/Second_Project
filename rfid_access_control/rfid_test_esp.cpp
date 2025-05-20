#include <SPI.h>
#include <MFRC522.h>

// ESP8266 보드 기준 핀 설정 (GPIO 번호)
#define SS_PIN 2   // D8 → RC522의 SDA
#define RST_PIN 15   // D4 → RC522의 RST

MFRC522 mfrc(SS_PIN, RST_PIN);

// 등록된 RFID UID 목록 (공백으로 구분된 문자열 형태)
String authorizedRFIDs[] = {
  "180 175 140 4",     // 윤진님의 카드 UID
  "48 207 16 168"      // 예비 카드
};

void setup() {
  Serial.begin(115200);
  delay(1000);              // 전원 안정화
  SPI.begin();              // SPI 초기화
  mfrc.PCD_Init();          // RC522 초기화
  Serial.println("📡 RFID 리더기 준비 완료. 카드를 태그해주세요.");
}

void loop() {
  // 카드가 감지되지 않으면 루프 탈출
  if (!mfrc.PICC_IsNewCardPresent()) return;
  if (!mfrc.PICC_ReadCardSerial()) return;

  // 읽은 UID를 문자열로 변환
  String currentUID = "";
  for (byte i = 0; i < mfrc.uid.size; i++) {
    currentUID += String(mfrc.uid.uidByte[i]);
    if (i < mfrc.uid.size - 1) currentUID += " ";
  }

  // UID 출력
  Serial.print("📎 your RFID : ");
  Serial.println(currentUID);

  // 등록된 UID와 비교
  bool isAuthorized = false;
  for (int i = 0; i < sizeof(authorizedRFIDs) / sizeof(authorizedRFIDs[0]); i++) {
    if (currentUID == authorizedRFIDs[i]) {
      isAuthorized = true;
      break;
    }
  }

  if (isAuthorized) {
    Serial.println("✅ Welcome, authorized user!");
    // 여기서 LED 켜기나 부저 울리기 등 제어 가능
  } else {
    Serial.println("⛔ Unauthorized card.");
  }

  // 카드와 통신 종료
  mfrc.PICC_HaltA();
  mfrc.PCD_StopCrypto1();

  delay(1000);  // 중복 감지 방지 딜레이
}