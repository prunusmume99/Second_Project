import socket
import json
import time

HOST = '0.0.0.0'  # main_server.py 의 HOST
PORT = 9090            # main_server.py 의 PORT

def send_event(event: dict):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        payload = json.dumps(event)
        s.sendall(payload.encode('utf-8'))
        print("📤 Sent:", payload)


        data = s.recv(1024).decode('utf-8')
        print("📥 Received:", data)

if __name__ == "__main__":
    # 1) RFID 테스트
    send_event({"event": "rfid_scan", "uid": "C123456"})
    time.sleep(1)

    # 2) 루틴 시작
    send_event({"event": "touch_toggle", "card_num": "C123456"})
    time.sleep(1)

    # 3) 착석 감지
    send_event({"event": "fsr", "card_num": "C123456", "seated": True})
    time.sleep(2)

    # 4) 이탈 감지
    send_event({"event": "fsr", "card_num": "C123456", "seated": False})
    time.sleep(1)

    # 5) 루틴 종료
    send_event({"event": "touch_toggle", "card_num": "C123456"})
