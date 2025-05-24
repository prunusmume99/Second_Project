import zmq
from datetime import datetime

# === 임시 인증 UID 리스트 (향후 DB로 대체 가능) ===
AUTHORIZED_UIDS = {
    "180 175 140 4": "윤진",
    "48 207 16 168": "예비 카드 A",
    "85 163 163 4": "예비 카드 B",
    "69 39 144 4": "예비 카드 C"
}

def create_sub_socket(context, connect_to: str, topic_filter: bytes = b""):
    socket = context.socket(zmq.SUB)
    socket.connect(connect_to)  # XPUB 포트
    socket.setsockopt(zmq.SUBSCRIBE, topic_filter)
    return socket

def create_pub_socket(context, connect_to: str):
    socket = context.socket(zmq.PUB)
    socket.connect(connect_to)  # XSUB 포트
    return socket

def main():
    context = zmq.Context()

    sub_socket = create_sub_socket(context, "tcp://localhost:6001")
    pub_socket = create_pub_socket(context, "tcp://localhost:6000")

    print("[FSM] 센서 메시지 수신 대기 중...")

    while True:
        try:
            message = sub_socket.recv_json()
            print(f"[FSM] 받은 메시지: {message}")

            if message.get("event") == "rfid":
                uid = message.get("uid")
                did = message.get("desk_id") or message.get("did")  # 호환성 고려
                timestamp = message.get("timestamp")

                if uid in AUTHORIZED_UIDS:
                    result = "success"
                    name = AUTHORIZED_UIDS[uid]
                else:
                    result = "fail"
                    name = "Unknown"

                response = {
                    "event": "auth_result",
                    "uid": uid,
                    "did": did,
                    "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                    "result": result,
                    "name": name
                }

                pub_socket.send_json(response)
                print(f"[FSM] 인증 결과 전송: {response}")

        except Exception as e:
            print(f"⚠️ FSM 처리 오류: {e}")

if __name__ == "__main__":
    main()
