import socket
import selectors
import zmq
import datetime
import json
import pymysql  # 또는 원하는 DB

HOST = '0.0.0.0'
PORT = 8080
ZMQ_IPC = 'ipc:///tmp/esp_data'

clients = {}

context = zmq.Context()
zmq_socket = context.socket(zmq.PUB)
zmq_socket.bind(ZMQ_IPC)

sel = selectors.DefaultSelector()

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
            host = "192.168.0.60",
            user = "root",
            password = "djwls123",
            database = "study_db.db"
        )
        cur = conn.cursor()
        cur.execute(
            "SELECT COUNT(*) FROM `user` WHERE Desk_Num=%s AND Card_Num=%s",
            (did, uid)
        )
        count = cur.fetchone()[0]
        conn.close()
        return count > 0
    except Exception as e:
        print(f"DB Error: {e}")
        return False

def handle_client(client_socket):
    try:
        data = client_socket.recv(1024)
        if data:
            clients[client_socket]['buffer'] += data

            while b'\n' in clients[client_socket]['buffer']:
                line, _, clients[client_socket]['buffer'] = clients[client_socket]['buffer'].partition(b'\n')
                try:
                    parsed = json.loads(line.decode().strip())
                    event = parsed.get("event")
                    did = parsed.get("did")
                    uid = parsed.get("uid")

                    # 최초 id 저장
                    if clients[client_socket].get('id', 'Unknown') == 'Unknown' and did:
                        clients[client_socket]['id'] = did
                        print(f"🎯 Client {clients[client_socket]['addr']} identified as {did}")

                    print(f"{datetime.datetime.now()}: Received {event} from {did} - UID: {uid}")

                    if event == "ping":
                        pass

                    elif event == "rfid" and did and uid:
                        is_auth = check_auth(did, uid)
                        response_event = "Auth" if is_auth else "Block"

                        resp = {
                            "event": response_event,
                            "did": did,
                            "uid": uid,
                            "timestamp": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        }
                        client_socket.send((json.dumps(resp) + "\n").encode())
                        print(f" → Sent to {did}: {resp['event']}")

                    # 모든 이벤트에 대해 ZMQ로 중계
                    zmq_socket.send_string(f"{did}: {json.dumps(parsed)}")

                except json.JSONDecodeError as e:
                    print(f"❌ JSON Parse Error: {e}, Raw: {line}")
        else:
            close_client(client_socket)
    except Exception as e:
        print(f"Error: {e}")
        close_client(client_socket)

def close_client(client_socket):
    addr = clients[client_socket]['addr']
    print(f"{datetime.datetime.now()}: Connection closed for {addr}")
    sel.unregister(client_socket)
    client_socket.close()
    del clients[client_socket]

def main():
    server_socket = setup_server()
    while True:
        events = sel.select(timeout=0.01)
        for key, _ in events:
            callback = key.data
            callback(key.fileobj)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("서버 종료 중...")
        for client_socket in list(clients.keys()):
            sel.unregister(client_socket)
            client_socket.close()
        zmq_socket.close()
        context.term()
