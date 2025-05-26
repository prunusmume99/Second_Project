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
    "sensor_input": 5010,   # sensor ESP → Pi (TCP 서버로서 센서 ESP8266 클라이언트가 접속해오는 포트)
    "actuator_output": 7010 # Pi → actuator ESP
}

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
    server_socket.bind((HOST, PORTS['sensor_input']))
    server_socket.listen(10)
    sel.register(server_socket, selectors.EVENT_READ, accept_connection)
    print(f"{datetime.datetime.now()}: Server started on {HOST}:{PORTS['sensor_input']}")
    return server_socket


def accept_connection(server_socket):
    client_socket, addr = server_socket.accept()
    client_socket.setblocking(False)
    clients[client_socket] = {'addr': addr, 'id': 'Unknown', 'buffer': b''}
    sel.register(client_socket, selectors.EVENT_READ, handle_sensor_input)
    print(f"{datetime.datetime.now()}: New connection from {addr}")


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


def forward_to_actuator(parsed: dict, actuIP: str):
    actu_ip = actuIP
    port = PORTS['actuator_output']

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


def handle_sensor_input(client_socket):
    addr = clients[client_socket]['addr']
    try:
        recv_data = client_socket.recv(1024)
        if recv_data:
            clients[client_socket]['buffer'] += recv_data
            # 줄바꿈(\n)으로 메시지 완성 여부 확인
            while b'\n' in clients[client_socket]['buffer']:
                line, _, clients[client_socket]['buffer'] = clients[client_socket]['buffer'].partition(b'\n')
                try:
                    parsed = json.loads(line.decode().strip())
                    event = parsed.get("event")
                    actuIP = parsed.get("actuIP")
                    did = parsed.get("did")
                    uid = parsed.get("uid")
                    value = parsed.get("value")
                    timestamp = parsed.get("timestamp")

                    # 최초 id 저장
                    if clients[client_socket].get('id', 'Unknown') == 'Unknown' and did:
                        clients[client_socket]['id'] = did
                        print(f"🎯 Client {addr} identified as {did}")

                    if event == "ping":
                        print(".", end="")
                    else:
                        print(f"{datetime.datetime.now()}: Received {event} from {did} - UID: {uid}, JSON: {line.decode()}")

                    if event == "rfid" and did and uid:
                        is_auth = check_auth(did, uid)
                        resp = {
                            "event": event,
                            "actuIP": actuIP,
                            "did": did,
                            "uid": uid,
                            "value": 1 if is_auth else 0,
                            "timestamp": timestamp
                        }
                        client_socket.send((json.dumps(resp) + "\n").encode())
                        print(f" → Sent to {did}: {json.dumps(resp)}")

                        command = create_actuator_command(event, 1 if is_auth else 0, "Auth-Result", timestamp)
                        forward_to_actuator(command, actuIP)

                    elif event == "touch" and uid:
                        command_value = ""
                        if value == 10:
                            command_value = "Default-Mode"
                        elif value == 11:
                            command_value = get_statistics(value, uid)
                        elif value == 12:
                            command_value = get_statistics(value, uid)
                        elif value == 2:
                            command_value = "Double-Touch"
                        elif value == 3:
                            command_value = "Long-Touch"
                            execute_finish(uid, timestamp)

                        command = create_actuator_command(event, value, command_value, timestamp)
                        forward_to_actuator(command, actuIP)

                    elif event == "action" and uid:
                        execute_recording(value, uid, timestamp)
                        command = create_actuator_command(event, value, "Rec.Study-Time" if value else "Rec.Break-Time", timestamp)
                        forward_to_actuator(command, actuIP)

                    else:
                        client_socket.send(line)
                        print("→", end="")


                    # 모든 이벤트 ZMQ 전송
                    zmq_socket.send_string(f"{did}: {json.dumps(parsed)}")

                except json.JSONDecodeError as e:
                    print(f"❌ JSON Parse Error: {e}, Raw: {line}")
        else:
            # 클라이언트 연결 종료
            close_sensor_connection(client_socket)
    except Exception as e:
        print(f"{datetime.datetime.now()}: Error with {clients[client_socket]['id']} ({addr}): {e}")
        close_sensor_connection(client_socket)


