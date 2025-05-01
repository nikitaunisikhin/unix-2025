#!/usr/bin/env bash

make
rm -f result.txt

for i in {1..10}; do
    ./locker -f file.txt &
done
sleep 300
killall -SIGINT locker

# cleanup
rm -f file.txt.lck locker

echo "Done. Check result.txt"