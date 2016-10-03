#!/bin/sh

hostname=localhost
server=${hostname}:5001

config=$1
if [ "$config" == "" ]; then
    echo "missing config"
    exit 1
fi
fstype=$2

proj=elgg

source ./common.sh

#if [ ! -e examples/cumulusclips_orig/LICENSE.txt ]; then
#(cd examples/cumulusclips_orig; tar zxvf data.tar.gz --skip-old-files)
#fi

#rm -f examples/cumulusclips_orig/cc-core/logs/*

cat etc/$proj-nginx.conf | sed -e 's/ROOT_PATH/examples\/elgg/' | sed -e 's/grandet/'$hostname'/' > run/$proj-nginx.conf

reset_mysql
mysql -f -uroot --socket=./run/mysqld.sock < etc/${proj}-init.sql
cat etc/$proj-db.sql | sed -e "s/HOST_NAME/$server/" > run/$proj-db.sql
mysql -f -u$proj -p$proj --socket=./run/mysqld.sock < run/$proj-db.sql

cache_size=`get_arg $proj $config cache`
echo "cache size: $cache_size"

reset_server
mount_fs "$fstype" /tmp/${proj}_upload $cache_size ${proj} cloud-econ-s3fs

prepare
killthem
runthem $proj

