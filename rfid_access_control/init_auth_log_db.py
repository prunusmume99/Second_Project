import sqlite3

def create_auth_log_db():
    conn = sqlite3.connect('auth_log.db')
    cur = conn.cursor()

    cur.execute('''
    CREATE TABLE IF NOT EXISTS access_log (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        uid TEXT NOT NULL,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        result TEXT CHECK(result IN ('허용', '거부')) NOT NULL,
        reason TEXT
    );
    ''')

    conn.commit()
    conn.close()
    print("✅ auth_log.db 생성 완료")

if __name__ == "__main__":
    create_auth_log_db()
