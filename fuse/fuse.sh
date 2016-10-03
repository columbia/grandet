#!/bin/sh

mount_point=$1
cache_size=$2
mount_name=$3

mount_folder=/tmp/grandet/$mount_name
rm -fr $mount_folder
mkdir -p $mount_folder

#env PYTHONPATH=build/sdk/python/ fuse/grandet_fuse_v3.py $mount_folder $mount_point $cache_size debug
# env PYTHONPATH=build/sdk/python/ fuse/grandet_fuse_v3.py $mount_folder $mount_point $cache_size
# to use cpp version:
#fuse/cpp/grandet_fuse -d $mount_folder -c $cache_size : $mount_point -f -s -o debug
fuse/cpp/grandet_fuse -d $mount_folder -c $cache_size : $mount_point -f
