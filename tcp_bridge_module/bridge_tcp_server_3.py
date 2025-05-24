# bridge_tcp_server.py
import socket
import zmq
import threading
import json

# === TCP 설정 ===
HOST = '0.0.0.0'
PORT = 5001

# === ZMQ 설정 ===
context = zmq.Context()

# PUB 소켓: ESP → FSM
pub_socket = context.socket(zmq.PUB)
pub_socket.connect("tcp://localhost:6000")  # XSUB로 연결

# SUB 소켓: FSM → ESP
sub_socket = context.socket(zmq.SUB)
sub_socket.connect("tcp://localhost:6001")  # XPUB로 연결
sub_socket.setsockopt(zmq.SUBSCRIBE, b"")

# === 클라이언트 커넥션 저장용 변수 ===
client_conn = None
client_addr = None
conn_lock = threading.Lock()

def handle_tcp():
    global client_conn, client_addr

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, PORT))
    server.listen(1)
    print(f"[TCP Bridge] 연결 대기 중: {HOST}:{PORT}")

    while True:
        conn, addr = server.accept()
        print(f"[TCP Bridge] 클라이언트 연결됨: {addr}")

        with conn_lock:
            client_conn = conn
            client_addr = addr

        try:
            data = conn.recv(1024).decode().strip()
            print(f"[TCP Bridge] 받은 데이터: {data}")

            json_data = json.loads(data)
            pub_socket.send_json(json_data)
            print("[TCP Bridge] ✅ ZMQ로 전송 완료!")

            # conn은 닫지 않고 유지!
        except json.JSONDecodeError as e:
            print(f"⚠️ JSON 파싱 오류: {e}")
        except Exception as e:
            print(f"❌ TCP 처리 중 예외: {e}")

def handle_zmq_sub():
    global client_conn

    print("[ZMQ → TCP Bridge] FSM 결과 수신 대기 중...")

    while True:
        message = sub_socket.recv_json()
        print(f"[ZMQ SUB] FSM 결과 수신: {message}")

        with conn_lock:
            if client_conn:
                try:
                    client_conn.sendall((json.dumps(message) + "\n").encode())
                    print("[ZMQ → TCP] ✅ 인증 결과 전송 완료!")
                except Exception as e:
                    print(f"❌ TCP 전송 실패: {e}")
                    client_conn = None
            else:
                print("⚠️ 현재 연결된 클라이언트 없음 → 결과 미전송")

def main():
    tcp_thread = threading.Thread(target=handle_tcp, daemon=True)
    zmq_thread = threading.Thread(target=handle_zmq_sub, daemon=True)

    tcp_thread.start()
    zmq_thread.start()

    tcp_thread.join()
    zmq_thread.join()

if __name__ == "__main__":
    main()
