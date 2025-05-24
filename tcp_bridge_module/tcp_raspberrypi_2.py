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
PORT = 5010       # ESP8266 클라이언트가 연결할 포트
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
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.setblocking(False)
    server_socket.bind((HOST, PORT))
    server_socket.listen(10)
    sel.register(server_socket, selectors.EVENT_READ, accept_connection)
    print(f"{datetime.datetime.now()}: Server started on {HOST}:{PORT}")
    return server_socket

def accept_connection(server_socket):
    client_socket, addr = server_socket.accept()
    client_socket.setblocking(False)
    clients[client_socket] = {'addr': addr, 'id': 'Unknown', 'buffer': b''}
    sel.register(client_socket, selectors.EVENT_READ, handle_client)
    print(f"{datetime.datetime.now()}: New connection from {addr}")

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

def handle_client(client_socket):
    addr = clients[client_socket]['addr']
    try:
        data = client_socket.recv(1024)
        if data:
            clients[client_socket]['buffer'] += data
            # 줄바꿈(\n)으로 메시지 완성 여부 확인
            while b'\n' in clients[client_socket]['buffer']:
                line, _, clients[client_socket]['buffer'] = clients[client_socket]['buffer'].partition(b'\n')
                try:
                    parsed = json.loads(line.decode().strip())
                    event = parsed.get("event")
                    did = parsed.get("did")
                    uid = parsed.get("uid")
                    value = parsed.get("value")
                    timestamp = parsed.get("timestamp")

                    # 최초 id 저장
                    if clients[client_socket].get('id', 'Unknown') == 'Unknown' and did:
                        clients[client_socket]['id'] = did
                        print(f"🎯 Client {addr} identified as {did}")

                    print(f"{datetime.datetime.now()}: Received {event} from {did} - UID: {uid}, JSON: {line.decode()}")

                    if event == "ping":
                        client_socket.send(("ping\n").encode())
                        print(f" → Sent to {did}: ping")

                    elif event == "rfid" and did and uid:
                        is_auth = check_auth(did, uid)
                        response_value = 1 if is_auth else 0

                        resp = {
                            "event": event,
                            "did": did,
                            "uid": uid,
                            "value": response_value,
                            "timestamp": timestamp
                        }
                        client_socket.send((json.dumps(resp) + "\n").encode())
                        print(f" → Sent to {did}: {json.dumps(resp)}")

                    else:
                        client_socket.send((line + "\n").encode())
                        print(f" → Sent to {did}: {line}")

                    # 모든 이벤트에 대해 ZMQ로 중계
                    zmq_socket.send_string(f"{did}: {json.dumps(parsed)}")

                except json.JSONDecodeError as e:
                    print(f"❌ JSON Parse Error: {e}, Raw: {line}")
        else:
            # 클라이언트 연결 종료
            close_client(client_socket)
    except Exception as e:
        print(f"{datetime.datetime.now()}: Error with {clients[client_socket]['id']} ({addr}): {e}")
        close_client(client_socket)

def close_client(client_socket):
    addr = clients[client_socket]['addr']
    client_id = clients[client_socket]['id']
    print(f"{datetime.datetime.now()}: Connection closed for {client_id} ({addr})")
    sel.unregister(client_socket)
    client_socket.close()
    del clients[client_socket]

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