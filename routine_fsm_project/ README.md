- **feedback/user_feedback.py**  
  루틴 종료 시 총 공부 시간, 중단 횟수, 재개 횟수, 완료된 루틴 횟수 등  
  사용자 피드백 요약을 출력하는 함수 포함
  독립 실행 테스트도 가능한 코드

---
routine_logger.py
루틴 상태 변화를 시간과 함께 기록하는 모듈입

루틴이 시작(STARTED), 진행(IN_PROGRESS), 완료(COMPLETED)될 때마다 해당 상태를 logs/routine_log.txt 파일에 저장

기록 형식은 [타임스탬프] | Routine: [루틴ID] | State: [상태] 형태로 되어 있어, 시간 순서대로 루틴 상태 변화를 추적 가능

visualization.py
routine_logger.py가 생성한 로그 파일을 읽어서 루틴 상태 기록을 분석 및 출력하는 모듈

원래는 matplotlib를 이용해 시간에 따른 상태 변화와 상태별 빈도를 시각화하는 그래프를 만들 계획이었으나, 현재 프로젝트 실행 환경의 제한(그래프 창 출력 불가 등)으로 인해 그래프 기능 구현이 완료되지 않았습니다.

대신 로그 내용을 터미널에 출력하는 기본적인 상태 요약 기능만 구현되어 있음


logs/routine_log.txt
routine_logger.py가 루틴 상태 변화를 기록하는 로그 파일

시간, 루틴 ID, 상태 정보를 포함하며, 프로젝트 내 루틴 상태 추적 및 분석의 기본 데이터가 됨
