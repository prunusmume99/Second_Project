from ..core_logic.routine_manager import RoutineManager


def main():
    print("루틴 타이머 시작")
    study_time = int(input("공부 시간(초)을 입력하세요: "))
    break_time = int(input("휴식 시간(초)을 입력하세요: "))
    total_cycles = int(input("총 반복 횟수를 입력하세요: "))

    manager = RoutineManager(study_time, break_time, total_cycles)
    manager.start()

    while True:
        cmd = input("\n명령어 입력 (pause/resume/seated/left/exit): ").strip().lower()

        if cmd == "pause":
            manager.pause()
        elif cmd == "resume":
            manager.resume()
        elif cmd == "seated":
            manager.set_seated(True)
        elif cmd == "left":
            manager.set_seated(False)
        elif cmd == "exit":
            print("프로그램 종료")
            break
        else:
            print("알 수 없는 명령어입니다.")

if __name__ == "__main__":
    main()
