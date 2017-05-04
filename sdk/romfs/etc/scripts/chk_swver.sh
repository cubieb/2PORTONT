#!/bin/sh

SWVER0=$(/bin/nv getenv sw_version0 | awk -F'=' '{print $2}') 
SWVER1=$(/bin/nv getenv sw_version1 | awk -F'=' '{print $2}')
FWVER=$(awk -F" " '{print $1}' /etc/version)

if [ "$SWVER0" == "0" ] && [ "$SWVER1" == "0" ]; then
	/bin/nv setenv sw_version0 $FWVER
	/bin/nv setenv sw_version1 $FWVER
	echo "All SW_VERSION are 0, set to " $FWVER
fi
