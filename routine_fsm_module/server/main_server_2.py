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

# ============ 루틴 및 모드 관리 클래스 ============
class RoutineManager:
    def __init__(self):
        self.authorized = False
        self.active = False
        self.seated = False
        self.start_time = None
        self.mode = 0  # 0: OFF, 1: 모드1, 2: 모드2

    def authorize_user(self, uid):
        # 테스트용 UID 무조건 통과
        self.authorized = (uid == "C123456")
        print(f"{'✅' if self.authorized else '❌'} UID {uid} 인증 {'성공' if self.authorized else '실패'}")
        return {'authorized': self.authorized}

    def toggle_active(self):
        if not self.authorized:
            return {'error': 'not_authorized'}
        self.active = not self.active
        if self.active:
            self.mode = 0
            print("✅ 루틴 ON")
        else:
            self.mode = 0
            self.seated = False
            self.start_time = None
            print("🛑 루틴 OFF (센서 비활성화)")
        return {'active': self.active}

    def single_touch(self, card):
        if not self.active:
            return {'error': 'routine_not_active'}
        # 모드 순환: 1 → 2 → 1 …
        self.mode = 1 if self.mode != 1 else 2
        print(f"🔄 싱글 터치: 모드 {self.mode}")
        data = query_study_time(card, period='today' if self.mode == 1 else 'month')
        return {'mode': self.mode, 'data': data}

    def seat_change(self, card, seated_now):
        if not self.active:
            return {'error': 'routine_not_active'}
        # 착석 감지
        if seated_now and not self.seated:
            self.start_time = datetime.now()
            self.seated = True
            print("💺 착석 감지 → LED ON")
            return {'command': 'led_on'}
        # 이탈 감지
        if not seated_now and self.seated:
            end_time = datetime.now()
            insert_record(card, 1, self.start_time, end_time)
            self.seated = False
            self.start_time = None
            print("🚶 이탈 감지 → LED OFF")
            return {'command': 'led_off'}
        return {'seated': seated_now}

# ============ DB 기록 함수 ============
def insert_record(card, status, start, end):
    conn = mysql.connector.connect(**DB_CONFIG)
    cur = conn.cursor()
    sql = """
    INSERT INTO Record_Table (Card_Num, Status, Start_Time, End_Time)
    VALUES (%s, %s, %s, %s)
    """
    cur.execute(sql, (card, status, start, end))
    conn.commit()
    cur.close()
    conn.close()
    print(f"✅ 기록 저장: {card} | {start} ~ {end}")

# ============ 공부시간 조회 함수 ============
def query_study_time(card, period='today'):
    conn = mysql.connector.connect(**DB_CONFIG)
    cur = conn.cursor()

    if period == 'today':
        sql = """
        SELECT SEC_TO_TIME(
            SUM(TIMESTAMPDIFF(SECOND, Start_Time, End_Time))
        ) FROM Record_Table
        WHERE Card_Num=%s
          AND DATE(Start_Time)=CURDATE()
          AND Status=1
        """
        cur.execute(sql, (card,))
        result = cur.fetchone()[0] or "00:00:00"
    else:  # month: 일(d)단위 포함
        sql = """
        SELECT SUM(TIMESTAMPDIFF(SECOND, Start_Time, End_Time))
        FROM Record_Table
        WHERE Card_Num=%s
          AND YEAR(Start_Time)=YEAR(CURDATE())
          AND MONTH(Start_Time)=MONTH(CURDATE())
          AND Status=1
        """
        cur.execute(sql, (card,))
        total_sec = cur.fetchone()[0] or 0
        days    = total_sec // 86400
        hours   = (total_sec % 86400) // 3600
        minutes = (total_sec % 3600)  // 60
        seconds = total_sec % 60
        if days > 0:
            result = f"{days}d {hours:02d}:{minutes:02d}:{seconds:02d}"
        else:
            result = f"{hours:02d}:{minutes:02d}:{seconds:02d}"

    cur.close()
    conn.close()
    print(f"📊 {period} 공부시간 조회: {result}")
    return result

# ============ TCP 서버 시작 ============
HOST = '0.0.0.0'
PORT = 9090
mgr = RoutineManager()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.bind((HOST, PORT))
    server.listen()
    print(f"📡 서버 대기 중: {HOST}:{PORT}")

    while True:
        conn, addr = server.accept()
        with conn:
            raw = conn.recv(2048).decode()
            if not raw:
                continue
            try:
                msg = json.loads(raw)
                evt  = msg.get("event")
                card = msg.get("card_num", "C123456")

                if evt == "rfid_scan":
                    resp = mgr.authorize_user(msg.get("uid", ""))
                elif evt == "touch_double":
                    resp = mgr.toggle_active()
                elif evt == "touch_single":
                    resp = mgr.single_touch(card)
                elif evt == "fsr":
                    resp = mgr.seat_change(card, msg.get("seated", False))
                else:
                    resp = {'error': 'unknown_event'}
            except Exception as e:
                resp = {'error': str(e)}

            conn.sendall(json.dumps(resp).encode())
            print("📤 Response:", resp)
