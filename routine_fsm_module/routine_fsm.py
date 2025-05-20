# routine_fsm_module/routine_fsm.py
# FSM 모듈: 상태 전이 및 메시지 수신 처리
# 이 메시지를 기반으로 상태 전이 로직 수행할 예정

import zmq

# ZMQ SUB 소켓을 생성하는 함수
# 이 소켓은 브로커의 XPUB 포트에 연결되어 데이터를 수신하는 역할을 함
def create_sub_socket(connect_to: str, topic_filter: bytes = b""):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(connect_to)  # XPUB 포트에 연결
    socket.setsockopt(zmq.SUBSCRIBE, topic_filter)  # 빈 값이면 모든 메시지 수신
    return socket

def main():
    sub_socket = create_sub_socket("tcp://localhost:6001")

    print("[FSM] 센서 메시지 수신 대기 중...")
    while True:
        message = sub_socket.recv_json()
        print(f"[FSM] 받은 메시지 : {message}")
        # 이 메시지를 기반으로 상태 전이 로직 수행 예정

if __name__ == "__main__":
    main()