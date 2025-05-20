// gcc insert_user.c -o insert_user -lmysqlclient

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

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
    char name[50];
    char contact[20];

    printf("▶ 사용자 ID를 입력하세요 (문자열): ");
    fgets(user_id, sizeof(user_id), stdin);
    user_id[strcspn(user_id, "\n")] = '\0';

    printf("▶ 이름을 입력하세요: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("▶ 연락처를 입력하세요: ");
    fgets(contact, sizeof(contact), stdin);
    contact[strcspn(contact, "\n")] = '\0';

    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO users (user_id, name, contact) VALUES ('%s', '%s', '%s')",
             user_id, name, contact);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "❌ 사용자 INSERT 실패: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("✅ 사용자 등록 완료!\n");

    mysql_close(conn);
    return EXIT_SUCCESS;
}
