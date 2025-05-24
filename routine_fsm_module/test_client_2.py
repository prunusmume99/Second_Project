import socket
import json
import time

HOST = '0.0.0.0'
PORT = 9090

def send(event: dict):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(json.dumps(event).encode())
        print("📤 Sent:", event)
        resp = s.recv(2048).decode()
        print("📥 Received:", resp)

if __name__ == "__main__":
    # 1) RFID 인증
    send({"event": "rfid_scan", "uid": "C123456"})
    time.sleep(1)

    # 2) 더블 터치 → 루틴 ON
    send({"event": "touch_double", "card_num": "C123456"})
    time.sleep(1)

    # 3) FSR 착석 → LED ON
    send({"event": "fsr", "card_num": "C123456", "seated": True})
    time.sleep(1)

    # 4) 싱글 터치 → 모드1 (당일 공부시간)
    send({"event": "touch_single", "card_num": "C123456"})
    time.sleep(1)

    # 5) 싱글 터치 → 모드2 (월별 공부시간)
    send({"event": "touch_single", "card_num": "C123456"})
    time.sleep(1)

    # 6) FSR 이탈 → LED OFF + DB 기록
    send({"event": "fsr", "card_num": "C123456", "seated": False})
    time.sleep(1)

    # 7) 더블 터치 → 루틴 OFF
    send({"event": "touch_double", "card_num": "C123456"})
