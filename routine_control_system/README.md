1. routine_manager.py
루틴 관리 클래스 RoutineManager를 구현한 파일

공부 시간과 휴식 시간, 전체 반복 횟수를 설정하여 루틴을 자동으로 실행

착석 여부와 중단 상태를 감지해 루틴을 일시정지하거나 재개할 수 있음

현재 상태, 남은 시간, 반복 횟수를 출력하며 터미널에서 실행 가능

2. command_line.py
사용자 CLI 환경을 제공하는 인터페이스 스크립트

RoutineManager 클래스를 이용해 루틴 시작, 일시정지, 재개, 착석 상태 변경 등의 명령어 처리

현재 루틴 상태, 남은 시간, 반복 횟수를 실시간 출력

사용자의 입력에 따라 루틴 동작을 제어하며 테스트 및 시뮬레이션에 사용됨

3. routine_tests.py
RoutineManager 클래스 기능을 검증하는 단위 테스트 스크립트

다양한 시나리오로 루틴의 중단, 재개, 타이머 동작 등을 자동으로 점검

단독 실행 불가하며, 프로젝트 최상위 디렉토리에서 python -m unittest routine_control_system.test_cases.routine_tests 명령으로 실행해야 정상 작동

프로젝트 내 핵심 모듈(routine_manager.py)과 함께 사용해야 하므로, 독립적인 스크립트가 아님