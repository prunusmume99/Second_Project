import os
import datetime

class RoutineLogger:
    def __init__(self, log_dir='logs', log_file='routine_log.txt'):
        self.log_dir = log_dir
        self.log_file = log_file
        
        # 로그 폴더가 없으면 생성
        if not os.path.exists(self.log_dir):
            os.makedirs(self.log_dir)
        
        # 전체 경로
        self.log_path = os.path.join(self.log_dir, self.log_file)
    
    def log_state(self, routine_id, state, message=''):
        """
        루틴 상태를 로그 파일에 기록
        :param routine_id: 루틴 식별자 (예: 'morning_routine')
        :param state: 상태 (예: 'STARTED', 'IN_PROGRESS', 'COMPLETED')
        :param message: 추가 메시지 (선택)
        """
        timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        log_entry = f"{timestamp} | Routine: {routine_id} | State: {state} | Message: {message}\n"
        
        with open(self.log_path, 'a') as f:
            f.write(log_entry)
    
    def read_logs(self):
        """로그 파일 전체를 읽어서 반환"""
        if not os.path.exists(self.log_path):
            return []
        
        with open(self.log_path, 'r') as f:
            return f.readlines()

# 간단한 테스트 코드 (직접 실행 시)
if __name__ == '__main__':
    logger = RoutineLogger()
    logger.log_state('morning_routine', 'STARTED', 'Routine 시작')
    logger.log_state('morning_routine', 'IN_PROGRESS', '사용자 운동 중')
    logger.log_state('morning_routine', 'COMPLETED', '루틴 완료')

    print("현재 로그:")
    logs = logger.read_logs()
    for line in logs:
        print(line.strip())
