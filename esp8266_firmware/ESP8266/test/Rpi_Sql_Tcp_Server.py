# ✅ Raspberry Pi TCP 중계 서버 (SQL + LCD/LED 명령 전송)

import socket
import json
import pymysql

# DB 연결
conn = pymysql.connect(
    host='localhost', user='bangme', password='djwls123', database='study_db', charset='utf8'
)
cursor = conn.cursor()

# LCD/LED 제어 ESP8266 주소
DISPLAY_NODE_IP = '192.168.0.61'
DISPLAY_NODE_PORT = 9091

def send_to_display_node(message):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((DISPLAY_NODE_IP, DISPLAY_NODE_PORT))
        s.sendall((message + '\n').encode())
        s.close()
        print("📤 LCD/LED에 전송:", message)
    except Exception as e:
        print("❌ 전송 실패:", e)

def handle_data(data):
    try:
        json_data = json.loads(data)
        event = json_data.get("event")
        uid = json_data.get("uid")

        if event == "rfid_scan":
            print(f"🪪 UID 인증: {uid}")
            send_to_display_node("lcd:RFID 인증됨")

        elif event == "mode":
            mode = int(json_data.get("mode"))
            if mode == 1:
                # 당일 학습 시간 조회
                cursor.execute("""
                    SELECT SEC_TO_TIME(SUM(TIME_TO_SEC(duration)))
                    FROM study_record
                    WHERE user_id = (SELECT user_id FROM users WHERE uid = %s)
                    AND DATE(start_time) = CURDATE()
                """, (uid,))
                result = cursor.fetchone()[0] or '00:00:00'
                send_to_display_node(f"lcd:Today {result}")
            elif mode == 2:
                # 월간 학습 시간 조회
                cursor.execute("""
                    SELECT SEC_TO_TIME(SUM(TIME_TO_SEC(duration)))
                    FROM study_record
                    WHERE user_id = (SELECT user_id FROM users WHERE uid = %s)
                    AND start_time >= DATE_FORMAT(NOW(), '%Y-%m-01')
                """, (uid,))
                result = cursor.fetchone()[0] or '00:00:00'
                send_to_display_node(f"lcd:Month {result}")

        elif event == "fsr":
            fsr = int(json_data.get("value"))
            if fsr > 300:
                send_to_display_node("led_on")
            else:
                send_to_display_node("led_off")
    except Exception as e:
        print("⚠️ 처리 실패:", e)

# 센서 노드로부터 수신
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(("0.0.0.0", 9090))
server.listen(1)
print("📡 Raspberry Pi 서버 대기 중 (port 9090)")

try:
    while True:
        client, addr = server.accept()
        data = client.recv(1024).decode()
        print("📥 수신:", data.strip())
        handle_data(data.strip())
        client.close()
except KeyboardInterrupt:
    print("서버 종료")
    cursor.close()
    conn.close()
    server.close()
