#!/bin/sh

VERSION=$1

filelist=`find . -name *.so | grep -v LIB`

for name in $filelist
do
	mv $name $name.$VERSION
done

mv omci_app omci_app.$VERSION
mv omcicli  omcicli.$VERSION
mv DRV/omcidrv.ko DRV/omcidrv.ko.$VERSION
