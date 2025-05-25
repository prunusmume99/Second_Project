# sudo apt install python3-venv python3-full
# python3 -m venv myenv
# source myenv/bin/activate
# pip install pyzmq
# pip install pymysql

import socket
import selectors
import zmq
import datetime
import json
import pymysql  # 또는 원하는 DB

# 서버 설정>
HOST = '0.0.0.0'  # 모든 인터페이스에서 수신
PORTS = {
    "sensor_input": 5010,     # 센서 ESP → Pi (TCP 서버로서 센서 ESP8266 클라이언트가 접속해오는 포트)
    "actuator_a": 8080,       # Pi → actuator ESP A
    "actuator_b": 9090        # Pi → actuator ESP B
}

# ─── 변경: Actuator ESP 서버 정보 추가 ───────────────────────
# 2번째 코드에는 없던, actuator로 메시지를 포워딩할 대상
# ACTUATOR_IP = '192.168.0.87'
ACTUATOR_PORT = 9090 # 기본 포트로 유지, 그러나 유동 포트로 확장 가능

ZMQ_IPC = 'ipc:///tmp/esp_data'  # ZeroMQ IPC 엔드포인트

# 클라이언트 상태 관리
clients = {}  # {sock: {'addr': addr, 'id': client_id, 'buffer': data}}

# ZeroMQ 설정
context = zmq.Context()
zmq_socket = context.socket(zmq.PUB)
zmq_socket.bind(ZMQ_IPC)

# selectors 및 폴링 설정
sel = selectors.DefaultSelector()
poller = zmq.Poller()
poller.register(zmq_socket, zmq.POLLIN)

def setup_server():
    for role, port in PORTS.items():
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server_socket.setblocking(False)
        server_socket.bind((HOST, port))
        server_socket.listen(10)

        # 역할에 따라 accept 핸들러 내부에서 분기하게 만들기 위해 data로 포트 전달
        sel.register(server_socket, selectors.EVENT_READ, lambda sock, p=port: accept_connection(sock, p))
        print(f"{datetime.datetime.now()}: Server started for {role} on port {port}")


def accept_connection(server_socket, port):
    client_socket, addr = server_socket.accept()
    client_socket.setblocking(False)

    clients[client_socket] = {
        'addr': addr,
        'id': 'Unknown',
        'buffer': b''
    }

    if port == PORTS["sensor_input"]:
        sel.register(client_socket, selectors.EVENT_READ, handle_sensor_input)
        print(f"{datetime.datetime.now()}: Sensor client connected from {addr}")
    else:
        sel.register(client_socket, selectors.EVENT_READ, handle_actuator_proxy)
        print(f"{datetime.datetime.now()}: Actuator client connected from {addr}")


def check_auth(did, uid):
    # 데이터베이스에서 did, uid 일치 여부 확인
    try:
        conn = pymysql.connect(
            host = "localhost",
            user = "root",
            password = "djwls123",
            database = "study_db"
        )
        cur = conn.cursor()
        cur.execute(
            "SELECT COUNT(*) FROM User_Table WHERE Desk_Num=%s AND Card_Num=%s",
            (did, uid)
        )
        count = cur.fetchone()[0]
        conn.close()
        return count > 0
    except Exception as e:
        print(f"DB Error: {e}")
        return False

def forward_to_actuator(parsed: dict):
    actu_ip = parsed.get("actuIP")
    port = ACTUATOR_PORT  # 필요 시 parsed.get("actuPort")로 확장 가능

    if not actu_ip:
        print("⚠️ No actuIP in message")
        return

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((actu_ip, port))
            s.sendall((json.dumps(parsed) + "\n").encode())
            print(f"✅ Forwarded to actuator at {actu_ip}:{port}")
    except Exception as e:
        print(f"❌ Failed to send to actuator {actu_ip}:{port} → {e}")

# def forward_to_actuator(msg: str):
#     try:
#         with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#             s.connect((ACTUATOR_IP, ACTUATOR_PORT))
#             s.sendall((msg + "\n").encode())
#     except Exception as e:
#         print(f"{datetime.datetime.now()}: [Error] Forward to Actuator failed: {e}")

