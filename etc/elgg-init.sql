DROP USER 'elgg'@'localhost';
DROP DATABASE elgg;

CREATE USER 'elgg'@'localhost' identified by 'elgg';
CREATE DATABASE elgg;
GRANT ALL ON elgg.* TO 'elgg'@'localhost';
