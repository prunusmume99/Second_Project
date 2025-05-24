import socket               # TCP 소켓
import selectors            # 멀티플렉싱용
import zmq                  # ZeroMQ IPC
import json                 # JSON 파싱/생성
from datetime import datetime
import mysql.connector      # MySQL 연결 라이브러리

# ============ MySQL 설정 ============
DB_CONFIG = {
    'host': 'localhost',
    'user': 'bangme',
    'password': 'djwls123',
    'database': 'study_db'
}

# ============ TCP 서버 설정 ============
HOST = '0.0.0.0'  # 모든 인터페이스 수신
PORT = 8080       # ESP 이벤트 수신 포트
sel = selectors.DefaultSelector()

# 연결된 클라이언트 저장
clients = {}

# ============ ZeroMQ 브리지 설정 ============
context = zmq.Context()
zmq_pub = context.socket(zmq.PUB)
zmq_pub.bind('ipc:///tmp/esp_data')  # 내부 IPC 엔드포인트

# 클라이언션 래퍼 클래스
class Connection:
    def __init__(self, sock):
        self.sock = sock
        self.addr = sock.getpeername()
        self.buffer = b""
    def fileno(self): return self.sock.fileno()
    def read(self):
        data = self.sock.recv(1024)
        if data:
            self.buffer += data
            # 줄바꿈 기준으로 메시지 완성
            while b'\n' in self.buffer:
                line, self.buffer = self.buffer.split(b'\n', 1)
                handle_message(line.decode().strip(), self)
        else:
            # 연결 종료 시 정리
            sel.unregister(self.sock)
            self.sock.close()
    def write(self, msg):
        self.sock.sendall((msg + '\n').encode())

# 신규 연결 수락 함수
def accept(sock):
    conn, addr = sock.accept()
    conn.setblocking(False)
    clients[conn] = Connection(conn)
    sel.register(conn, selectors.EVENT_READ, clients[conn].read)
    print(f"{datetime.now()}: Connection from {addr}")

# 수신된 메시지 처리 함수
def handle_message(raw, conn_obj):
    try:
        msg = json.loads(raw)
        event = msg.get('event')
        cid = msg.get('client_id', 'Unknown')
        # ZeroMQ로 이벤트 브로드캐스트
        zmq_pub.send_string(f"{cid}:{raw}")
        # MySQL 처리 (예: insert_record) 가능
        # acknowledgе
        conn_obj.write(f"ACK:{raw}")
        print(f"{datetime.now()}: {cid} -> {event}")
    except Exception as e:
        conn_obj.write(f"ERROR:{e}")

# 서버 초기화 및 등록
lsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
lsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
lsock.bind((HOST, PORT))
lsock.listen()
lsock.setblocking(False)
sel.register(lsock, selectors.EVENT_READ, accept)
print(f"Server listening on {HOST}:{PORT}")

# 이벤트 루프
while True:
    events = sel.select()
    for key, _ in events:
        callback = key.data
        callback(key.fileobj)