proj=$1

mkdir -p dist/${proj}

i=0
while true; do
    build/console/console a
    build/console/console i > dist/${proj}/dist_${i}.txt
    i=`expr $i + 1`
    sleep 5
done
