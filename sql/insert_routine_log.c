// insert_routine_log.c
// 컴파일: gcc insert_routine_log.c -o insert_routine_log -lmysqlclient

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#define HOST "localhost"
#define USER "root"
#define PASS "Dmsrb7867!"
#define DB   "study_db"

int insert_routine_log(MYSQL *conn, const char *user_id, int total_study_time, int pause_count, const char *status) {
    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO routine_log (user_id, total_study_time, pause_count, status) "
        "VALUES ('%s', %d, %d, '%s')",
        user_id, total_study_time, pause_count, status);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "❌ INSERT 실패: %s\n", mysql_error(conn));
        return 0;
    }
    printf("✅ routine_log 삽입 성공: user_id=%s, total_study_time=%d, pause_count=%d, status=%s\n",
           user_id, total_study_time, pause_count, status);
    return 1;
}

int main() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() 실패\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(conn, HOST, USER, PASS, DB, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    char user_id[50];
    int total_study_time;
    int pause_count;
    char status[20];

    printf("▶ 사용자 ID 입력: ");
    fgets(user_id, sizeof(user_id), stdin);
    user_id[strcspn(user_id, "\n")] = 0;

    printf("▶ 총 공부 시간(분) 입력: ");
    scanf("%d", &total_study_time);
    getchar(); // 개행 제거

    printf("▶ 일시 정지 횟수 입력: ");
    scanf("%d", &pause_count);
    getchar();

    printf("▶ 상태 입력 (COMPLETE, PAUSED, FORCE_EXIT): ");
    fgets(status, sizeof(status), stdin);
    status[strcspn(status, "\n")] = 0;

    insert_routine_log(conn, user_id, total_study_time, pause_count, status);

    mysql_close(conn);
    return EXIT_SUCCESS;
}
