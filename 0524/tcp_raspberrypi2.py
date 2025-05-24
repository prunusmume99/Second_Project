#!/usr/bin/env python3
import socket
import selectors
import zmq
import datetime

# ─── 서버 설정 ───────────────────────────────────────────────
HOST = '0.0.0.0'
PORT = 8080

# ─── 변경: Actuator ESP 서버 정보 추가 ───────────────────────
# 2번째 코드에는 없던, actuator로 메시지를 포워딩할 대상
ACTUATOR_IP = '192.168.0.87'
ACTUATOR_PORT = 9090

# ─── ZeroMQ IPC 엔드포인트 (기존과 동일) ────────────────────
ZMQ_IPC = 'ipc:///tmp/esp_data'

# ─── 클라이언트 상태 관리 ──────────────────────────────────
# 2번째 코드: clients 각 항목에 'id' 필드가 있었으나,
# 여긴 'buffer'만 남김 (id 기반 브로드캐스트 로직 제거)
clients = {}  # { sock: { 'addr': (ip,port), 'buffer': bytes } }

# ─── ZeroMQ PUB 설정 (기존과 동일) ─────────────────────────
context = zmq.Context()
zmq_socket = context.socket(zmq.PUB)
zmq_socket.bind(ZMQ_IPC)

# ─── selectors 설정 (기존과 동일) ─────────────────────────
sel = selectors.DefaultSelector()


def setup_server():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.setblocking(False)
    srv.bind((HOST, PORT))
    srv.listen(10)
    sel.register(srv, selectors.EVENT_READ, accept_connection)
    print(f"{datetime.datetime.now()}: Server started on {HOST}:{PORT}")
    return srv

def accept_connection(srv_sock):
    client_sock, addr = srv_sock.accept()
    client_sock.setblocking(False)
    # 2번째 코드: {'id':'Unknown', 'buffer':b''}
    # 수정된 코드: 'id' 필드 제거
    clients[client_sock] = {'addr': addr, 'buffer': b''}
    sel.register(client_sock, selectors.EVENT_READ, handle_client)
    print(f"{datetime.datetime.now()}: New connection from {addr}")

# ─── 변경: Actuator로 메시지를 포워딩하는 함수 추가 ───────────
def forward_to_actuator(msg: str):
    """Actuator ESP TCP 서버로 단일 메시지 포워딩"""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((ACTUATOR_IP, ACTUATOR_PORT))
            s.sendall((msg + "\n").encode())
    except Exception as e:
        print(f"{datetime.datetime.now()}: [Error] Forward to Actuator failed: {e}")


def handle_client(client_sock):
    addr = clients[client_sock]['addr']
    try:
        data = client_sock.recv(1024)
        if not data:
            close_client(client_sock)
            return

        clients[client_sock]['buffer'] += data

        # 완성된 메시지(\n 단위)를 처리
        while b'\n' in clients[client_sock]['buffer']:
            raw, _, rest = clients[client_sock]['buffer'].partition(b'\n')
            clients[client_sock]['buffer'] = rest
            message = raw.decode().strip()
            if not message:
                continue

            # 1) 로깅 (기존과 동일)
            print(f"{datetime.datetime.now()}: Received from {addr}: {message}")

            # 2) ZMQ 발행 (기존과 동일)
            zmq_socket.send_string(f"{addr}: {message}")

            # 3) 원본 센서 클라이언트에 에코 응답 (기존과 동일)
            try:
                client_sock.send((message + "\n").encode())
            except Exception:
                pass

            # ─── 변경: 모든 클라이언트에 브로드캐스트 코드를 제거하고,
            #       오직 Actuator ESP에만 메시지를 포워딩하도록 대체 ───
            forward_to_actuator(message)


    except Exception as e:
        print(f"{datetime.datetime.now()}: Error with {addr}: {e}")
        close_client(client_sock)

def close_client(client_sock):
    addr = clients[client_sock]['addr']
    print(f"{datetime.datetime.now()}: Connection closed for {addr}")
    sel.unregister(client_sock)
    client_sock.close()
    del clients[client_sock]

def main():
    setup_server()
    try:
        while True:
            # 2번째 코드: zmq.poll + selectors 병행
            # 수정된 코드: selectors만 사용 (poller 제거)
            events = sel.select(timeout=0.01)
            for key, _ in events:
                callback = key.data
                callback(key.fileobj)
    except KeyboardInterrupt:
        print(f"{datetime.datetime.now()}: Server stopped by user")
    finally:
        for sock in list(clients):
            close_client(sock)
        zmq_socket.close()
        context.term()

if __name__ == "__main__":
    main()
