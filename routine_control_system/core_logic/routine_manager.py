import time

class RoutineManager:
    def __init__(self, study_duration, break_duration, total_cycles):
        self.study_duration = study_duration    # 공부 시간 (초)
        self.break_duration = break_duration    # 휴식 시간 (초)
        self.total_cycles = total_cycles        # 전체 루틴 반복 횟수

        self.current_cycle = 0                   # 현재 몇 번째 루틴 중인지
        self.is_paused = False                   # 중단 상태 여부
        self.is_seated = True                    # 착석 상태 (True: 착석, False: 미착석)
        self.state = "IDLE"                      # 상태 (IDLE, STUDY, BREAK, PAUSED)

    def start(self):
        self.current_cycle = 1
        self.state = "STUDY"
        print(f"루틴 시작: 총 {self.total_cycles}회 반복")
        self.run_cycle()

    def run_cycle(self):
        while self.current_cycle <= self.total_cycles:
            print(f"\n[{self.current_cycle}회차] 상태: {self.state}")

            if self.state == "STUDY":
                self.countdown(self.study_duration)
                self.state = "BREAK"
            elif self.state == "BREAK":
                self.countdown(self.break_duration)
                self.current_cycle += 1
                if self.current_cycle <= self.total_cycles:
                    self.state = "STUDY"
                else:
                    self.state = "FINISHED"
            elif self.state == "PAUSED":
                print("루틴 일시정지 상태... 계속하려면 resume() 호출")
                break

            if self.state == "FINISHED":
                print("모든 루틴 완료!")
                break

    def countdown(self, duration):
        for remaining in range(duration, 0, -1):
            if self.is_paused or not self.is_seated:
                self.state = "PAUSED"
                print("중단됨 - 타이머 일시정지")
                break
            print(f"남은 시간: {remaining}초", end="\r")
            time.sleep(1)

    def pause(self):
        self.is_paused = True
        self.state = "PAUSED"
        print("루틴 일시정지됨")

    def resume(self):
        if self.is_paused:
            self.is_paused = False
            print("루틴 재개됨")
            self.state = "STUDY"  # 중단된 상태에 따라 조절 가능
            self.run_cycle()

    def set_seated(self, seated):
        self.is_seated = seated
        if not seated:
            self.pause()
        else:
            self.resume()

if __name__ == "__main__":
    rm = RoutineManager(study_duration=10, break_duration=5, total_cycles=3)
    rm.start()
