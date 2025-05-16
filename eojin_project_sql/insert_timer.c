#include <stdio.h>
#include <mysql/mysql.h>
#include <time.h>

int main() {
    MYSQL *conn;
    char query[512];
    char user_id[50];
    int study_time, rest_time, study_cycle = 1, total_study_time;
    time_t start, end;

    printf("유저 ID를 입력하세요: ");
    scanf("%s", user_id);
    getchar();

    printf("🔔 공부를 시작하려면 ENTER를 누르세요...");
    getchar();
    time(&start);
    printf("공부 중입니다... 끝났으면 ENTER를 누르세요.\n");
    getchar();
    time(&end);
    study_time = (int)difftime(end, start);

    printf("☕ 쉬는 시간을 시작하려면 ENTER를 누르세요...");
    getchar();
    time(&start);
    printf("쉬는 중입니다... 끝났으면 ENTER를 누르세요.\n");
    getchar();
    time(&end);
    rest_time = (int)difftime(end, start);

    total_study_time = study_time * study_cycle;

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() 실패\n");
        return 1;
    }

    if (mysql_real_connect(conn, "localhost", "bangme", "djwls123", "study_db", 0, NULL, 0) == NULL) {
        fprintf(stderr, "MySQL 연결 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    snprintf(query, sizeof(query),
        "INSERT INTO study_log (user_id, study_time, rest_time, study_cycle, total_study_time) "
        "VALUES ('%s', %d, %d, %d, %d);",
        user_id, study_time, rest_time, study_cycle, total_study_time);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "INSERT 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    printf("✅ 공부/쉬는 시간 기록 완료!\n");
    mysql_close(conn);
    return 0;
}