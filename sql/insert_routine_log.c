#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

#define HOST "localhost"
#define USER "root"
#define PASS "Dmsrb7867!"
#define DB   "study_db"

int main() {
    MYSQL *conn;
    char user_id[51];
    int total_study_time;
    int pause_count;
    char status[10];  // 'COMPLETE', 'PAUSED', 'FORCE_EXIT'

    // 입력 받기
    printf("User ID를 입력하세요 (최대 50자): ");
    scanf("%50s", user_id);

    printf("총 공부 시간(초)을 입력하세요: ");
    scanf("%d", &total_study_time);

    printf("일시정지 횟수를 입력하세요: ");
    scanf("%d", &pause_count);

    printf("상태를 입력하세요 (COMPLETE, PAUSED, FORCE_EXIT): ");
    scanf("%9s", status);

    // MySQL 연결 초기화
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, HOST, USER, PASS, DB, 0, NULL, 0)) {
        fprintf(stderr, "MySQL 연결 실패: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    // 쿼리문 작성
    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO routine_log (user_id, total_study_time, pause_count, status) "
        "VALUES ('%s', %d, %d, '%s');",
        user_id, total_study_time, pause_count, status);

    // 쿼리 실행
    if (mysql_query(conn, query)) {
        fprintf(stderr, "쿼리 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("routine_log에 데이터가 성공적으로 삽입되었습니다.\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}
