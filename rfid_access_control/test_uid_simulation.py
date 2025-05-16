import zmq
import time

context = zmq.Context()
pub = context.socket(zmq.PUB)
pub.bind("tcp://*:5555")

time.sleep(1)  # socket 안정화 시간
test_uids = ["04A1BC23D5", "0000000000"]

for uid in test_uids:
    msg = f"rfid_auth {uid}"
    pub.send_string(msg)
    print(f"[UID 전송] ✅ UID: {uid}")
    time.sleep(1)
