# basic_tcp_server.py
import socket

HOST = '0.0.0.0'
PORT = 5001

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((HOST, PORT))
server.listen(1)

print(f"[서버] 포트 {PORT}에서 대기 중...")

while True:
    conn, addr = server.accept()
    print(f"[서버] 클라이언트 연결됨: {addr}")

    data = conn.recv(1024).decode()
    print(f"[서버] 받은 데이터: {data}")

    conn.close()
