import zmq
import time
from datetime import datetime

AUTHORIZED_UIDS = {
    "180 175 140 4": "윤진",
    "48 207 16 168": "예비 카드"
}

def create_sub_socket(context, connect_to: str, topic_filter: bytes = b""):
    socket = context.socket(zmq.SUB)
    socket.connect(connect_to)
    socket.setsockopt(zmq.SUBSCRIBE, topic_filter)
    return socket

def create_pub_socket(context, connect_to: str):
    socket = context.socket(zmq.PUB)
    socket.connect(connect_to)
    return socket

def main():
    context = zmq.Context()
    sub_socket = create_sub_socket(context, "tcp://localhost:6001")
    pub_socket = create_pub_socket(context, "tcp://localhost:6000")

    poller = zmq.Poller()
    poller.register(sub_socket, zmq.POLLIN)

    print("[FSM] 센서 메시지 수신 대기 중...")

    while True:
        socks = dict(poller.poll(timeout=1000))  # 1초마다 체크

        if sub_socket in socks:
            try:
                message = sub_socket.recv_json()
                print(f"[FSM] 받은 메시지: {message}")

                # 자신이 보낸 메시지는 무시
                if message.get("event") == "auth_result":
                    continue

                if message.get("event") == "rfid":
                    uid = message.get("uid")
                    did = message.get("did")
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
                print(f"⚠️ FSM 처리 중 오류: {e}")

        else:
            print("💤 메시지 없음, 대기 중...")

if __name__ == "__main__":
    main()
