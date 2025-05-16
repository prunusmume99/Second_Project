import time

class TimerFSM:
    def __init__(self, study_minutes, break_minutes):
        self.study_time = study_minutes * 60  # 분 → 초
        self.break_time = break_minutes * 60
        self.state = '집중'  # 시작 상태
        self.time_left = self.study_time
        self.cycle_count = 0

    def tick(self):
        if self.time_left > 0:
            self.time_left -= 1
        else:
            if self.state == '집중':
                self.state = '휴식'
                self.time_left = self.break_time
            else:
                self.state = '집중'
                self.time_left = self.study_time
                self.cycle_count += 1  # 공부+휴식 완료 시 카운트

    def get_status(self):
        minutes = self.time_left // 60
        seconds = self.time_left % 60
        return (f"상태: {self.state} | 남은 시간: {minutes:02d}:{seconds:02d} | "
                f"완료된 루틴 수: {self.cycle_count}")

def get_valid_time(prompt):
    while True:
        try:
            value = int(input(prompt))
            if value <= 0:
                print("⚠️  1 이상의 숫자를 입력하세요.")
            else:
                return value
        except ValueError:
            print("⚠️  숫자만 입력해주세요.")

def main():
    print("===== 맞춤형 공부 타이머 시작 =====")
    study = get_valid_time("공부 시간(분)을 입력하세요: ")
    rest = get_valid_time("휴식 시간(분)을 입력하세요: ")

    fsm = TimerFSM(study, rest)

    try:
        while True:
            print(fsm.get_status(), end='\r', flush=True)  # 한 줄로 상태 계속 갱신
            time.sleep(1)
            fsm.tick()
    except KeyboardInterrupt:
        print("\n⏹ 타이머 종료. 총 완료 루틴 수:", fsm.cycle_count)

if __name__ == "__main__":
    main()
