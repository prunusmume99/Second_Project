-- 직전 한달동안의 평균 패턴 구하기
SELECT
  FLOOR(AVG(CASE WHEN Status = 1 THEN Amount END)) AS avg_study_minutes,
  FLOOR(AVG(CASE WHEN Status = 0 THEN Amount END)) AS avg_break_minutes
FROM Recd_Table
WHERE Start_Time >= (CURDATE() - INTERVAL 1 MONTH) AND Start_Time < CURDATE()
AND Card_Num = ''


-- 하루동안 총 공부, 휴식 시간 구하기
SELECT
  SUM(CASE WHEN Status = 1 THEN Amount ELSE 0 END) AS total_study_minutes,
  SUM(CASE WHEN Status = 0 THEN Amount ELSE 0 END) AS total_break_minutes
FROM Recd_Table
WHERE Start_Time >= CURDATE()
AND Card_Num = ''
