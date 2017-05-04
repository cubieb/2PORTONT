#!/bin/sh

TR142_MODULE=/lib/modules/rtk_tr142.ko

if [ -f $TR142_MODULE ]; then
	echo "Loading TR-142 Module..."
	/bin/insmod $TR142_MODULE
else
	echo "$TR142_MODULE not found\n"
fi
