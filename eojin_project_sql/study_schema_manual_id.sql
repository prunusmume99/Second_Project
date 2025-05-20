-- 테이블 생성 명령어
-- mysql -u bangme(유저) -p study_db < study_schema_manual_id.sql
-- sudo mysql -u root -p 로 접속해서 SET GLOBAL log_bin_trust_function_creators = 1;
-- 설정 꼭 하기 이거 안넣으면 테이블 생성 안됨 트리거 떄문에

-- 데이터베이스 생성
CREATE DATABASE IF NOT EXISTS study_db;
USE study_db;

-- 사용자 테이블 생성
CREATE TABLE IF NOT EXISTS users (
    user_id VARCHAR(20) PRIMARY KEY,
    name VARCHAR(50) NOT NULL,
    contact VARCHAR(20) NOT NULL
);

-- 학습 기록 테이블 생성
CREATE TABLE IF NOT EXISTS study_record (
    record_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(20) NOT NULL,
    start_time DATETIME NOT NULL,
    end_time DATETIME NOT NULL,
    mode TINYINT(1) NOT NULL COMMENT '0: 사용자 정의, 1: 뽀모도로',
    action TINYINT(1) NOT NULL COMMENT '0: 휴식, 1: 공부',
    duration TIME NOT NULL COMMENT '공부 또는 휴식 시간의 총 길이',
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- 트리거 생성: 새로운 기록이 삽입될 때 duration 자동 계산
DELIMITER $$

CREATE TRIGGER before_insert_study_record
BEFORE INSERT ON study_record
FOR EACH ROW
BEGIN
    SET NEW.duration = TIMEDIFF(NEW.end_time, NEW.start_time);
END$$

DELIMITER ;
