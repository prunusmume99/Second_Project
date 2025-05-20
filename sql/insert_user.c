// gcc insert_user.c -o insert_user -lmysqlclient

#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HOST "localhost"
#define USER "root"
#define PASS "Dmsrb7867!"
#define DB   "study_db"

int main() {
    MYSQL *conn;
    char user_id[100];

    // 유저 아이디 입력
    printf("유저 아이디를 입력하세요: ");
    scanf("%s", user_id);

    // MySQL 연결
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, HOST, USER, PASS, DB, 0, NULL, 0)) {
        fprintf(stderr, "MySQL 연결 실패: %s\n", mysql_error(conn));
        return EXIT_FAILURE;
    }

    // study_record에 초기화
    char query_record[256];
    snprintf(query_record, sizeof(query_record),
             "REPLACE INTO study_record (user_id, study_seconds, break_seconds) "
             "VALUES ('%s', 0, 0);",
             user_id);

    if (mysql_query(conn, query_record)) {
        fprintf(stderr, "study_record 쿼리 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    // study_summary도 초기화
    char query_summary[256];
    snprintf(query_summary, sizeof(query_summary),
             "REPLACE INTO study_summary (user_id, total_study_seconds, study_cycles) "
             "VALUES ('%s', 0, 0);",
             user_id);

    if (mysql_query(conn, query_summary)) {
        fprintf(stderr, "study_summary 쿼리 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("'%s' 유저가 study_record 및 study_summary에 초기화 등록되었습니다.\n", user_id);

    mysql_close(conn);
    return EXIT_SUCCESS;
}