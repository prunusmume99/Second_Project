// gcc insert_record.c -o insert_record -lmysqlclient

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql/mysql.h>

#define TIME_STR_SIZE 20

void get_time_string(time_t raw_time, char *buffer, size_t size) {
    struct tm *t = localtime(&raw_time);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

int insert_record(MYSQL *conn, const char *user_id, const char *start, const char *end, int mode, int action) {
    char query[512];
    snprintf(query, sizeof(query),
        "INSERT INTO study_record (user_id, start_time, end_time, mode, action) "
        "VALUES ('%s', '%s', '%s', %d, %d)",
        user_id, start, end, mode, action);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "❌ INSERT 실패 (%d): %s\n", action, mysql_error(conn));
        return 0;
    }

    printf("✅ %s 기록 완료: %s ~ %s\n", action == 1 ? "공부" : "휴식", start, end);
    return 1;
}

int main() {
    MYSQL *conn;
    const char *server = "localhost";
    const char *user = "bangme";
    const char *password = "djwls123";
    const char *database = "study_db";

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() 실패\n");
        return EXIT_FAILURE;
    }

    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    char user_id[20];
    int mode;

    printf("▶ 사용자 ID를 입력하세요 (문자열): ");
    fgets(user_id, sizeof(user_id), stdin);
    user_id[strcspn(user_id, "\n")] = '\0';

    printf("▶ 모드를 입력하세요 (0: 사용자 정의, 1: 뽀모도로): ");
    scanf("%d", &mode);
    getchar(); // 개행 제거

    time_t t_study_start, t_study_end, t_rest_end;
    char study_start[TIME_STR_SIZE], study_end[TIME_STR_SIZE];
    char rest_start[TIME_STR_SIZE], rest_end[TIME_STR_SIZE];

    // 1. 공부 시작
    printf("▶ 공부를 시작하려면 Enter 키를 누르세요...");
    getchar();
    t_study_start = time(NULL);
    get_time_string(t_study_start, study_start, TIME_STR_SIZE);
    printf("✅ 공부 시작: %s\n", study_start);

    // 2. 공부 종료 + 휴식 시작
    printf("▶ 공부 종료 및 휴식 시작을 하려면 Enter 키를 누르세요...");
    getchar();
    t_study_end = time(NULL);
    get_time_string(t_study_end, study_end, TIME_STR_SIZE);
    get_time_string(t_study_end, rest_start, TIME_STR_SIZE); // 휴식 시작은 공부 종료와 같음

    insert_record(conn, user_id, study_start, study_end, mode, 1); // 공부 기록

    // 3. 휴식 종료
    printf("▶ 휴식을 종료하려면 Enter 키를 누르세요...");
    getchar();
    t_rest_end = time(NULL);
    get_time_string(t_rest_end, rest_end, TIME_STR_SIZE);

    insert_record(conn, user_id, rest_start, rest_end, mode, 0); // 휴식 기록

    printf("🎉 루틴 완료! 프로그램을 종료합니다.\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}
