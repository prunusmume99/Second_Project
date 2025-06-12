 # communication module/zmq_router.py
# pip install pyzmq 

import zmq

def main():
    context = zmq.Context()

    # XSUB 소켓: 퍼블리셔로부터 받는 쪽
    xsub_socket = context.socket(zmq.XSUB)
    xsub_socket.bind("tcp://*:6000")  # 예: sensor, rfid 등에서 PUB

    # XPUB 소켓: 서브스크라이버에게 보내는 쪽
    xpub_socket = context.socket(zmq.XPUB)
    xpub_socket.bind("tcp://*:6001")  # 예: fsm, db 등에서 SUB

    print("[ZMQ Router] 메시지 중계 라우터 작동 중...")

    # ZMQ proxy: 메시지 릴레이
    # ZMQ의 내부 프록시 기능을 사용해 XSUP -> XPUB 으로 메시지를 중계
    try:
        zmq.proxy(xsub_socket, xpub_socket)
    except KeyboardInterrupt:
        print("\n[ZMQ Router] 사용자 요청으로 중단됨")
    finally:
        xsub_socket.close()
        xpub_socket.close()
        context.term()

if __name__ == "__main__":
    main()