def handle_actuator_proxy(client_sock):
    addr = clients[client_sock]['addr']
    try:
        tcp_payload = client_sock.recv(1024)
        if not tcp_payload:
            close_actuator_connection(client_sock)
            return

        clients[client_sock]['buffer'] += tcp_payload

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
        close_actuator_connection(client_sock)

def handle_sensor_input(client_socket):
    addr = clients[client_socket]['addr']
    try:
        recv_data = client_socket.recv(1024)
        if recv_data:
            clients[client_socket]['buffer'] += recv_data
            while b'\n' in clients[client_socket]['buffer']:
                line, _, clients[client_socket]['buffer'] = clients[client_socket]['buffer'].partition(b'\n')
                try:
                    parsed = json.loads(line.decode().strip())
                    event = parsed.get("event")
                    did = parsed.get("did")
                    uid = parsed.get("uid")
                    value = parsed.get("value")
                    timestamp = parsed.get("timestamp")
                    actuIP = parsed.get("actuIP", "")

                    # 최초 id 저장
                    if clients[client_socket].get('id', 'Unknown') == 'Unknown' and did:
                        clients[client_socket]['id'] = did
                        print(f"🎯 Client {addr} identified as {did}")

                    print(f"{datetime.datetime.now()}: Received {event} from {did} - UID: {uid}, JSON: {line.decode()}")

                    if event == "ping":
                        print(f"📡 Received ping from {did}")
                        pass

                    elif event == "rfid" and did and uid:
                        is_auth = check_auth(did, uid)
                        resp = {
                            "event": "rfid",
                            "did": did,
                            "uid": uid,
                            "value": 1 if is_auth else 0,
                            "timestamp": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                            "actuIP": actuIP
                        }
                        client_socket.send((json.dumps(resp) + "\n").encode())
                        print(f" → Sent to {did}: {'Auth' if is_auth else 'Block'}")

                    elif event in ("touch", "action"):
                        forward_to_actuator(parsed)

                    # 모든 이벤트 ZMQ 전송
                    zmq_socket.send_string(f"{did}: {json.dumps(parsed)}")

                except json.JSONDecodeError as e:
                    print(f"❌ JSON Parse Error: {e}, Raw: {line}")
        else:
            close_sensor_connection(client_socket)
    except Exception as e:
        print(f"{datetime.datetime.now()}: Error with {clients[client_socket]['id']} ({addr}): {e}")
        close_sensor_connection(client_socket)


def close_sensor_connection(client_socket):
    addr = clients[client_socket]['addr']
    client_id = clients[client_socket]['id']
    print(f"{datetime.datetime.now()}: Connection closed for {client_id} ({addr})")
    sel.unregister(client_socket)
    client_socket.close()
    del clients[client_socket]

def close_actuator_connection(client_sock):
    addr = clients[client_sock]['addr']
    print(f"{datetime.datetime.now()}: Connection closed for {addr}")
    sel.unregister(client_sock)
    client_sock.close()
    del clients[client_sock]

def main():
    server_socket = setup_server()
    
    while True:
        # zmq.poll과 selectors를 함께 사용
        zmq_events = dict(poller.poll(10))  # 10ms 타임아웃
        selector_events = sel.select(timeout=0.01)  # 10ms 타임아웃
        
        # ZeroMQ 이벤트 처리 (현재는 PUB 소켓이므로 입력 없음, 예비용)
        if zmq_socket in zmq_events and zmq_events[zmq_socket] & zmq.POLLIN:
            message = zmq_socket.recv_string()
            print(f"{datetime.datetime.now()}: ZeroMQ received: {message}")
        
        # TCP 소켓 이벤트 처리
        for key, mask in selector_events:
            callback = key.data
            callback(key.fileobj)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(f"{datetime.datetime.now()}: Server stopped")
        for client_socket in clients:
            sel.unregister(client_socket)
            client_socket.close()
        zmq_socket.close()
        context.term()