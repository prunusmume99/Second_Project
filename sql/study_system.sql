CREATE DATABASE IF NOT EXISTS study_db;
USE study_db;

-- 1️⃣ study_record: 유저별 공부/쉬는 시간 (초 단위)
CREATE TABLE IF NOT EXISTS study_record (
    user_id VARCHAR(50) PRIMARY KEY,
    study_seconds INT NOT NULL DEFAULT 0,
    break_seconds INT NOT NULL DEFAULT 0,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);

-- 2️⃣ study_summary: 누적 공부 시간 및 30초 단위 사이클 계산
CREATE TABLE IF NOT EXISTS study_summary (
    user_id VARCHAR(50) PRIMARY KEY,
    total_study_seconds INT NOT NULL DEFAULT 0,
    study_cycles INT NOT NULL DEFAULT 0,
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES study_record(user_id)
);

-- 3️⃣ study_routine: 루틴 성공/실패 결과 저장 테이블
CREATE TABLE IF NOT EXISTS study_routine (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(50) NOT NULL,
    routine_result ENUM('성공', '실패') NOT NULL,
    cycle_count INT NOT NULL,
    recorded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES study_summary(user_id)
);

-- 4️⃣ 트리거: study_summary에 INSERT 시 루틴 결과 자동 기록
DELIMITER $$

CREATE TRIGGER trg_routine_check_insert
AFTER INSERT ON study_summary
FOR EACH ROW
BEGIN
    INSERT INTO study_routine (user_id, routine_result, cycle_count)
    VALUES (
        NEW.user_id,
        CASE
            WHEN NEW.study_cycles >= 3 THEN '성공'
            ELSE '실패'
        END,
        NEW.study_cycles
    );
END$$

DELIMITER ;

-- 5️⃣ 트리거: study_summary에 UPDATE 시 루틴 결과 자동 기록
DELIMITER $$

CREATE TRIGGER trg_routine_check_update
AFTER UPDATE ON study_summary
FOR EACH ROW
BEGIN
    INSERT INTO study_routine (user_id, routine_result, cycle_count)
    VALUES (
        NEW.user_id,
        CASE
            WHEN NEW.study_cycles >= 3 THEN '성공'
            ELSE '실패'
        END,
        NEW.study_cycles
    );
END$$

DELIMITER ;