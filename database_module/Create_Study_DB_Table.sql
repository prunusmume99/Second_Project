-- 테이블 생성 명령어
-- mysql -u bangme(유저) -p study_db < study_schema_manual_id.sql
-- sudo mysql -u root -p 로 접속해서 SET GLOBAL log_bin_trust_function_creators = 1;
-- 설정 꼭 하기 이거 안넣으면 테이블 생성 안됨 트리거 떄문에

-- 데이터베이스 생성
CREATE DATABASE IF NOT EXISTS study_db;
USE study_db;

-- 사용자 테이블 생성
CREATE TABLE IF NOT EXISTS User_Table (
    Card_Num VARCHAR(20) PRIMARY KEY COMMENT '사용자 카드 넘버',
    Desk_Num VARCHAR(20) NOT NULL COMMENT '사용자 배정 자리 넘버'
);

-- 학습 기록 테이블 생성
CREATE TABLE IF NOT EXISTS Recd_Table (
    Log_ID INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    Card_Num VARCHAR(20) NOT NULL,
    Status TINYINT NOT NULL DEFAULT 0 COMMENT '0: Break, 1: Study',
    Start_Time DATETIME NULL,
    End_Time DATETIME NULL,
    Amount INT NOT NULL DEFAULT 0 COMMENT '공부 또는 휴식 분 단위 양',
    FOREIGN KEY (Card_Num) REFERENCES User_Table(Card_Num)
);
