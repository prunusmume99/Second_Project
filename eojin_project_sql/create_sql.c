#include <stdio.h>

int main() {
    FILE *fp = fopen("create_table.sql", "w");

    if (fp == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    fprintf(fp, "CREATE TABLE study_log (\n");
    fprintf(fp, "    id INT AUTO_INCREMENT PRIMARY KEY,\n");
    fprintf(fp, "    user_id VARCHAR(50) NOT NULL,\n");
    fprintf(fp, "    study_time INT NOT NULL,\n");
    fprintf(fp, "    rest_time INT NOT NULL,\n");
    fprintf(fp, "    study_cycle INT NOT NULL,\n");
    fprintf(fp, "    total_study_time INT NOT NULL\n");
    fprintf(fp, ");\n");

    fclose(fp);
    printf("✅ create_table.sql 파일이 생성되었습니다.\n");

    return 0;
}