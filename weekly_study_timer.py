import time
import threading
from datetime import datetime

class WeeklyTimerFSM:
    def __init__(self):
        self.seconds = 0
        self.running = False
        self.lock = threading.Lock()
        self.current_week = self.get_current_week()

    def get_current_week(self):
        # 현재 연도와 주차 반환 (예: 2025-21)
        now = datetime.now()
        year, week_num, _ = now.isocalendar()
        return (year, week_num)

    def reset_if_new_week(self):
        week_now = self.get_current_week()
        if week_now != self.current_week:
            with self.lock:
                self.seconds = 0
                self.current_week = week_now
                print(f"\n새로운 주({self.current_week[0]}년 {self.current_week[1]}주차) 시작! 시간 초기화됨.")

    def start(self):
        with self.lock:
            if not self.running:
                self.running = True
                print("타이머 시작!")
            else:
                print("타이머가 이미 실행 중입니다.")

    def stop(self):
        with self.lock:
            if self.running:
                self.running = False
                print(f"타이머 중지! 이번 주 총 경과 시간: {self.seconds // 3600}시간 "
                      f"{(self.seconds % 3600) // 60}분 {self.seconds % 60}초")
            else:
                print("타이머가 실행 중이 아닙니다.")

    def tick(self):
        while True:
            time.sleep(1)
            self.reset_if_new_week()
            with self.lock:
                if self.running:
                    self.seconds += 1

def main():
    timer = WeeklyTimerFSM()
    t = threading.Thread(target=timer.tick, daemon=True)
    t.start()

    try:
        while True:
            cmd = input("명령어 입력 (start, stop, exit): ").strip().lower()
            if cmd == "start":
                timer.start()
            elif cmd == "stop":
                timer.stop()
            elif cmd == "exit":
                if timer.running:
                    timer.stop()
                print("프로그램 종료")
                break
            else:
                print("알 수 없는 명령어입니다.")
    except KeyboardInterrupt:
        if timer.running:
            timer.stop()
        print("\n프로그램 강제 종료")

if __name__ == "__main__":
    main()
