#!/bin/sh

rm -f test/*

for i in {10..95}
do
	./piproxy -a 10.54.21.$i -t 16 -s 10 -f 1 > test/test$i.dat &
done

sleep 15

ls -lh test
