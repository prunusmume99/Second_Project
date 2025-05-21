# db_subscriber_pub.py

import zmq
import mysql.connector
import json
import time

ZMQ_SUB_ADDR = "tcp://localhost:6001"
ZMQ_PUB_ADDR = "tcp://localhost:6000"  # 다시 PUB로 송신

MYSQL_CONFIG = {
    "host": "localhost",
    "user": "bangme",
    "password": "djwls123",
    "database": "study_db"
}

def connect_db():
    return mysql.connector.connect(**MYSQL_CONFIG)

def insert_study_record(conn, data):
    cursor = conn.cursor()
    query = """
        INSERT INTO study_record (user_id, start_time, end_time, mode, action)
        VALUES (%s, %s, %s, %s, %s)
    """
    values = (
        data["user_id"],
        data["start_time"],
        data["end_time"],
        data["mode"],
        data["action"]
    )
    cursor.execute(query, values)
    conn.commit()
    cursor.close()

def main():
    context = zmq.Context()

    # SUB 설정
    sub_socket = context.socket(zmq.SUB)
    sub_socket.connect(ZMQ_SUB_ADDR)
    sub_socket.setsockopt(zmq.SUBSCRIBE, b"")

    # PUB 설정
    pub_socket = context.socket(zmq.PUB)
    pub_socket.connect(ZMQ_PUB_ADDR)

    print("[DB SUB+PUB] 수신 대기 중...")
    conn = connect_db()

    while True:
        try:
            msg = sub_socket.recv_json()
            print(f"[DB SUB] 수신: {msg}")
            insert_study_record(conn, msg)
            print("✅ DB 저장 완료")

            # PUB: 저장 성공 알림 또는 가공된 데이터 전송
            result = {"status": "stored", "user_id": msg["user_id"]}
            pub_socket.send_json(result)
            print("📡 PUB 전송 완료:", result)

        except Exception as e:
            print(f"❌ 에러: {e}")
            time.sleep(1)

if __name__ == "__main__":
    main()
