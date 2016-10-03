#!/bin/bash

#git clone git@repair:grandet
#cd grandet
location=`pwd`
#install 4.9.2 cpp
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get -y update
sudo apt-get -y install g++-4.9
cd /usr/bin
sudo rm gcc g++ cpp
sudo ln -s gcc-4.9 gcc
sudo ln -s g++-4.9 g++
sudo ln -s cpp-4.9 cpp

# install external deps
cd $location/external
sudo apt-get -y install python-setuptools libcurl4-gnutls-dev libssl-dev libboost-thread-dev libboost-system-dev libboost-filesystem-dev libboost-regex-dev libhiredis-dev php5-dev libattr1-dev mysql-server libboost-timer-dev libxml2-dev libboost-test-dev re2c wget

if [ "`cmake --version | grep '3.[1-9]'`" == "" ]; then
sudo apt-get -y remove cmake
wget http://www.cmake.org/files/v3.2/cmake-3.2.1.tar.gz
tar -xvf cmake-3.2.1.tar.gz
cd cmake-3.2.1
./configure --prefix=/usr 
./bootstrap --prefix=/usr
make
sudo make install
cd ..
fi

# install fusepy
cd fusepy
sudo python setup.py build
sudo python setup.py install
cd ..
# install libaws
cd libaws
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
cd ../..
# install redis3m
cd redis3m
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
cd ../..
# install xattr
cd xattr-1.2.0
phpize
./configure
make
sudo make install
cd ..

# install runtime deps
mkdir -p run
sudo apt-get -y install php5-fpm nginx python-numpy python-pil redis-server python-protobuf php5-mysql php5-curl python-redis php5-gd
echo "extension=xattr.so" > 20-xattr.ini
sudo cp 20-xattr.ini /etc/php5/cli/conf.d/
sudo cp 20-xattr.ini /etc/php5/fpm/conf.d/
rm 20-xattr.ini
if [ ! -h "/usr/sbin/php-fpm" ]; then
sudo ln -s php5-fpm /usr/sbin/php-fpm
fi
if [ ! -h "/etc/nginx/fastcgi.conf" ]; then
sudo ln -s /etc/nginx/fastcgi_params /etc/nginx/fastcgi.conf
fi

echo `pwd`
sudo mv /etc/mysql/my.cnf.bak my.cnf
sudo cp /etc/mysql/my.cnf my.cnf.bak
sudo sed '/^user/d;/^pid-file/d' my.cnf.bak > my.cnf
sudo mv /etc/mysql/my.cnf my.cnf.bak
sudo mv my.cnf /etc/mysql/
#edit /etc/mysql/my.cnf, remove user, pid-file
#edit /etc/php5/fpm/php.ini, change both “size =” to 64M, add “
sudo apt-get install -y apparmor-utils
sudo aa-complain /usr/sbin/mysqld

# prepare to build grandet
sudo apt-get -y install libprotobuf-dev libreadline-dev libgtest-dev clang-3.5 libboost-program-options-dev protobuf-compiler
#install new version (>= 3.1) of cmake
#install curlpp (libcurlpp-dev)
sudo apt-get -y install libcurlpp-dev

cd /usr/src/gtest
sudo cmake .
sudo make
sudo mv libgtest* /usr/lib/
# build grandet
cd $location
mkdir -p build
cd build
#env CC=clang-3.5 env CXX=clang++-3.5 
cmake ..
make

# ffmpeg
sudo apt-get install -y libav-tools
sudo ln -s avconv /usr/bin/ffmpeg
sudo ln -s /usr/bin/ffmpeg /sbin/ffmpeg
# start redis
#setup /ssd and /magnetic
sudo mkdir -p /ssd
sudo mkdir -p /magnetic
sudo chmod 777 /ssd
sudo chmod 777 /magnetic
cd $location
redis-server etc/ssd_redis.conf
redis-server etc/mag_redis.conf
# start grandet
cd build
make python



server/server
# run
#> setup config.ini
#> bash <script> <config>
#> python client.py <config>
