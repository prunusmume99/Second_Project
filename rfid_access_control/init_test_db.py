import sqlite3

def create_test_db():
    conn = sqlite3.connect('rfid_auth.db')
    cur = conn.cursor()

    cur.execute('''
    CREATE TABLE IF NOT EXISTS authorized_uids (
        uid TEXT PRIMARY KEY,
        user_name TEXT NOT NULL
    );
    ''')

    cur.executemany('''
    INSERT INTO authorized_uids (uid, user_name) VALUES (?, ?)
    ''', [
        ('04A1BC23D5', '윤진'),
        ('123456789A', '김동필'),
    ])

    conn.commit()
    conn.close()
    print("✅ rfid_auth.db 생성 완료")

if __name__ == "__main__":
    create_test_db()
