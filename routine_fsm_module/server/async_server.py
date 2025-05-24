import socket, selectors, zmq, json
from datetime import datetime

# MySQL 연결 설정 (필요 시)
DB_CONFIG = {...}

HOST, PORT = '0.0.0.0', 8080
sel = selectors.DefaultSelector()

# ZeroMQ PUB 소켓
context = zmq.Context()
zmq_pub = context.socket(zmq.PUB)
zmq_pub.bind('ipc:///tmp/esp_data')

class Connection:
    def __init__(self, sock):
        self.sock   = sock
        self.addr   = sock.getpeername()
        self.buffer = b''
    def fileno(self): return self.sock.fileno()
    def read(self):
        data = self.sock.recv(1024)
        if data:
            self.buffer += data
            while b'\n' in self.buffer:
                line, self.buffer = self.buffer.split(b'\n',1)
                handle_message(line.decode().strip(), self)
        else:
            sel.unregister(self.sock); self.sock.close()
    def write(self, msg):
        self.sock.sendall((msg+'\n').encode())

def accept(server_sock):
    conn, addr = server_sock.accept()
    conn.setblocking(False)
    clients[conn] = Connection(conn)
    sel.register(conn, selectors.EVENT_READ, clients[conn].read)
    print(f"{datetime.now()}: Conn from {addr}")

def handle_message(raw, conn_obj):
    msg = json.loads(raw)
    cid = msg.get('client_id','Unknown')
    evt = msg.get('event')
    # 내부 브리지로 전송
    zmq_pub.send_string(f"{cid}:{raw}")
    # ACK
    conn_obj.write(f"ACK:{raw}")
    print(f"{datetime.now()}: {cid} -> {evt}")

# 서버 소켓 설정
server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_sock.bind((HOST, PORT))
server_sock.listen()
server_sock.setblocking(False)
sel.register(server_sock, selectors.EVENT_READ, accept)
clients = {}

print(f"Server listening on {HOST}:{PORT}")
while True:
    for key, _ in sel.select():
        callback = key.data
        callback(key.fileobj)
