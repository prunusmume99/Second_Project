# bridge_sub_to_tcp.py
import zmq
import socket
import json
import time

def create_sub_socket(connect_to: str, topic_filter: bytes = b""):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(connect_to)
    socket.setsockopt(zmq.SUBSCRIBE, topic_filter)
    return socket

ESP8266_IP = '192.168.0.103'  # 실제 ESP8266 장치의 IP 주소
ESP8266_PORT = 6002          # ESP8266이 수신 대기 중인 포트

def send_to_esp8266(json_data):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((ESP8266_IP, ESP8266_PORT))
            s.sendall(json.dumps(json_data).encode('utf-8'))
            print("✅ ESP8266으로 메시지 전송 완료")
    except Exception as e:
        print(f"❌ ESP8266 전송 실패: {e}")

def main():
    sub_socket = create_sub_socket("tcp://localhost:6001")

    print("[Bridge SUB→TCP] 메시지 수신 대기 중...")
    while True:
        try:
            message = sub_socket.recv_json()
            print(f"[Bridge SUB→TCP] 받은 메시지: {message}")
            send_to_esp8266(message)
            time.sleep(0.5)
        except Exception as e:
            print(f"⚠️ 에러 발생: {e}")
            time.sleep(1)

if __name__ == "__main__":
    main()