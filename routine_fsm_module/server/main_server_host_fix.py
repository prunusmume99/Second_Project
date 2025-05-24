import socket
import json
from datetime import datetime
import mysql.connector

# ============ MySQL 연결 설정 ============
DB_CONFIG = {
    'host': 'localhost',
    'user': 'bangme',
    'password': 'djwls123',
    'database': 'study_db'
}

# ============ 루틴 상태 클래스 ============
class RoutineManager:
    def __init__(self):
        self.active = False
        self.seated = False
        self.start_time = None
        self.authorized = False

    def authorize_user(self, uid):
        # 테스트용으로 UID는 무조건 "C123456"만 통과
        self.authorized = (uid == "C123456")
        print(f"{'✅' if self.authorized else '❌'} UID {uid} 인증 {'성공' if self.authorized else '실패'}")
        return {'authorized': self.authorized}

    def toggle_routine(self):
        if not self.authorized:
            print("🚫 인증되지 않은 사용자입니다.")
            return {'error': 'not_authorized'}

        self.active = not self.active
        print(f"{'✅ 루틴 시작' if self.active else '🛑 루틴 종료'}")
        return {'active': self.active}

    def seat_change(self, card_num, seated_now):
        if not self.active:
            return {'error': 'routine_not_active'}

        if seated_now and not self.seated:
            self.start_time = datetime.now()
            print("💺 착석 감지:", self.start_time)
            resp = {'seated': True, 'time': self.start_time.isoformat()}

        elif not seated_now and self.seated:
            end_time = datetime.now()
            print("🚶 이탈 감지:", end_time)
            insert_record(card_num, 1, self.start_time, end_time)
            resp = {
                'seated': False,
                'logged': True,
                'start': self.start_time.isoformat(),
                'end': end_time.isoformat()
            }
            self.start_time = None

        else:
            resp = {'seated': seated_now}

        self.seated = seated_now
        return resp

# ============ DB 기록 함수 ============
def insert_record(card_num, status, start_time, end_time):
    conn = mysql.connector.connect(**DB_CONFIG)
    cursor = conn.cursor()
    try:
        query = """
        INSERT INTO Record_Table (Card_Num, Status, Start_Time, End_Time)
        VALUES (%s, %s, %s, %s)
        """
        cursor.execute(query, (card_num, status, start_time, end_time))
        conn.commit()
        print(f"✅ 기록 저장: {card_num} | {start_time} ~ {end_time}")
    except Exception as e:
        print("❌ DB 오류:", e)
    finally:
        cursor.close()
        conn.close()

# ============ TCP 서버 시작 ============
HOST = '0.0.0.0'   # ESP8266 코드의 remoteHost와 동일하게
PORT = 9090            # ESP8266 코드의 remotePort와 동일하게
manager = RoutineManager()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.bind((HOST, PORT))
    server.listen()
    print(f"📡 TCP 서버 실행 중: {HOST}:{PORT} ...")

    while True:
        conn, addr = server.accept()
        with conn:
            print(f"📥 연결됨: {addr}")
            raw = conn.recv(1024).decode()
            if not raw:
                continue

            try:
                msg = json.loads(raw)
                card = msg.get("card_num", "C123456")
                event = msg.get("event")

                if event == "rfid_scan":
                    resp = manager.authorize_user(msg.get("uid", ""))
                elif event == "touch_toggle":
                    resp = manager.toggle_routine()
                elif event == "fsr":
                    resp = manager.seat_change(card, msg.get("seated", False))
                else:
                    resp = {'error': 'unknown_event'}

            except Exception as e:
                resp = {'error': f'json_parse_failed: {e}'}

            reply = json.dumps(resp)
            conn.sendall(reply.encode('utf-8'))
            print("📤 Response:", reply)
