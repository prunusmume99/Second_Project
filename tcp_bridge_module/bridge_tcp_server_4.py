import socket
import zmq
import json

HOST = '0.0.0.0'
PORT = 5001

context = zmq.Context()

# ESP → FSM
pub_socket = context.socket(zmq.PUB)
pub_socket.connect("tcp://localhost:6000")

# FSM → ESP
sub_socket = context.socket(zmq.SUB)
sub_socket.connect("tcp://localhost:6001")
sub_socket.setsockopt(zmq.SUBSCRIBE, b"")

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((HOST, PORT))
server.listen(1)

print(f"[Bridge TCP] 연결 대기 중: {HOST}:{PORT}")

while True:
    conn, addr = server.accept()
    print(f"[Bridge TCP] 클라이언트 연결됨: {addr}")

    data = conn.recv(1024).decode().strip()
    print(f"[Bridge TCP] 받은 데이터: {data}")

    try:
        json_data = json.loads(data)
        pub_socket.send_json(json_data)
        print("[Bridge TCP] ✅ ZMQ로 전송 완료!")
    except json.JSONDecodeError as e:
        print(f"⚠️ JSON 파싱 오류: {e}")
        conn.close()
        continue

    # ✅ 응답 수신 대기 (auth_result)
    try:
        poller = zmq.Poller()
        poller.register(sub_socket, zmq.POLLIN)
        socks = dict(poller.poll(2000))  # 2초 대기

        if sub_socket in socks:
            response = sub_socket.recv_json()
            print(f"[Bridge TCP] 🔁 응답 수신: {response}")
            conn.sendall((json.dumps(response) + "\n").encode())
        else:
            print("⚠️ FSM 응답 없음")
    except Exception as e:
        print(f"❌ 응답 전송 실패: {e}")

    conn.close()
