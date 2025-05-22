import socket
import json
import pymysql
import time
import I2C_LCD_driver
from datetime import datetime

# DB 연결 설정
conn = pymysql.connect(
    host='localhost',
    user='bangme',
    password='djwls123',
    database='study_db',
    charset='utf8'
)
cursor = conn.cursor()

# LCD 드라이버 초기화
lcd = I2C_LCD_driver.lcd()

def get_today_time(user_id):
    cursor.execute(f"""
        SELECT SEC_TO_TIME(SUM(TIME_TO_SEC(duration)))
        FROM study_record
        WHERE user_id = %s AND DATE(start_time) = CURDATE()
    """, (user_id,))
    return cursor.fetchone()[0] or '00:00:00'

def get_month_time(user_id):
    cursor.execute(f"""
        SELECT SEC_TO_TIME(SUM(TIME_TO_SEC(duration)))
        FROM study_record
        WHERE user_id = %s
        AND start_time >= DATE_FORMAT(CURDATE(), '%Y-%m-01')
        AND start_time < DATE_FORMAT(DATE_ADD(CURDATE(), INTERVAL 1 MONTH), '%Y-%m-01')
    """, (user_id,))
    return cursor.fetchone()[0] or '00:00:00'

def display_mode_info(user_id, mode):
    if mode == 1:
        time_str = get_today_time(user_id)
        lcd.lcd_display_string("Today Study Time", 1)
        lcd.lcd_display_string(f"{time_str}", 2)
    elif mode == 2:
        time_str = get_month_time(user_id)
        lcd.lcd_display_string("Month Study Time", 1)
        lcd.lcd_display_string(f"{time_str}", 2)
    else:
        lcd.lcd_display_string("Unknown Mode", 1)
        lcd.lcd_display_string(" ", 2)

def handle_data(json_data):
    if 'event' in json_data and json_data['event'] == 'mode':
        uid = json_data.get('uid')
        mode = int(json_data.get('mode'))
        print(f"📥 모드 {mode} 요청 - UID: {uid}")

        # UID로부터 user_id 조회 (users 테이블 필요)
        cursor.execute("SELECT user_id FROM users WHERE uid = %s", (uid,))
        result = cursor.fetchone()
        if result:
            user_id = result[0]
            display_mode_info(user_id, mode)
        else:
            lcd.lcd_display_string("Unknown User", 1)
            lcd.lcd_display_string("Check UID", 2)

# TCP 수신 서버 설정
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('0.0.0.0', 9090))
server.listen(1)
print("📡 Listening for ESP8266...")

try:
    while True:
        client, addr = server.accept()
        print(f"📶 연결됨: {addr}")
        data = client.recv(1024).decode('utf-8')
        if data:
            print(f"📨 수신: {data.strip()}")
            try:
                payload = data.split(':', 1)[1].strip()
                json_data = json.loads(payload)
                handle_data(json_data)
            except Exception as e:
                print("⚠️ JSON 처리 오류:", e)
        client.close()
except KeyboardInterrupt:
    print("서버 종료")
finally:
    cursor.close()
    conn.close()
    server.close()
