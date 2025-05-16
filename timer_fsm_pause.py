import time

class TimerFSM:
    def __init__(self, study_time, break_time):
        self.study_time = study_time * 60  # 분 -> 초
        self.break_time = break_time * 60
        self.state = '집중'
        self.time_left = self.study_time
        self.cycle_count = 0
        self.is_paused = False  # 착석 상태 시뮬레이션용

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

def main():
    study_time = get_user_time("공부 시간(분)을 입력하세요: ")
    break_time = get_user_time("휴식 시간(분)을 입력하세요: ")

    timer = TimerFSM(study_time, break_time)

    try:
        while True:
            print(timer.get_status())
            time.sleep(1)
            timer.tick()

            # 시뮬레이션: 공부 시작 후 10초 뒤 일시정지 → 5초 후 재개
            if timer.state == '집중' and timer.time_left == (study_time * 60 - 10):
                print("💤 착석 중단! 타이머 일시정지...")
                timer.pause()
                time.sleep(5)
                print("✅ 착석 복귀! 타이머 재개...")
                timer.resume()

    except KeyboardInterrupt:
        print("\n⛔ 프로그램 종료")

if __name__ == "__main__":
    main()
