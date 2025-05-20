import time
import threading
import random  # 실제 센서 대신 시뮬레이션용

class RoutineManager:
    def __init__(self, study_duration, break_duration, total_cycles):
        self.study_duration = study_duration
        self.break_duration = break_duration
        self.total_cycles = total_cycles

        self.current_cycle = 0
        self.is_paused = False
        self.is_seated = True
        self.state = "IDLE"

        # 착석 상태 감지기 스레드 실행
        self.monitor_thread = threading.Thread(target=self.auto_detect_seated)
        self.monitor_thread.daemon = True
        self.monitor_thread.start()

    def auto_detect_seated(self):
        """착석 상태를 자동으로 감지 (시뮬레이션)"""
        while True:
            # 실제 센서 로직 대신 시뮬레이션
            simulated_seated = random.choice([True, True, True, False])  # 착석 상태 위주
            if simulated_seated != self.is_seated:
                self.set_seated(simulated_seated)
            time.sleep(5)  # 5초마다 감지

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
                print("루틴 일시정지 상태... 자동 재개 대기 중")
                time.sleep(1)  # 잠깐 대기 후 다시 체크
                continue

            if self.state == "FINISHED":
                print("모든 루틴 완료!")
                break

    def countdown(self, duration):
        for remaining in range(duration, 0, -1):
            if self.is_paused or not self.is_seated:
                self.state = "PAUSED"
                print("중단됨 - 타이머 일시정지")
                return
            print(f"남은 시간: {remaining}초", end="\r")
            time.sleep(1)

    def pause(self):
        self.is_paused = True
        self.state = "PAUSED"
        print("루틴 일시정지됨 (미착석 감지됨)")

    def resume(self):
        if self.is_paused and self.is_seated:
            self.is_paused = False
            print("루틴 재개됨 (착석 감지됨)")
            self.state = "STUDY"
            self.run_cycle()

    def set_seated(self, seated):
        self.is_seated = seated
        if not seated:
            self.pause()
        else:
            self.resume()

# 실행 예시
if __name__ == "__main__":
    rm = RoutineManager(study_duration=10, break_duration=5, total_cycles=3)
    rm.start()
