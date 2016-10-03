#!/bin/sh

conf=$1

conf_file=config-${conf}.json

if [ ! -e build/$conf_file ]; then
    echo "Config $conf not found!"
    exit 1
fi

cd build

echo "Resetting server"
console/console a
console/console i > /dev/null
console/console r
echo "Changing to config $conf"
mv config.json config-old.json
ln -s $conf_file config.json
profiler/profiler --out profile.json

killall server
