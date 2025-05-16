import sqlite3
import zmq

def log_access(uid, result, reason=""):
    conn = sqlite3.connect('auth_log.db')
    cur = conn.cursor()
    cur.execute('INSERT INTO access_log (uid, result, reason) VALUES (?, ?, ?)', (uid, result, reason))
    conn.commit()
    conn.close()
    print(f"[seat_auth_check] 📝 로그 기록 완료: {result}")

def is_uid_authorized(uid):
    conn = sqlite3.connect('rfid_auth.db')
    cur = conn.cursor()
    cur.execute("SELECT user_name FROM authorized_uids WHERE uid = ?", (uid,))
    row = cur.fetchone()
    conn.close()

    if row:
        user_name = row[0]
        print(f"[seat_auth_check] ✅ 인증 성공 ({user_name})")
        log_access(uid, "허용", "등록된 UID")
        return True, user_name
    else:
        print(f"[seat_auth_check] ❌ 인증 실패")
        log_access(uid, "거부", "미등록 UID")
        return False, None

def main():
    context = zmq.Context()
    sub = context.socket(zmq.SUB)
    sub.connect("tcp://localhost:5555")
    sub.setsockopt_string(zmq.SUBSCRIBE, "rfid_auth")

    pub = context.socket(zmq.PUB)
    pub.bind("tcp://*:5556")

    while True:
        msg = sub.recv_string()
        _, uid = msg.split()
        print(f"[seat_auth_check] 🔍 인증 요청 수신 → UID: {uid}")
        status, user = is_uid_authorized(uid)
        result_msg = f"auth_result {'허용' if status else '거부'} {user or '알 수 없음'}"
        pub.send_string(result_msg)
        print(f"[seat_auth_check] 📤 결과 전송: {result_msg}")

if __name__ == "__main__":
    main()
