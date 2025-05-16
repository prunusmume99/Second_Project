#include <stdio.h>
#include <mysql/mysql.h>

int main() {
    MYSQL *conn;
    char query[512];
    char user_id[50];
    int study_time, rest_time, study_cycle, total_study_time;

    printf("유저 ID: ");
    scanf("%s", user_id);
    printf("공부 시간(분): ");
    scanf("%d", &study_time);
    printf("쉬는 시간(분): ");
    scanf("%d", &rest_time);
    printf("공부 사이클 횟수: ");
    scanf("%d", &study_cycle);

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

    printf("✅ 데이터 삽입 완료!\n");
    mysql_close(conn);
    return 0;
}