import socket
import json
import pymysql
from datetime import datetime
import time
import smbus2
from lcd import LCD  # 사용자 정의 lcd.py 필요

# DB 연결 정보
DB_HOST = 'localhost'
DB_USER = 'bangme'
DB_PASSWORD = 'djwls123'
DB_NAME = 'study_db'

# LCD I2C 설정
lcd = LCD(bus=smbus2.SMBus(1), addr=0x27)  # I2C 주소에 따라 조정

# TCP 서버 설정
HOST = '0.0.0.0'
PORT = 8080

# ✅ UID 인증 → users 테이블 조회
def authenticate_uid(uid):
    conn = pymysql.connect(host=DB_HOST, user=DB_USER, password=DB_PASSWORD, db=DB_NAME)
    cursor = conn.cursor()
    cursor.execute("SELECT name FROM users WHERE user_id = %s", (uid,))
    result = cursor.fetchone()
    conn.close()
    return result[0] if result else None

# ✅ access_log 테이블에 기록
def log_access(uid, name):
    conn = pymysql.connect(host=DB_HOST, user=DB_USER, password=DB_PASSWORD, db=DB_NAME)
    cursor = conn.cursor()
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS access_log (
            id INT AUTO_INCREMENT PRIMARY KEY,
            uid VARCHAR(20),
            name VARCHAR(50),
            timestamp DATETIME
        )
    """)
    cursor.execute("INSERT INTO access_log (uid, name, timestamp) VALUES (%s, %s, %s)",
                   (uid, name, datetime.now()))
    conn.commit()
    conn.close()

def handle_connection(conn, addr):
    print(f"🔌 연결됨: {addr}")
    data = conn.recv(1024).decode().strip()
    print(f"📨 수신: {data}")

    try:
        header, body = data.split(":", 1)
        payload = json.loads(body)
        uid = payload.get("uid")
        timestamp = payload.get("timestamp")

        name = authenticate_uid(uid)

        if name:
            print(f"✅ 인증됨: {name} ({uid})")
            lcd.clear()
            lcd.message("접근 허용\n" + name)
            log_access(uid, name)
        else:
            print(f"❌ 미등록 UID: {uid}")
            lcd.clear()
            lcd.message("접근 거부\n등록되지 않음")

    except Exception as e:
        print(f"⚠️ 오류 발생: {e}")
    finally:
        conn.close()

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((HOST, PORT))
        server.listen(5)
        print(f"🚀 UID 인증 서버 실행 중: {HOST}:{PORT}")
        while True:
            conn, addr = server.accept()
            handle_connection(conn, addr)

if __name__ == "__main__":
    main()
