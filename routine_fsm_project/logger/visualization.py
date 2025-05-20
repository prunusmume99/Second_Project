from datetime import datetime

class RoutineVisualizer:
    def __init__(self, log_path='logs/routine_log.txt'):
        self.log_path = log_path

    def parse_logs(self):
        records = []
        try:
            with open(self.log_path, 'r') as f:
                for line in f:
                    parts = line.strip().split('|')
                    if len(parts) < 3:
                        continue
                    timestamp_str = parts[0].strip()
                    routine_part = parts[1].strip()
                    state_part = parts[2].strip()
                    
                    timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S')
                    routine_id = routine_part.replace('Routine:', '').strip()
                    state = state_part.replace('State:', '').strip()
                    records.append((timestamp, routine_id, state))
        except FileNotFoundError:
            print(f"로그 파일이 없습니다: {self.log_path}")
        return records

    def print_state_summary(self, routine_id_filter=None):
        records = self.parse_logs()
        if not records:
            print("로그 데이터가 없습니다.")
            return

        if routine_id_filter:
            records = [r for r in records if r[1] == routine_id_filter]

        if not records:
            print(f"'{routine_id_filter}' 루틴에 대한 로그가 없습니다.")
            return

        print(f"=== '{routine_id_filter or '전체'}' 루틴 상태 로그 ===")
        for timestamp, routine_id, state in records:
            print(f"{timestamp} | {routine_id} | {state}")

if __name__ == '__main__':
    visualizer = RoutineVisualizer()
    # 특정 루틴 ID 넣거나 None으로 전체 출력 가능
    visualizer.print_state_summary('morning_routine')
