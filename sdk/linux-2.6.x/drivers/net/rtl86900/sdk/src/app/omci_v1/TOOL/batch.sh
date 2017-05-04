#!/bin/sh

cmd=pon_gen.py
dir=$1

files=`find ./$dir -name *.conf | awk '{FS="/"} { print $3 }'`

for conf in $files
do
	./$cmd -f $conf -fd $dir
done



