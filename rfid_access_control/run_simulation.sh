#!/bin/bash

echo "📦 Step 1: 데이터베이스 초기화 중..."
python3 init_test_db.py
python3 init_auth_log_db.py

echo ""
echo "🧠 Step 2: 인증 처리 서버 실행 (백그라운드)..."
gnome-terminal -- bash -c "python3 seat_auth_check.py; exec bash"

sleep 1

echo "📺 Step 3: LCD 피드백 시뮬레이터 실행 (백그라운드)..."
gnome-terminal -- bash -c "./lcd_feedback; exec bash"

sleep 1

echo ""
echo "🛂 Step 4: UID 테스트 시뮬레이션 시작..."
python3 test_uid_simulation.py

echo ""
echo "📜 Step 5: 최근 인증 로그 5건 확인:"
sqlite3 auth_log.db "SELECT * FROM access_log ORDER BY id DESC LIMIT 5;"

echo ""
echo "✅ 모든 시뮬레이션 완료!"
