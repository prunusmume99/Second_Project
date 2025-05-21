# insert_user.py

import mysql.connector

# DB 연결 정보
db_config = {
    "host": "localhost",
    "user": "bangme",
    "password": "djwls123",
    "database": "study_db"
}

def main():
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()

        user_id = input("▶ 사용자 ID를 입력하세요 (문자열): ").strip()
        name = input("▶ 이름을 입력하세요: ").strip()
        contact = input("▶ 연락처를 입력하세요: ").strip()

        query = """
            INSERT INTO users (user_id, name, contact)
            VALUES (%s, %s, %s)
        """
        cursor.execute(query, (user_id, name, contact))
        conn.commit()

        print("✅ 사용자 등록 완료!")

    except mysql.connector.Error as err:
        print(f"❌ MySQL 오류: {err}")

    finally:
        if cursor:
            cursor.close()
        if conn:
            conn.close()

if __name__ == "__main__":
    main()
