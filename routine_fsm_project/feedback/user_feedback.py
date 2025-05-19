def print_user_feedback(study_time, pause_count, resume_count, complete_cycles):
    print("===== 루틴 종료 후 사용자 피드백 요약 =====")
    print(f"총 공부 시간: {study_time}분")
    print(f"중단된 횟수: {pause_count}회")
    print(f"재개된 횟수: {resume_count}회")
    print(f"완료된 루틴 횟수: {complete_cycles}회")

if __name__ == "__main__":
    # 테스트용 더미 데이터
    study_time = 60
    pause_count = 2
    resume_count = 3
    complete_cycles = 5

    print_user_feedback(study_time, pause_count, resume_count, complete_cycles)
