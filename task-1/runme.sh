#!/bin/bash

make -s sparse
make -s create_file_A

./create_file_A
./sparse A B
./sparse A D -b 100

gzip -kf A
gzip -kf B

gzip -cd B.gz | ./sparse C

for file in A A.gz B B.gz C D; do
    if [ -e "$file" ]; then
        blocks=$(stat -f "%b" "$file")
        block_size=$(stat -f "%B" "$file")
        total=$((blocks * block_size))
        echo "FILE $file: $total bytes"
    else
        echo "FILE $file: not found" >&2
    fi
done > result.txt
