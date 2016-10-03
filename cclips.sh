#!/bin/sh

server=127.0.0.1:5001

config=$1
fstype=$2
proj=cclips_orig
sproj=cclips
xproj=cumulusclips_orig
root=examples/$xproj

source ./common.sh

if [ ! -e $root/LICENSE.txt ]; then
(cd $root; tar zxvf data.tar.gz --skip-old-files)
fi

rm -f $root/cc-core/logs/*
mkdir -p $root/cc-core/logs

cat etc/nginx.conf | sed -e 's/ROOT_PATH/examples\/cumulusclips_orig/' > run/$sproj-nginx.conf

reset_mysql

echo ">>> Importing initial database content"
mysql -f -uroot --socket=./run/mysqld.sock < etc/$sproj-init.sql
cat etc/$sproj-db.sql | sed -e "s/HOST_NAME/$server/" > run/$sproj-db.sql
mysql -f -u$sproj -p$sproj --socket=./run/mysqld.sock < run/$sproj-db.sql

prepare
killthem
runthem $sproj

cache_size=`get_arg $xproj $config cache`
echo "cache size: $cache_size"

reset_server

echo ">>> Killing ffmpeg"
pkill -9 ffmpeg

mount_fs "$fstype" $root/cc-content/uploads $cache_size ${proj} cloud-econ-s3fs
sleep 1

mkdir -p $root/cc-content/uploads/h264
mkdir -p $root/cc-content/uploads/temp
mkdir -p $root/cc-content/uploads/mobile
mkdir -p $root/cc-content/uploads/thumbs
