// gcc study_timer.c -o study_timer -lmysqlclient

#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOST "localhost"
#define USER "root"
#define PASS "Dmsrb7867!"
#define DB   "study_db"

void wait_for_enter(const char *message) {
    printf("%s", message);
    while (getchar() != '\n'); // 엔터 대기
}

int main() {
    MYSQL *conn;
    char user_id[100];
    time_t study_start, study_end, break_start, break_end;

    // 유저 아이디 입력
    printf("유저 아이디를 입력하세요: ");
    scanf("%s", user_id);
    getchar(); // 엔터 처리

    // 공부 시작
    wait_for_enter("엔터를 누르면 공부가 시작됩니다...");
    study_start = time(NULL);
    printf("공부 시작 시간: %s", ctime(&study_start));

    // 공부 종료 + 쉬는 시간 시작
    wait_for_enter("엔터를 누르면 공부가 끝나고 쉬는 시간이 시작됩니다...");
    study_end = time(NULL);
    break_start = study_end;
    printf("쉬는 시간 시작 시간: %s", ctime(&break_start));

    // 쉬는 시간 종료
    wait_for_enter("엔터를 누르면 쉬는 시간이 끝납니다...");
    break_end = time(NULL);
    printf("쉬는 시간 끝 시간: %s", ctime(&break_end));

    int study_seconds = (int)(study_end - study_start);
    int break_seconds = (int)(break_end - break_start);

    printf("\n측정된 공부 시간: %d초\n", study_seconds);
    printf("측정된 쉬는 시간: %d초\n", break_seconds);

    // MySQL 연결
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, HOST, USER, PASS, DB, 0, NULL, 0)) {
        fprintf(stderr, "MySQL 연결 실패: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    // study_record 업데이트
    char record_query[512];
snprintf(record_query, sizeof(record_query),
    "INSERT INTO study_record (user_id, study_seconds, break_seconds) "
    "VALUES ('%s', %d, %d) "
    "ON DUPLICATE KEY UPDATE study_seconds = %d, break_seconds = %d;",
    user_id, study_seconds, break_seconds, study_seconds, break_seconds);

    if (mysql_query(conn, record_query)) {
        fprintf(stderr, "study_record 업데이트 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    // study_summary 업데이트 (누적)
    char summary_query[512];
    snprintf(summary_query, sizeof(summary_query),
             "INSERT INTO study_summary (user_id, total_study_seconds, study_cycles) "
             "VALUES ('%s', %d, FLOOR(%d / 30)) "
             "ON DUPLICATE KEY UPDATE "
             "total_study_seconds = total_study_seconds + VALUES(total_study_seconds), "
             "study_cycles = FLOOR((total_study_seconds + VALUES(total_study_seconds)) / 30);",
             user_id, study_seconds, study_seconds);

    if (mysql_query(conn, summary_query)) {
        fprintf(stderr, "study_summary 업데이트 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("DB에 저장 완료!\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}