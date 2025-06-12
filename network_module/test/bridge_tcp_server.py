# bridge_tcp_server.py
import socket
import zmq
import json

# 서버 설정
HOST = '0.0.0.0'
PORT = 5001

# ZMQ PUB 소켓 설정 (XSUB 포트로 연결)
context = zmq.Context()
pub_socket = context.socket(zmq.PUB)
pub_socket.connect("tcp://localhost:6000")

# TCP 소켓 서버 설정
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print(f"[Bridge TCP] 연결 대기 중: {HOST}:{PORT}")

while True:
    conn, addr = server.accept()
    print(f"[Bridge TCP] 클라이언트 연결됨: {addr}")

    data = conn.recv(1024).decode('utf-8').strip()
    print(f"[Bridge TCP] 받은 데이터: {data}")

    try:
        json_data = json.loads(data)
        pub_socket.send_json(json_data)
        print("[Bridge TCP] ✅ ZMQ로 전송 완료!")
    except json.JSONDecodeError as e:
        print(f"⚠️ JSON 파싱 오류: {e}")

    conn.close()