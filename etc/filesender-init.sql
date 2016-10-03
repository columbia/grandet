drop user 'filesender'@'localhost';
drop database filesender;

create user 'filesender'@'localhost' identified by 'filesender';
create database filesender;
grant all on filesender.* to 'filesender'@'localhost';
