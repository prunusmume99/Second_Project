  # sensor_input_module/sensor_packet_sender.py

import zmq
import time
import json # 현재는 사용하지 않지만, 나중에 코드 확장 시를 대비해 미리 임포트
from datetime import datetime # datetime 모듈을 사용하여 현재 시간을 ISO 포맷으로 변환

# ZMQ PUB 소켓을 생성하는 함수
# 이 소켓은 브로커의 XSUB 포트에 연결되어 데이터를 전송하는 역할을 함
def create_pub_socket(bind_to: str):
    context = zmq.Context()
    socket = context.socket(zmq.PUB)
    socket.connect(bind_to)  # 브로커의 XSUB 포트에 연결
    return socket

# 더미 센서 데이터를 생성하는 함수
# 이 데이터는 JSON 형식으로 변환되어 PUB 소켓을 통해 전송됨
# 현재는 고정된 값으로 설정되어 있지만, 실제 센서 데이터로 대체 가능
def generate_dummy_sensor_data():
    return {
        "desk_id": "D12",
        "event": "touch",
        "value": 1,
        "timestamp": datetime.now().isoformat()
    }

# ZMQ PUB 소켓을 생성하고, 1초마다 더미 센서 데이터를 전송하는 루프를 실행
# 이 루프는 무한히 반복되며, Ctrl+C로 종료할 수 있음
# 데이터는 send_json() 함수에 의해 자동으로 JSON 문자열로 변환되어 전송됨

def main():
    # ZMQ 브로커의 XSUB 포트에 연결
    pub_socket = create_pub_socket("tcp://localhost:6000")
    
    print("[Sender] 더미 센서 데이터 1초마다 무한 전송 중...")
    while True:
        data = generate_dummy_sensor_data()
        pub_socket.send_json(data)
        print(f"[Sender] Sent: {data}")
        time.sleep(1)

if __name__ == "__main__":
    main()