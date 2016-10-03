function prepare() {
    echo ">>> Clearing run/"
    rm -r run/log/*
    mkdir -p run
    mkdir -p run/log
    mkdir -p run/data
}

function killthem() {
    echo ">>> Killing php-fpm and nginx"
    pkill php-fpm
    pkill nginx

    sleep 1

    if [ "`pgrep nginx`" != "" ]; then
        echo "still has nginx"
        exit 1
    fi
    if [ "`pgrep php-fpm`" != "" ]; then
        echo "still has php-fpm"
        exit 1
    fi
}

function runthem() {
    proj=$1
    user=`whoami`
    cat etc/php-fpm.conf | sed -e "s/CURRENT_USER/$user/" > run/php-fpm.conf

    echo ">>> Running php-fpm"
    php-fpm -y run/php-fpm.conf -p .

    echo ">>> Running nginx"
    nginx -c run/$proj-nginx.conf -p .
}

function reset_mysql() {
    echo ">>> Shutdown old mysql"
    mysqladmin -uroot --socket=./run/mysqld.sock shutdown

    echo ">>> Reinitialize mysql database"
    rm -r run/data
    mkdir -p run/data
    mysql_install_db --datadir=./run/data --basedir=/usr

    echo ">>> Starting mysql"
    mysqld --datadir=`pwd`/run/data --socket=`pwd`/run/mysqld.sock --pid-file=`pwd`/run/mysqld.pid --log-error=`pwd`/run/mysqld.log &
    while [ ! -e run/mysqld.sock ]; do
        echo "waiting for mysqld socket";
        sleep 1;
    done
}

function reset_fuse() {
    path=$1
    cache_size=$2
    mount_name=$3
    logname=run/fuse_${mount_name}.log

    echo ">>> Unmount $path"
    fusermount -u $path
    if [ "`mount | grep -i $path`" != "" ]; then
        echo "Fail to unmount!"
        exit 1
    fi

    echo ">>> Reset mountpoint"
    rm -rf $path
    mkdir -p $path

    echo ">>> Mount new fs"
    fuse/fuse.sh $path $cache_size $mount_name 2> $logname &
    sleep 1
    # fuse-cpp: fuse.grandet_fuse
    # fuse-py: GrandetOperation
    if [ "`mount | grep -i grandet`" == "" ]; then
        echo "FUSE mount error!"
        cat $logname
        exit 1
    fi
}

function get_arg() {
    proj=$1
    config=$2
    name=$3
    grep $config examples/$proj/config.ini -A 30 | grep $name | head -n 1 | cut -d = -f 2-2
}

function reset_server() {
    echo ">>> Resetting grandet server"
    build/console/console a
    build/console/console i > /dev/null
    build/console/console r
    redis-cli flushall
}

function mount_fs() {
    fstype=$1
    path=$2
    cache_size=$3
    mount_name=$4
    bucket=$5
    logname=run/fuse_${mount_name}.log

    echo ">>> Unmount $path"
    fusermount -u $path
    if [ "`mount | grep -i $path`" != "" ]; then
        echo "Fail to unmount!"
        exit 1
    fi

    echo ">>> Reset mountpoint $path"
    rm -rf $path
    mkdir -p $path

    if [ "$fstype" = "s3fs" ]; then
        cache=/tmp/grandet/s3fs_${mount_name}
        echo ">>> Creating S3FS cache at $cache"
        mkdir -p $cache

        echo ">>> Mounting S3FS"
        s3fs ${bucket} ${path} -o passwd_file=s3pw,use_cache=${cache}
        if [ "`mount | grep -i s3fs`" = "" ]; then
            echo "S3FS mount error!"
            exit 1
        fi
        rm -fr ${path}/*
    elif [ "$fstype" = "ssd" ]; then
        ssd_path=/ssd/${mount_name}
        echo ">>> Linking from $ssd_path"
        mkdir -p $ssd_path
        rm -rf $path
        ln -s $ssd_path $path
    elif [ "$fstype" = "fuse" ]; then
        fuse_path="/tmp/xmp/ssd/${mount_name}"
        echo ">>> Linking from $fuse_path"
        mkdir -p $fuse_path
        rm -rf $path
        ln -s $fuse_path $path
    else
        echo ">>> Mount Grandet-Fuse"
        fuse/fuse.sh $path $cache_size $mount_name 2> $logname &
        sleep 1
        # fuse-cpp: fuse.grandet_fuse
        # fuse-py: GrandetOperation
        if [ "`mount | grep -i grandet`" = "" ]; then
            echo "FUSE mount error!"
            cat $logname
            exit 1
        fi
    fi
}

