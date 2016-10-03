#!/bin/sh

config=$1
if [ "$config" == "" ]; then
    echo "missing config"
    exit 1
fi
fstype=$2

proj=piwigo_orig
sproj=piwigo

source ./common.sh

cat etc/nginx.conf | sed -e 's/ROOT_PATH/examples\/piwigo_orig/' > run/$proj-nginx.conf

reset_mysql

echo ">>> Import initial database content"
mysql -f --socket=./run/mysqld.sock -uroot < etc/${sproj}.sql

cache_size=`get_arg $proj $config cache`
echo "cache size: $cache_size"

reset_server

mount_fs "$fstype" examples/$proj/_data $cache_size ${proj}_data cloud-econ-s3fs
mount_fs "$fstype" examples/$proj/upload $cache_size ${proj}_upload cloud-econ-s3fs-2

cp etc/$sproj-database.inc.php examples/$proj/local/config/database.inc.php

prepare
killthem
runthem $proj
