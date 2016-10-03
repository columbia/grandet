#!/bin/sh

source ./common.sh

config=$1
if [ "$config" == "" ]; then
    echo "missing config"
    exit 1
fi
fstype=$2

proj=filesender_orig
sproj=filesender
root=examples/$proj

#sh getsaml.sh
if [ ! -e $root/language ]; then
    (cd $root; tar zxvf data.tar.gz)
fi
mkdir -p $root/files
cat $root/config/config-temp.php | sed -e "s|PREFIX|`pwd`/$root|" > $root/config/config.php

#cp etc/simplesaml-config.php examples/simplesaml/config/config.php

cat etc/nginx.conf | sed -e "s|ROOT_PATH|$root|" > run/$sproj-nginx.conf

mkdir -p $root/log
rm -f $root/log/*

reset_mysql
sleep 1
mysql -f -uroot --socket=./run/mysqld.sock < etc/$sproj-init.sql
mysql -f -u$sproj -p$sproj --socket=./run/mysqld.sock < $root/scripts/mysql_filesender_db.sql

reset_server

cache_size=`get_arg $proj $config cache`
echo "cache size: $cache_size"

mount_fs "$fstype" $root/files $cache_size ${proj}_files cloud-econ-s3fs
mount_fs "$fstype" $root/tmp $cache_size ${proj}_tmp cloud-econ-s3fs-2

prepare
killthem
runthem $sproj

