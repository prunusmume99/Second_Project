import time
import zmq
import threading

class TimerFSM:
    def __init__(self, study_time, break_time):
        self.study_time = study_time * 60  # 분 -> 초
        self.break_time = break_time * 60
        self.state = '집중'
        self.time_left = self.study_time
        self.cycle_count = 0
        self.is_paused = False

    def tick(self):
        if not self.is_paused:
            if self.time_left > 0:
                self.time_left -= 1
            else:
                if self.state == '집중':
                    self.state = '휴식'
                    self.time_left = self.break_time
                else:
                    self.state = '집중'
                    self.time_left = self.study_time
                    self.cycle_count += 1

    def pause(self):
        self.is_paused = True

    def resume(self):
        self.is_paused = False

    def get_status(self):
        minutes = self.time_left // 60
        seconds = self.time_left % 60
        pause_status = "일시정지" if self.is_paused else "진행중"
        return f"상태: {self.state} ({pause_status}), 남은 시간: {minutes:02d}:{seconds:02d}, 완료 사이클: {self.cycle_count}"

def get_user_time(prompt):
    while True:
        try:
            value = int(input(prompt))
            if value <= 0:
                print("1 이상의 숫자를 입력하세요.")
            else:
                return value
        except ValueError:
            print("숫자를 입력하세요.")

def zmq_listener(timer, context):
    """외부 명령 구독 (일시정지/재개)"""
    socket = context.socket(zmq.SUB)
    socket.connect("tcp://localhost:5556")  # 외부 명령 발행자 주소, 팀과 조율 필요
    socket.setsockopt_string(zmq.SUBSCRIBE, "")  # 모든 메시지 구독

    while True:
        msg = socket.recv_string()
        if msg == "pause":
            timer.pause()
            print("[ZMQ 수신] 타이머 일시정지")
        elif msg == "resume":
            timer.resume()
            print("[ZMQ 수신] 타이머 재개")

def main():
    study_time = get_user_time("공부 시간(분)을 입력하세요: ")
    break_time = get_user_time("휴식 시간(분)을 입력하세요: ")

    timer = TimerFSM(study_time, break_time)

    context = zmq.Context()

    # 상태 발행용 소켓 (PUB)
    pub_socket = context.socket(zmq.PUB)
    pub_socket.bind("tcp://*:5555")  # 포트 5555로 상태 발행 (팀과 조율 필요)

    # 외부 명령 구독 스레드 시작
    listener_thread = threading.Thread(target=zmq_listener, args=(timer, context), daemon=True)
    listener_thread.start()

    try:
        while True:
            status = timer.get_status()
            print(status)
            pub_socket.send_string(status)  # 상태 발행
            time.sleep(1)
            timer.tick()

    except KeyboardInterrupt:
        print("\n프로그램 종료")

if __name__ == "__main__":
    main()
