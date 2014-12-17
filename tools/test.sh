#!/bin/sh

rm -f test/*

for i in {1..97}
do
	./piproxy -a 10.54.21.$i -t 16 -s 10 -f 1 > test/test$i.dat
done

ls -lh test | grep "   0"
