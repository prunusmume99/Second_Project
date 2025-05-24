# routine_fsm.py
import zmq
import time
from datetime import datetime

# 등록된 UID 목록
AUTHORIZED_UIDS = {
    "180 175 140 4": "윤진",
    "48 207 16 168": "예비 카드"
}

def create_sub_socket(connect_to: str, topic_filter: bytes = b""):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(connect_to)
    socket.setsockopt(zmq.SUBSCRIBE, topic_filter)
    return socket

def create_pub_socket(bind_to: str):
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.connect(bind_to)  # XPUB가 아닌 XSUB 포트로 연결해야 함 (bridge가 중계)
    return socket

def main():
    context = zmq.Context()

    sub_socket = create_sub_socket("tcp://localhost:6001")   # XPUB
    pub_socket = create_pub_socket("tcp://localhost:6000")   # XSUB

    print("[FSM] 센서 메시지 수신 대기 중...")

    while True:
        try:
            message = sub_socket.recv_json()
            print(f"[FSM] 받은 메시지: {message}")

            if message.get("event") == "rfid":
                uid = message.get("uid")
                did = message.get("did")
                timestamp = message.get("timestamp")

                if uid in AUTHORIZED_UIDS:
                    result = "success"
                    name = AUTHORIZED_UIDS[uid]
                else:
                    result = "fail"
                    name = None

                response = {
                    "event": "auth_result",
                    "uid": uid,
                    "did": did,
                    "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                    "result": result,
                    "name": name or "Unknown"
                }

                pub_socket.send_json(response)
                print(f"[FSM] 인증 결과 전송: {response}")

        except Exception as e:
            print(f"⚠️ FSM 처리 오류: {e}")
            time.sleep(0.5)

if __name__ == "__main__":
    main()