def create_actuator_command(event: str, code: int, value: str, timestamp: str) -> dict:
    command = {
        "event": event,
        "code": code,
        "value": value,
        "timestamp": timestamp
    }
    return command


def check_auth(did: str, uid: str):
    # 데이터베이스에서 did, uid 일치 여부 확인
    try:
        conn = pymysql.connect(
            host = "localhost",
            user = "root",
            password = "djwls123",
            database = "study_db"
        )
        with conn.cursor() as cur:
            cur.execute(
                "SELECT COUNT(*) FROM User_Table WHERE Desk_Num=%s AND Card_Num=%s",
                (did, uid)
            )
            count = cur.fetchone()[0] or 0
        return count > 0
    except Exception as e:
        print(f"check_auth Error: {e}")
        return False
    finally:
        if 'conn' in locals():
            conn.close()


def execute_recording(code: int, uid: str, timestamp: str):
    try:
        conn = pymysql.connect(
            host="localhost",
            user="root",
            password="djwls123",
            database="study_db"
        )
        with conn.cursor() as cur:
            conn.begin()    # 트랜잭션 시작 (자동 커밋 끄기)
            cur.execute("""
                UPDATE Record_Table SET End_Time = %s, Amount = TIMESTAMPDIFF(MINUTE, Start_Time, %s)
                WHERE Card_Num = %s AND End_Time IS NULL
                """, (timestamp, timestamp, uid))
            cur.execute("""
                INSERT INTO Record_Table (Card_Num, Status, Start_Time)
                VALUES (%s, %d, %s);
                """, (uid, code, timestamp, ))
        conn.commit()       # 완료 커밋
        return True
    except Exception as e:
        print(f"execute_recording Error: {e}")
        if 'conn' in locals():
            conn.rollback()
        return False
    finally:
        if 'conn' in locals():
            conn.close()


def execute_finish(uid: str, timestamp: str):
    try:
        conn = pymysql.connect(
            host="localhost",
            user="root",
            password="djwls123",
            database="study_db"
        )
        with conn.cursor() as cur:
            conn.begin()    # 트랜잭션 시작 (자동 커밋 끄기)
            cur.execute("""
                UPDATE Record_Table SET End_Time = %s, Amount = TIMESTAMPDIFF(MINUTE, Start_Time, %s)
                WHERE Card_Num = %s AND End_Time IS NULL
                """, (timestamp, timestamp, uid))
        conn.commit()       # 완료 커밋
        return True
    except Exception as e:
        print(f"execute_finish Error: {e}")
        if 'conn' in locals():
            conn.rollback()
        return False
    finally:
        if 'conn' in locals():
            conn.close()


def get_statistics(code: int, uid: str) -> str:
    try:
        conn = pymysql.connect(
            host="localhost",
            user="root",
            password="djwls123",
            database="study_db"
        )
        with conn.cursor() as cur:
            if code == 11:      # 하루동안 총 기록 시간 구하기
                cur.execute("""
                SELECT
                    SUM(CASE WHEN Status = 1 THEN Amount ELSE 0 END) AS total_study_minutes,
                    SUM(CASE WHEN Status = 0 THEN Amount ELSE 0 END) AS total_break_minutes
                FROM Record_Table
                WHERE Start_Time >= CURDATE() AND Card_Num = %s
                """, (uid,))
                row = cur.fetchone()
                study_time = row[0] or 0
                break_time = row[1] or 0
            elif code == 12:    # 한 달 동안의 평균 패턴 구하기
                cur.execute("""
                SELECT
                    FLOOR(AVG(CASE WHEN Status = 1 THEN Amount END)) AS avg_study_minutes,
                    FLOOR(AVG(CASE WHEN Status = 0 THEN Amount END)) AS avg_break_minutes
                FROM Record_Table
                WHERE Start_Time >= (CURDATE() - INTERVAL 1 MONTH) AND Start_Time < CURDATE()
                AND Card_Num = %s
                """, (uid,))
                row = cur.fetchone()
                study_time = row[0] or 0
                break_time = row[1] or 0
        return f"Std:{study_time},Brk:{break_time}"
    except Exception as e:
        print(f"get_statistics Error: {e}")
        return f"get_statistics({code}) Error: {e}"
    finally:
        if 'conn' in locals():
            conn.close()


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