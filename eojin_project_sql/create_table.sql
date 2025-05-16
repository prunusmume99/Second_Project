CREATE TABLE study_log (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id VARCHAR(50) NOT NULL,
    study_time INT NOT NULL,
    rest_time INT NOT NULL,
    study_cycle INT NOT NULL,
    total_study_time INT NOT NULL
);
