-- 프로시저 생성
DELIMITER $$

CREATE PROCEDURE generate_study_data()
BEGIN
    DECLARE cur_date DATE;
    DECLARE end_date DATE;
    DECLARE session_count INT;
    DECLARE i INT;
    DECLARE user_card VARCHAR(255);
    DECLARE start_dt DATETIME;
    DECLARE end_dt DATETIME;
    DECLARE study_duration INT;
    DECLARE break_duration INT;
    DECLARE session_type INT; -- 0: 휴식, 1: 공부
    DECLARE user_index INT;

    SET cur_date = '2025-04-01';
    SET end_date = '2025-05-25';

    WHILE cur_date <= end_date DO
        SET user_index = 0;
        WHILE user_index < 2 DO
            SET user_card = IF(user_index = 0, '147 148 214 5', '180 175 140 4');
            SET session_count = FLOOR(RAND() * 5) + 6; -- 6 ~ 10회

            SET start_dt = STR_TO_DATE(CONCAT(cur_date, ' 08:00:00'), '%Y-%m-%d %H:%i:%s');

            SET i = 0;
            WHILE i < session_count DO
                -- 공부 세션
                SET study_duration = CONVERT(FLOOR(RAND() * 36 + 25), UNSIGNED); -- 25~60분 정수로 변환
                SET end_dt = DATE_ADD(start_dt, INTERVAL study_duration MINUTE);

                INSERT INTO Recd_Table (Card_Num, Status, Start_Time, End_Time, Amount)
                VALUES (user_card, 1, start_dt, end_dt, TIMESTAMPDIFF(MINUTE, start_dt, end_dt));

                SET start_dt = end_dt;

                -- 휴식 세션
                SET break_duration = CONVERT(FLOOR(RAND() * 11 + 10), UNSIGNED); -- 10~20분 정수로 변환
                SET end_dt = DATE_ADD(start_dt, INTERVAL break_duration MINUTE);

                INSERT INTO Recd_Table (Card_Num, Status, Start_Time, End_Time, Amount)
                VALUES (user_card, 0, start_dt, end_dt, TIMESTAMPDIFF(MINUTE, start_dt, end_dt));

                SET start_dt = end_dt;

                SET i = i + 1;
            END WHILE;

            SET user_index = user_index + 1;
        END WHILE;

        SET cur_date = DATE_ADD(cur_date, INTERVAL 1 DAY);
    END WHILE;
END $$

DELIMITER ;

-- 프로시저 실행
CALL generate_study_data();

-- 프로시저 제거
DROP PROCEDURE IF EXISTS generate_study_data;

-- Amount 컬럼값이 100단위로 들어갔을 경우 업데이트
UPDATE Recd_Table SET Amount = TIMESTAMPDIFF(MINUTE, Start_Time, End_Time);