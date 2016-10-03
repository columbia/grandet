drop user 'cclips'@'localhost';
drop database cclips;

create user 'cclips'@'localhost' identified by 'cclips';
create database cclips;
grant all on cclips.* to 'cclips'@'localhost';
