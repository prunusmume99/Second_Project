import time
import threading
from datetime import datetime

class StudyTimer:
    def __init__(self):
        self.running = False
        self.lock = threading.Lock()

        # 누적 시간 저장 (초)
        self.daily_seconds = 0
        self.weekly_seconds = 0

        # 당일/주간 기준 초기화 시간
        self.current_day = datetime.now().date()
        self.current_week = datetime.now().isocalendar()[1]

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
                print(f"타이머 중지!")
            else:
                print("타이머가 실행 중이 아닙니다.")

    def reset_if_needed(self):
        now = datetime.now()
        today = now.date()
        week = now.isocalendar()[1]

        if today != self.current_day:
            # 날짜가 바뀌면 당일 누적 초기화
            self.daily_seconds = 0
            self.current_day = today

        if week != self.current_week:
            # 주가 바뀌면 주간 누적 초기화
            self.weekly_seconds = 0
            self.current_week = week

    def tick(self):
        while True:
            time.sleep(1)
            with self.lock:
                if self.running:
                    self.daily_seconds += 1
                    self.weekly_seconds += 1
                self.reset_if_needed()

    def get_daily_time(self):
        minutes = self.daily_seconds // 60
        seconds = self.daily_seconds % 60
        return f"오늘 공부 시간: {minutes}분 {seconds}초"

    def get_weekly_time(self):
        minutes = self.weekly_seconds // 60
        seconds = self.weekly_seconds % 60
        return f"이번 주 공부 시간: {minutes}분 {seconds}초"

def main():
    timer = StudyTimer()
    t = threading.Thread(target=timer.tick, daemon=True)
    t.start()

    try:
        while True:
            cmd = input("명령어 입력 (start, stop, daily, weekly, exit): ").strip().lower()
            if cmd == "start":
                timer.start()
            elif cmd == "stop":
                timer.stop()
            elif cmd == "daily":
                print(timer.get_daily_time())
            elif cmd == "weekly":
                print(timer.get_weekly_time())
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
